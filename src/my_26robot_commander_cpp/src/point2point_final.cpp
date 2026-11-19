#include <array>
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <iterator>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <Eigen/Geometry>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <moveit/move_group_interface/move_group_interface.h>
#include <moveit_msgs/msg/display_trajectory.hpp>
#include <my_26robot_interfaces/msg/j_joint_command.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/header.hpp>

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>(
        "point2point_final",
        rclcpp::NodeOptions().automatically_declare_parameters_from_overrides(true));

    auto serial_publisher =
        node->create_publisher<my_26robot_interfaces::msg::JJointCommand>(
            "/send_pack", rclcpp::SensorDataQoS());
    auto display_publisher =
        node->create_publisher<moveit_msgs::msg::DisplayTrajectory>(
            "display_planned_path", 10);

    std::mutex feedback_mutex;
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
                    std::lock_guard<std::mutex> lock(feedback_mutex);
                    feedback = *msg;
                    feedback_ready = true;
                }
                feedback_cv.notify_one();
            });

    std::mutex ack_mutex;
    std::condition_variable ack_cv;
    std::string pending_frame_id;
    bool command_acknowledged = false;
     // 订阅串口驱动发来的发送确认消息。
    auto ack_subscription =
        node->create_subscription<std_msgs::msg::Header>(
            "/send_pack_ack", rclcpp::SensorDataQoS(),
            [&](const std_msgs::msg::Header::SharedPtr ack) {
                std::lock_guard<std::mutex> lock(ack_mutex);
                if (!pending_frame_id.empty() &&
                    ack->frame_id == pending_frame_id) {
                    command_acknowledged = true;
                    ack_cv.notify_one();
                }
            });

    rclcpp::executors::SingleThreadedExecutor executor;
    executor.add_node(node);
    std::thread spinner([&executor]() { executor.spin(); });

    //关闭 ROS 2 并等待后台线程结束
    auto stop_node = [&]() {
        rclcpp::shutdown();
        if (spinner.joinable()) {
            spinner.join();
        }
    };

    auto arm = moveit::planning_interface::MoveGroupInterface(node, "arm");
    arm.setMaxVelocityScalingFactor(0.8);
    arm.setMaxAccelerationScalingFactor(0.8);

    my_26robot_interfaces::msg::JJointCommand current_feedback;
    {
        std::unique_lock<std::mutex> lock(feedback_mutex);
        if (!feedback_cv.wait_for(
                lock, std::chrono::seconds(30),
                [&feedback_ready]() {
                    return feedback_ready || !rclcpp::ok();
                }) ||
            !feedback_ready) {
            RCLCPP_ERROR(node->get_logger(), "等待 mode=1 的关节反馈超时。");
            stop_node();
            return 1;
        }
        current_feedback = feedback;
    }

    auto start_state = arm.getCurrentState();
    if (!start_state) {
        RCLCPP_ERROR(node->get_logger(), "无法获取 MoveIt 机器人状态。");
        stop_node();
        return 1;
    }

    //同步角度
    const std::array<std::string, 6> joint_names = {
        "joint1", "joint2", "joint3", "joint4", "joint5", "joint6"};
    for (std::size_t joint = 0; joint < joint_names.size(); ++joint) {
        start_state->setVariablePosition(
            joint_names[joint], current_feedback.joint_angles[joint]);
    }
    start_state->update();
    arm.setStartState(*start_state);

    const auto & current_transform =
        start_state->getGlobalLinkTransform(arm.getEndEffectorLink());
    const Eigen::Quaterniond orientation(current_transform.rotation());
    geometry_msgs::msg::PoseStamped target_pose;
    target_pose.header.frame_id = arm.getPlanningFrame();
    target_pose.pose.position.x = current_transform.translation().x();
    target_pose.pose.position.y = current_transform.translation().y();
    target_pose.pose.position.z = current_transform.translation().z() + 0.10;
    target_pose.pose.orientation.x = orientation.x();
    target_pose.pose.orientation.y = orientation.y();
    target_pose.pose.orientation.z = orientation.z();
    target_pose.pose.orientation.w = orientation.w();
    arm.setPoseTarget(target_pose);

    moveit::planning_interface::MoveGroupInterface::Plan plan;
    if (arm.plan(plan) != moveit::core::MoveItErrorCode::SUCCESS) {
        RCLCPP_ERROR(node->get_logger(), "位姿规划失败，不发送串口命令。");
        stop_node();
        return 1;
    }

    moveit_msgs::msg::DisplayTrajectory display_trajectory;
    display_trajectory.trajectory_start = plan.start_state_;
    display_trajectory.trajectory.push_back(plan.trajectory_);
    display_publisher->publish(display_trajectory);

    const auto & trajectory = plan.trajectory_.joint_trajectory;
    if (trajectory.points.empty()) {
        RCLCPP_ERROR(node->get_logger(), "规划结果中没有轨迹点。");
        stop_node();
        return 1;
    }

    std::array<std::size_t, 6> joint_indices{};
    for (std::size_t joint = 0; joint < joint_names.size(); ++joint) {
        const auto it = std::find(
            trajectory.joint_names.begin(),
            trajectory.joint_names.end(),
            joint_names[joint]);
        if (it == trajectory.joint_names.end()) {
            RCLCPP_ERROR(
                node->get_logger(), "规划轨迹缺少关节 %s。",
                joint_names[joint].c_str());
            stop_node();
            return 1;
        }
        joint_indices[joint] = static_cast<std::size_t>(
            std::distance(trajectory.joint_names.begin(), it));
    }

    const auto & final_point = trajectory.points.back();
    my_26robot_interfaces::msg::JJointCommand command;
    command.header.stamp = node->now();
    command.header.frame_id =
        "point2point-final-" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    for (std::size_t joint = 0; joint < joint_indices.size(); ++joint) {
        if (joint_indices[joint] >= final_point.positions.size()) {
            RCLCPP_ERROR(node->get_logger(), "最后一个轨迹点缺少关节角数据。");
            stop_node();
            return 1;
        }
        command.joint_angles[joint] =
            final_point.positions[joint_indices[joint]];
    }

    RCLCPP_INFO(
        node->get_logger(),
        "规划轨迹共 %zu 组，只发送最后一组关节角："
        "[%.6f, %.6f, %.6f, %.6f, %.6f, %.6f]",
        trajectory.points.size(),
        command.joint_angles[0],
        command.joint_angles[1],
        command.joint_angles[2],
        command.joint_angles[3],
        command.joint_angles[4],
        command.joint_angles[5]);

    const auto connection_deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(30);
    while (rclcpp::ok() &&
           (serial_publisher->get_subscription_count() == 0 ||
            ack_subscription->get_publisher_count() == 0) &&
           std::chrono::steady_clock::now() < connection_deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (serial_publisher->get_subscription_count() == 0 ||
        ack_subscription->get_publisher_count() == 0) {
        RCLCPP_ERROR(
            node->get_logger(),
            "未连接到完整串口发送/确认端点，不发送最后一个轨迹点。");
        stop_node();
        return 1;
    }

    {
        std::lock_guard<std::mutex> lock(ack_mutex);
        pending_frame_id = command.header.frame_id;
        command_acknowledged = false;
    }
    serial_publisher->publish(command);
    RCLCPP_INFO(node->get_logger(), "最后一个轨迹点已发布到 /send_pack。");

    bool acknowledged = false;
    {
        std::unique_lock<std::mutex> lock(ack_mutex);
        acknowledged = ack_cv.wait_for(
            lock, std::chrono::seconds(2),
            [&command_acknowledged]() {
                return command_acknowledged || !rclcpp::ok();
            });
        acknowledged = acknowledged && command_acknowledged;
    }
    if (acknowledged) {
        RCLCPP_INFO(node->get_logger(), "串口驱动已确认发送最后一个轨迹点。");
    } else {
        RCLCPP_ERROR(
            node->get_logger(),
            "等待串口确认超时；该确认表示驱动发送成功，不代表下位机已执行。");
        stop_node();
        return 1;
    }

    stop_node();
    return 0;
}
