#include <rclcpp/rclcpp.hpp>
#include <rclcpp/serialized_message.hpp>
#include <moveit/move_group_interface/move_group_interface.h>
#include <moveit/robot_model/joint_model_group.h>
#include <moveit/robot_state/robot_state.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <thread>
#include <vector>

using MoveGroupInterface = moveit::planning_interface::MoveGroupInterface;
namespace
{
// 串口协议固定使用六个关节角；ROS 消息类型为 float32[] joint_angles。
constexpr std::size_t kJointCount = 6;

// 序列化后的 ROS 消息缓冲区大小，供通用发布器发送序列化数据使用。
// CDR 布局（float32[] 无界序列）：
//   [0..3]   Encapsulation Header（4 字节，CDR_LE）
//   [4..7]   数组长度 uint32 = 6（4 字节）
//   [8..31]  6 个 float32（每个 4 字节，共 24 字节）
//   合计 = 4 + 4 + 24 = 32 字节
// 注意：float 只要求 4 字节对齐，payload 起点 offset 为 4，4 % 4 == 0，
// 因此元素之间、长度字段与首元素之间均不需要 padding（不同于 double 版本）。
constexpr std::size_t kSerializedJointCommandSize = 32;

// 将六个关节角编码为 JointCommand 的 ROS 2 CDR 序列化消息。
// 这样不需要在本目标中直接包含自定义消息头文件。
rclcpp::SerializedMessage serializeJointCommand(
    const std::array<float, kJointCount> & joint_angles)
{
    // CDR 数据中 float32 必须是 IEEE-754 单精度 32 位格式。
    // 编译期断言：确保当前平台的 float 恰好 4 字节，且符合 IEEE-754。
    static_assert(sizeof(float) == sizeof(std::uint32_t));
    static_assert(std::numeric_limits<float>::is_iec559);

    // 依次写入 CDR 封装信息、数组长度以及六个 float 关节角。
    // 使用字节数组构建消息，避免依赖生成的 JointCommand C++ 类型。
    // 注意 bytes{} 会做值初始化，全部清零——这是后面“只写低字节”的前提。
    std::array<std::uint8_t, kSerializedJointCommandSize> bytes{};

    // [0..1] Encapsulation Header 的 Representation Identifier，按大端固定编码：
    //   0x0000 = CDR_BE（大端）
    //   0x0001 = CDR_LE（小端）——这里用 0x0001，表示后续 payload 为小端 CDR。
    // [2..3] Representation Options，通常为 0，保持默认清零即可。
    bytes[0] = 0;
    bytes[1] = 1;

    // [4..7] 无界序列（float32[]）在 CDR 中先写一个 uint32 元素个数。
    // payload 起点为 offset 4，uint32 需 4 字节对齐，4 % 4 == 0，无需 padding。
    // 小端下 6 = 0x00000006 → 字节序为 06 00 00 00。
    // 因为缓冲区已清零，只需写低字节即可（前提：kJointCount < 256）。
    bytes[4] = static_cast<std::uint8_t>(kJointCount);

    for (std::size_t i = 0; i < joint_angles.size(); ++i) {
        // 先把 float 的 4 个字节按位复制到 uint32_t，
        // 既避免 reinterpret_cast 的严格别名（strict aliasing）UB，
        // 也避免把 float 直接转整型（那会改变数值）。
        std::uint32_t bits = 0;
        std::memcpy(&bits, &joint_angles[i], sizeof(bits));

        // payload 起点为 offset 4，float 需 4 字节对齐，
        // 首元素实际整体偏移 = 4 + 4 = 8，之后每个元素间隔 4 字节。
        // 总长 = 8 + 6*4 = 32，与 kSerializedJointCommandSize 一致。
        const std::size_t offset = 8 + i * sizeof(bits);

        // 按小端顺序逐字节写出：不管宿主是大端还是小端，
        // 输出的字节序都固定为小端，符合 CDR_LE 约定。
        for (std::size_t byte = 0; byte < sizeof(bits); ++byte) {
            bytes[offset + byte] =
                static_cast<std::uint8_t>((bits >> (byte * 8)) & 0xFF);
        }
    }

    // 将构造好的字节数据交给 ROS 2 通用发布器。
    // SerializedMessage 承载已序列化字节流，可配合 GenericPublisher
    // 或 Publisher<rclcpp::SerializedMessage> 直接发送。
    rclcpp::SerializedMessage message(bytes.size());
    auto & serialized = message.get_rcl_serialized_message();
    std::memcpy(serialized.buffer, bytes.data(), bytes.size());
    serialized.buffer_length = bytes.size();
    return message;
}
}  // namespace

int main(int argc, char **argv)
{
    // 初始化 ROS 2，解析命令行中的 ROS 参数。
    rclcpp::init(argc, argv);

    // MoveIt 的规划参数通常由 launch 文件传入，因此允许节点读取外部参数。
    auto node = std::make_shared<rclcpp::Node>(
        "trajectory_example",
        rclcpp::NodeOptions().automatically_declare_parameters_from_overrides(true));
    // 发布 JointCommand 到串口节点订阅的话题，由串口节点负责写入单片机。
    auto joint_command_publisher = node->create_generic_publisher(
        "joint_command", "my_26robot_interfaces/msg/JointCommand", 10);

    // MoveGroupInterface 需要节点持续处理 ROS 通信，所以在后台线程 spin。
    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(node);
    std::thread spinner([&executor]() { executor.spin(); });

    int result = 0;
    {
        // "arm" 必须与你的 MoveIt SRDF 中的 planning group 名称一致。
        MoveGroupInterface arm(node, "arm");

        // 降低规划轨迹的最大速度和加速度，比例范围通常为 0 到 1。
        arm.setMaxVelocityScalingFactor(0.2);
        arm.setMaxAccelerationScalingFactor(0.2);

        // 从 MoveIt 的状态监视器获取机械臂当前状态。
        // 真实角度应由硬件反馈发布到 /joint_states，并且关节名称、顺序、
        // 弧度单位都要和 URDF / MoveIt 配置一致。
        auto current_state = arm.getCurrentState(5.0);
        if (!current_state) {
            RCLCPP_ERROR(
                node->get_logger(),
                "5 秒内没有获取到机械臂当前状态，请检查 /joint_states 和 MoveIt 状态监视器。");
            result = 1;
        } else {
            const auto *joint_model_group =
                current_state->getJointModelGroup("arm");
            if (joint_model_group == nullptr) {
                RCLCPP_ERROR(node->get_logger(), "找不到名为 arm 的关节组。");
                result = 1;
            } else {
                // 获取 MoveIt 关节组变量顺序及当前位置，角度单位为弧度。
                std::vector<double> current_angles;
                current_state->copyJointGroupPositions(
                    joint_model_group, current_angles);
                const auto &joint_names = joint_model_group->getVariableNames();

                // 打印当前关节位置，便于确认硬件状态和关节顺序。
                RCLCPP_INFO(node->get_logger(), "读取到当前关节角（单位：弧度）：");
                for (std::size_t i = 0; i < current_angles.size(); ++i) {
                    RCLCPP_INFO(
                        node->get_logger(),
                        "  %s = %.3f",
                        joint_names[i].c_str(),
                        current_angles[i]);
                }

                // 示例目标角度：必须按 MoveIt 打印的变量顺序填写，单位为弧度。
                const std::vector<double> target_angles = {
                    0.0, -0.5, 0.8, 0.0, 0.5, 0.0};

                if (target_angles.size() != joint_names.size()) {
                    RCLCPP_ERROR(
                        node->get_logger(),
                        "目标角度数量与 arm 关节变量数量不一致。");
                    result = 1;
                } else {
                    // 明确把刚读取到的真实状态作为规划起点。
                    arm.setStartState(*current_state);
                    if (!arm.setJointValueTarget(target_angles)) {
                        RCLCPP_ERROR(
                            node->get_logger(),
                            "目标角度超出关节限制或无法设置，请检查目标值和关节顺序。");
                        result = 1;
                    } else {
                        // 让规划器从刚读取到的机械臂实际位置开始计算轨迹。
                        MoveGroupInterface::Plan plan;
                        const auto plan_result = arm.plan(plan);
                        if (plan_result != moveit::core::MoveItErrorCode::SUCCESS) {
                            RCLCPP_ERROR(
                                node->get_logger(),
                                "轨迹规划失败；机械臂没有执行运动。");
                            result = 1;
                        } else {
                            // 取出规划生成的关节轨迹点；每个点包含位置和相对起始时间。
                            const auto &points = plan.trajectory_.joint_trajectory.points;
                            if (points.empty()) {
                                RCLCPP_ERROR(
                                    node->get_logger(),
                                    "规划结果中没有轨迹点，无法发送给单片机。");
                                result = 1;
                            } else if (joint_names.size() != kJointCount) {
                                RCLCPP_ERROR(
                                    node->get_logger(),
                                    "arm 关节组不是 6 轴，无法发送给单片机。");
                                result = 1;
                            } else {
                                // 先检查所有轨迹点，确保各点都有六个有效的关节角。
                                bool valid_trajectory = true;
                                for (const auto &point : points) {
                                    if (point.positions.size() != kJointCount) {
                                        valid_trajectory = false;
                                        break;
                                    }
                                    for (const auto angle : point.positions) {
                                        if (!std::isfinite(angle)) {
                                            valid_trajectory = false;
                                            break;
                                        }
                                    }
                                    if (!valid_trajectory) {
                                        break;
                                    }
                                }

                                if (!valid_trajectory) {
                                    RCLCPP_ERROR(
                                        node->get_logger(),
                                        "轨迹包含非六轴或非有限关节角，拒绝发送。");
                                    result = 1;
                                } else {
                                    // 串口节点订阅 joint_command，并将六轴弧度角打包发送给单片机。
                                    // 等待订阅节点上线，避免发布器启动过早导致首批数据无人接收。
                                    const auto subscriber_deadline =
                                        std::chrono::steady_clock::now() +
                                        std::chrono::seconds(5);
                                    while (rclcpp::ok() &&
                                           joint_command_publisher->get_subscription_count() == 0 &&
                                           std::chrono::steady_clock::now() < subscriber_deadline) {
                                        std::this_thread::sleep_for(
                                            std::chrono::milliseconds(100));
                                    }

                                    if (joint_command_publisher->get_subscription_count() == 0) {
                                        RCLCPP_ERROR(
                                            node->get_logger(),
                                            "5 秒内没有发现 joint_command 订阅者；请先启动单片机串口节点。");
                                        result = 1;
                                    } else {
                                        // 以开始发送时刻为基准，按规划时间调度每个轨迹点。
                                        const auto trajectory_start =
                                            std::chrono::steady_clock::now();
                                        for (const auto &point : points) {
                                            const auto point_time = std::chrono::seconds(
                                                point.time_from_start.sec) +
                                                std::chrono::nanoseconds(
                                                    point.time_from_start.nanosec);
                                            std::this_thread::sleep_until(
                                                trajectory_start + point_time);

                                            // 转为固定长度数组，编码后发布给串口节点。
                                            std::array<double, kJointCount> angles{};
                                            std::copy(
                                                point.positions.begin(),
                                                point.positions.end(),
                                                angles.begin());
                                            joint_command_publisher->publish(
                                                serializeJointCommand(angles));
                                        }

                                        RCLCPP_INFO(
                                            node->get_logger(),
                                            "MoveIt 轨迹已按时间顺序发送到单片机串口节点。");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    rclcpp::shutdown();
    if (spinner.joinable()) {
        spinner.join();
    }
    return result;
}
