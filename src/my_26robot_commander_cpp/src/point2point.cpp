#include <chrono>
#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <rclcpp/rclcpp.hpp>
#include <moveit/move_group_interface/move_group_interface.h>
#include <moveit_msgs/msg/display_trajectory.hpp>
#include <my_26robot_interfaces/msg/j_joint_command.hpp>
#include <std_msgs/msg/header.hpp>
#include <Eigen/Geometry>

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>(
        "point2point",
        rclcpp::NodeOptions().automatically_declare_parameters_from_overrides(true));

    // 创建发布者，把关节命令发送到 RM 串口驱动的话题。
    auto serial_publisher =
        node->create_publisher<my_26robot_interfaces::msg::JJointCommand>(
            "/send_pack", rclcpp::SensorDataQoS());
    // 创建发布者，把规划好的轨迹发送给 RViz 显示。
    auto display_publisher =
        node->create_publisher<moveit_msgs::msg::DisplayTrajectory>(
            "display_planned_path", 10);

    // 防止回调和主线程同时读写。
    std::mutex feedback_mutex;
    // 等待有效关节反馈，回调收到反馈后会唤醒主线程。
    std::condition_variable feedback_cv;
    my_26robot_interfaces::msg::JJointCommand feedback;
    bool feedback_ready = false;

    auto feedback_subscription =
        node->create_subscription<my_26robot_interfaces::msg::JJointCommand>(
            "/recieve_pack", rclcpp::SensorDataQoS(),
            [&](const my_26robot_interfaces::msg::JJointCommand::SharedPtr msg) {
                if (msg->mode != 1) {
                    return;
                }
                {
                    // 锁住反馈数据，避免其他线程同时修改它。
                    std::lock_guard<std::mutex> lock(feedback_mutex);
                    // 保存整条有效反馈，包括六个关节角和 mode。
                    feedback = *msg;

                    feedback_ready = true;
                }
                // 通知正在等待反馈的主线程继续执行。
                feedback_cv.notify_one();
            });

    std::mutex send_flag_mutex;
    // 串口确认到达后，用它唤醒等待发送结果的线程。
    std::condition_variable send_flag_cv;
    std::size_t send_flag = 0;
    // 保存当前等待确认的命令 ID，用于匹配确认消息。
    std::string pending_frame_id;
    // 订阅串口驱动发来的发送确认消息。
    auto serial_ack_subscription =
        node->create_subscription<std_msgs::msg::Header>(
            "/send_pack_ack", rclcpp::SensorDataQoS(),
            // ack 是本次收到的确认消息。
            [&](const std_msgs::msg::Header::SharedPtr ack) {
                std::lock_guard<std::mutex> lock(send_flag_mutex);
                // 只有确认 ID 与当前待确认命令一致时才处理确认。
                if (send_flag > 0 && ack->frame_id == pending_frame_id) {
                    // 清除“有命令等待确认”的标记。
                    --send_flag;
                    // 清空已经完成确认的命令 ID。
                    pending_frame_id.clear();
                    // 唤醒正在等这条确认的发送线程。
                    send_flag_cv.notify_one();
                }
            });

    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(node);
    std::thread spinner([&executor]() { executor.spin(); });

    auto arm = moveit::planning_interface::MoveGroupInterface(node, "arm");
    arm.setMaxVelocityScalingFactor(0.8);
    arm.setMaxAccelerationScalingFactor(0.8);

    RCLCPP_INFO(node->get_logger(), "等待下位机反馈 mode=1 的关节角...");
    
    my_26robot_interfaces::msg::JJointCommand current_feedback;
    {
        // 获取反馈互斥锁，供条件变量安全检查反馈状态。
        std::unique_lock<std::mutex> lock(feedback_mutex);
        // 最多等待 30 秒，收到反馈或 ROS 关闭时停止等待。
        if (!feedback_cv.wait_for(
                lock, std::chrono::seconds(30),
                // 等待条件：收到有效反馈，或者 ROS 正在关闭。
                [&feedback_ready]() { return feedback_ready || !rclcpp::ok(); }) ||
            // 即使等待提前结束，也再次确认确实收到过有效反馈。
            !feedback_ready) {
            RCLCPP_ERROR(node->get_logger(), "等待 mode=1 的关节反馈超时，停止规划。");
            rclcpp::shutdown();
            // 如果回调线程还在运行，就等待它结束。
            if (spinner.joinable()) {
                spinner.join();
            }
            return 1;
        }
        // 把收到的有效反馈复制到主线程使用的变量中。
        current_feedback = feedback;
    }

    // 从 MoveIt 获取机器人当前状态，作为要更新的起始状态。
    auto start_state = arm.getCurrentState();
    // 检查 MoveIt 是否成功提供了机器人状态。
    if (!start_state) {
        // 获取失败时记录错误。
        RCLCPP_ERROR(node->get_logger(), "无法获取 MoveIt 机器人状态，停止规划。");
        // 关闭 ROS 2。
        rclcpp::shutdown();
        // 等待后台回调线程退出。
        if (spinner.joinable()) {
            spinner.join();
        }
        // 返回错误码，停止后续规划。
        return 1;
    }
    // 按消息中的关节顺序定义 MoveIt 使用的六个关节名称。
    const std::array<std::string, 6> joint_names = {
        "joint1", "joint2", "joint3", "joint4", "joint5", "joint6"};
    // 逐个处理六个关节，把下位机角度写入 MoveIt 状态。
    for (std::size_t joint = 0; joint < joint_names.size(); ++joint) {
        // 通过关节名设置对应的关节角。
        start_state->setVariablePosition(
            joint_names[joint], current_feedback.joint_angles[joint]);
    }
    // 根据更新后的关节角重新计算机器人各连杆的位姿。
    start_state->update();
    // 告诉 MoveIt 后续规划从这个反馈状态开始。
    arm.setStartState(*start_state);

    // 根据起始关节状态，计算末端执行器在规划坐标系中的变换。
    const auto & current_transform =
        start_state->getGlobalLinkTransform(arm.getEndEffectorLink());
    // 把末端的旋转矩阵转换为四元数，方便填写 ROS 位姿消息。
    const Eigen::Quaterniond current_orientation(current_transform.rotation());
    // 创建一个带坐标系信息的末端位姿消息。
    geometry_msgs::msg::PoseStamped current_pose;
    // 设置位姿使用的坐标系为 MoveIt 规划坐标系。
    current_pose.header.frame_id = arm.getPlanningFrame();
    // 读取末端当前的 X 坐标。
    current_pose.pose.position.x = current_transform.translation().x();
    // 读取末端当前的 Y 坐标。
    current_pose.pose.position.y = current_transform.translation().y();
    // 读取末端当前的 Z 坐标。
    current_pose.pose.position.z = current_transform.translation().z();
    // 设置末端当前旋转四元数的 X 分量。
    current_pose.pose.orientation.x = current_orientation.x();
    // 设置末端当前旋转四元数的 Y 分量。
    current_pose.pose.orientation.y = current_orientation.y();
    // 设置末端当前旋转四元数的 Z 分量。
    current_pose.pose.orientation.z = current_orientation.z();
    // 设置末端当前旋转四元数的 W 分量。
    current_pose.pose.orientation.w = current_orientation.w();

    // 输出根据反馈角度计算出的末端当前位置。
    RCLCPP_INFO(
        node->get_logger(),
        "当前末端位置: x=%.3f, y=%.3f, z=%.3f",
        current_pose.pose.position.x,
        current_pose.pose.position.y,
        current_pose.pose.position.z);

    // 复制当前位姿作为目标位姿，保留当前方向和 X、Y 坐标。
    auto target_pose = current_pose;
    // 把目标末端位置沿 Z 轴提高 0.10 米，也就是 10 厘米。
    target_pose.pose.position.z += 0.10;

    // 把目标位姿交给 MoveIt 作为规划目标。
    arm.setPoseTarget(target_pose);

    // 创建变量，用来保存 MoveIt 计算出来的完整规划结果。
    moveit::planning_interface::MoveGroupInterface::Plan plan;
    // 请求 MoveIt 规划；规划成功时 planned 为 true。
    const bool planned =
        arm.plan(plan) == moveit::core::MoveItErrorCode::SUCCESS;

    // 如果规划失败，就不发布或发送这次规划轨迹。
    if (!planned) {
        // 在终端显示规划失败原因。
        RCLCPP_ERROR(node->get_logger(), "普通位姿规划失败，未发送轨迹。");
    } else {
        // 规划成功后，等待串口驱动和 RViz 连接上对应的话题。
        RCLCPP_INFO(
            node->get_logger(),
            "等待 /send_pack 和 /send_pack_ack 串口端点...");
        // 设置串口连接等待的截止时间为当前时间后 30 秒。
        const auto subscriber_deadline =
            std::chrono::steady_clock::now() + std::chrono::seconds(30);
        // 只要 ROS 仍运行、串口发布或确认连接尚未齐备且未超时，就继续等待。
        while (rclcpp::ok() &&
               (serial_publisher->get_subscription_count() == 0 ||
                serial_ack_subscription->get_publisher_count() == 0) &&
               std::chrono::steady_clock::now() < subscriber_deadline)
        {
            // 暂停 100 毫秒，避免循环过快占用处理器。
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // 串口订阅者和确认发布者都存在时，认为串口链路已连接。
        const bool serial_connected =
            serial_publisher->get_subscription_count() > 0 &&
            serial_ack_subscription->get_publisher_count() > 0;
        // 串口链路未完整连接时，提示用户这次不会发送串口命令。
        if (!serial_connected) {
            // 输出串口连接超时警告。
            RCLCPP_WARN(
                node->get_logger(),
                "30 秒内没有发现完整串口发送/确认端点；"
                "本次不会向串口发送轨迹。");
        }

        // 提示程序开始等待 RViz 订阅轨迹话题。
        RCLCPP_INFO(node->get_logger(), "等待 RViz 轨迹显示订阅者...");
        // 设置 RViz 订阅者等待截止时间为当前时间后 30 秒。
        const auto rviz_deadline =
            std::chrono::steady_clock::now() + std::chrono::seconds(30);
        // 没有 RViz 订阅者且没有超时时，每 100 毫秒检查一次。
        while (rclcpp::ok() &&
               display_publisher->get_subscription_count() == 0 &&
               std::chrono::steady_clock::now() < rviz_deadline)
        {
            // 短暂休眠，给 RViz 时间完成连接。
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        // RViz 没有订阅时，提醒用户可能看不到轨迹。
        if (display_publisher->get_subscription_count() == 0) {
            // 输出 RViz 连接超时警告。
            RCLCPP_WARN(
                node->get_logger(),
                "30 秒内没有发现 display_planned_path 订阅者；"
                "RViz 可能看不到计划轨迹。");
        }

        // 创建用于向 RViz 展示轨迹的消息。
        moveit_msgs::msg::DisplayTrajectory display_trajectory;
        // 把规划使用的起始机器人状态放进显示消息。
        display_trajectory.trajectory_start = plan.start_state_;
        // 把 MoveIt 规划出的轨迹添加到显示消息。
        display_trajectory.trajectory.push_back(plan.trajectory_);
        // 发布显示消息，供 RViz 订阅和展示。
        display_publisher->publish(display_trajectory);

        // 保存 joint1 到 joint6 在规划轨迹关节数组中的位置索引。
        std::array<std::size_t, 6> joint_indices{};
        // 先假设轨迹有效；发现缺项时再改成 false。
        bool valid_trajectory = true;
        // 逐个在 MoveIt 轨迹的关节名称列表中查找六个目标关节。
        for (std::size_t joint = 0; joint < joint_names.size(); ++joint) {
            // 查找当前关节名在轨迹关节名列表中的位置。
            const auto it = std::find(
                plan.trajectory_.joint_trajectory.joint_names.begin(),
                plan.trajectory_.joint_trajectory.joint_names.end(),
                joint_names[joint]);
            // 如果轨迹里找不到这个关节，就不能安全地发送数据。
            if (it == plan.trajectory_.joint_trajectory.joint_names.end()) {
                // 输出缺少的关节名。
                RCLCPP_ERROR(
                    node->get_logger(),
                    "规划轨迹缺少关节 %s，不能发送到串口。",
                    joint_names[joint].c_str());
                // 标记轨迹无效。
                valid_trajectory = false;
                // 结束查找剩余关节。
                break;
            }
            // 把迭代器位置换算成数组索引，供后面读取每个关节角。
            joint_indices[joint] = static_cast<std::size_t>(
                std::distance(
                    plan.trajectory_.joint_trajectory.joint_names.begin(), it));
        }

        // 检查轨迹中的每个点都包含所需关节的位置数据。
        for (const auto & point : plan.trajectory_.joint_trajectory.points) {
            // 逐个检查六个关节索引是否在当前轨迹点的位置数组范围内。
            for (const auto index : joint_indices) {
                // 索引越界说明这个轨迹点数据不完整。
                if (index >= point.positions.size()) {
                    // 告知用户轨迹点缺少关节位置。
                    RCLCPP_ERROR(
                        node->get_logger(),
                        "规划轨迹点缺少关节位置，不能发送到串口。");
                    // 标记轨迹无效，后续不发送。
                    valid_trajectory = false;
                    // 当前轨迹点检查到错误，结束内部循环。
                    break;
                }
            }
            // 若已发现错误，无需继续检查其他轨迹点。
            if (!valid_trajectory) {
                break;
            }
        }

        // 仅在轨迹结构检查通过后，打印轨迹点信息。
        if (valid_trajectory) {
            // 输出规划轨迹包含的点数。
            RCLCPP_INFO(
                node->get_logger(),
                "规划轨迹共 %zu 组。",
                plan.trajectory_.joint_trajectory.points.size());
            // 从轨迹点编号 0 开始计数。
            std::size_t point_number = 0;
            // 逐个读取轨迹点，用于输出它的时间和六个关节角。
            for (const auto & point : plan.trajectory_.joint_trajectory.points) {
                // 将时间转换成秒，方便人阅读。
                const double time_from_start =
                    static_cast<double>(point.time_from_start.sec) +
                    static_cast<double>(point.time_from_start.nanosec) / 1e9;
                // 打印轨迹点编号、相对时间以及六个关节角。
                RCLCPP_INFO(
                    node->get_logger(),
                    "轨迹点[%zu] t=%.3f s: joint1=%.6f, joint2=%.6f, "
                    "joint3=%.6f, joint4=%.6f, joint5=%.6f, joint6=%.6f",
                    point_number++,
                    time_from_start,
                    point.positions[joint_indices[0]],
                    point.positions[joint_indices[1]],
                    point.positions[joint_indices[2]],
                    point.positions[joint_indices[3]],
                    point.positions[joint_indices[4]],
                    point.positions[joint_indices[5]]);
            }
        }

        // 声明一个线程变量；满足发送条件时才会启动它发送串口命令。
        std::thread serial_sender;
        // 只有串口已连接、轨迹有效且至少包含一个点时才启动发送线程。
        if (serial_connected && valid_trajectory &&
            !plan.trajectory_.joint_trajectory.points.empty()) {
            // 启动独立线程，按规划时间逐点发送命令并等待串口确认。
            serial_sender = std::thread(
                // 捕获线程中需要使用的数据；引用捕获的状态在 join 前保持有效。
                [node, serial_publisher, &plan, joint_indices, &send_flag_mutex,
                 &send_flag_cv, &send_flag, &pending_frame_id]() {
                // 记录发送线程的起始时间，用来安排各轨迹点的发送时刻。
                const auto start_time = std::chrono::steady_clock::now();
                // 用启动时间生成本次运行的唯一标识部分。
                const auto run_id =
                    std::chrono::duration_cast<std::chrono::nanoseconds>(
                        start_time.time_since_epoch()).count();
                // 从轨迹点编号 0 开始，为每条命令生成不同 ID。
                std::size_t point_number = 0;
                // 按规划顺序逐个处理轨迹点。
                for (const auto & point : plan.trajectory_.joint_trajectory.points) {
                    // 把轨迹点的计划时间转换成纳秒。
                    const auto point_time = std::chrono::nanoseconds(
                        static_cast<std::int64_t>(point.time_from_start.sec) * 1000000000LL +
                        point.time_from_start.nanosec);
                    // 等到该轨迹点计划的时刻，尽量按规划时间发送。
                    std::this_thread::sleep_until(start_time + point_time);
                    // ROS 关闭时不再继续发送后续轨迹点。
                    if (!rclcpp::ok()) {
                        // 直接结束串口发送线程。
                        return;
                    }

                    // 锁住确认状态，准备等待上一条命令确认或发送当前命令。
                    std::unique_lock<std::mutex> lock(send_flag_mutex);
                    // 等待最多 2 秒，直到没有待确认命令或 ROS 开始关闭。
                    if (!send_flag_cv.wait_for(
                            lock, std::chrono::seconds(2),
                            // 发送标志清零表示上一条命令已确认。
                            [&send_flag]() {
                                return send_flag == 0 || !rclcpp::ok();
                            }) ||
                        // ROS 关闭时也停止发送。
                        !rclcpp::ok()) {
                        // 等待超时或程序关闭时输出错误并结束线程。
                        RCLCPP_ERROR(node->get_logger(), "等待串口发送标志位清零超时。");
                        return;
                    }

                    // 创建一条要发送给串口驱动的关节命令消息。
                    my_26robot_interfaces::msg::JJointCommand command;
                    // 给命令写入当前 ROS 时间戳。
                    command.header.stamp = node->now();
                    // 生成本次运行和当前轨迹点专属的确认匹配 ID。
                    command.header.frame_id =
                        "point2point-" + std::to_string(run_id) + "-" +
                        std::to_string(point_number++);
                    // 逐个复制轨迹点中的六个关节角到串口命令。
                    for (std::size_t joint = 0; joint < joint_indices.size(); ++joint) {
                        // 使用之前查出的索引，读取对应关节角并填入命令。
                        command.joint_angles[joint] =
                            point.positions[joint_indices[joint]];
                    }

                    // 保存当前命令 ID，后续只接受 ID 匹配的确认消息。
                    pending_frame_id = command.header.frame_id;
                    // 标记现在有一条命令等待串口确认。
                    ++send_flag;
                    // 发布关节命令，交给串口驱动处理。
                    serial_publisher->publish(command);

                    // 等待最多 2 秒，确认串口驱动处理了当前命令。
                    const bool acknowledged = send_flag_cv.wait_for(
                        lock, std::chrono::seconds(2),
                        // 收到匹配确认后，回调会把 send_flag 设回 0。
                        [&send_flag]() {
                            return send_flag == 0 || !rclcpp::ok();
                        });
                    // 等待超时或仍未收到确认时，不再发送后续点。
                    if (!acknowledged || send_flag != 0) {
                        // 输出确认超时错误。
                        RCLCPP_ERROR(
                            node->get_logger(),
                            "等待串口确认超时，停止发送后续轨迹点。");
                        // 结束串口发送线程。
                        return;
                    }
                }
            });
        // 串口已连接且轨迹有效，但规划结果没有轨迹点时报告错误。
        } else if (serial_connected && valid_trajectory) {
            // 输出空轨迹错误信息。
            RCLCPP_ERROR(node->get_logger(), "规划轨迹没有可发送的轨迹点。");
        }

        // 让 MoveIt 执行规划结果；RViz/MoveIt 仿真会显示执行过程。
        const auto execution_result = arm.execute(plan);
        // 如果串口发送线程已经启动，就等待它结束后再清理程序。
        if (serial_sender.joinable()) {
            serial_sender.join();
        }
        // 根据 MoveIt 的执行返回码输出结果。
        if (execution_result == moveit::core::MoveItErrorCode::SUCCESS) {
            // MoveIt 报告轨迹执行成功。
            RCLCPP_INFO(node->get_logger(), "轨迹执行完成。");
        } else {
            // MoveIt 报告轨迹执行失败。
            RCLCPP_ERROR(node->get_logger(), "轨迹执行失败。");
        }
    }

    // 通知 ROS 2 关闭，停止节点通信。
    rclcpp::shutdown();
    // 确保后台 ROS 回调线程结束后再退出程序。
    if (spinner.joinable()) {
        spinner.join();
    }
    // 规划成功返回 0；规划失败返回 1。
    return planned ? 0 : 1;
}
