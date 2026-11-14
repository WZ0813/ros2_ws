#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <rclcpp/rclcpp.hpp>
#include <example_interfaces/msg/bool.hpp>
#include <my_26robot_interfaces/msg/joint_command.hpp>
#include <my_26robot_interfaces/msg/pose_command.hpp>

namespace
{
// 使用订阅者对应的消息类型，确保话题数据结构一致。
using Bool = example_interfaces::msg::Bool;
using JointCmd = my_26robot_interfaces::msg::JointCommand;
using PoseCmd = my_26robot_interfaces::msg::PoseCommand;

// 参数格式错误时输出命令用法。
void printUsage(const char * executable)
{
  std::cerr
    << "Usage:\n"
    << "  " << executable << " joint <j1> <j2> <j3> <j4> <j5> <j6>\n"
    << "  " << executable << " pose <x> <y> <z> <roll> <pitch> <yaw> [cartesian]\n"
    << "  " << executable << " gripper <open|close>\n"
    << "Angles are in radians; cartesian accepts true/false or 1/0.\n";
}

// 将命令行字符串解析为有限的浮点数，拒绝无效值和非数值。
bool parseDouble(const std::string & text, double & value)
{
  try {
    std::size_t parsed = 0;
    value = std::stod(text, &parsed);
    return parsed == text.size() && std::isfinite(value);
  } catch (const std::exception &) {
    return false;
  }
}

// 解析布尔参数，支持 true/false 和 1/0 两种写法。
bool parseBool(const std::string & text, bool & value)
{
  if (text == "true" || text == "1") {
    value = true;
    return true;
  }
  if (text == "false" || text == "0") {
    value = false;
    return true;
  }
  return false;
}

template<typename PublisherT, typename MessageT>
bool publishOnce(
  const PublisherT & publisher, const MessageT & message,
  const rclcpp::Node::SharedPtr & node, const std::string & topic)
{
  // 等待订阅端发现发布者，避免节点刚启动时消息因无人接收而丢失。
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
  while (rclcpp::ok() && publisher->get_subscription_count() == 0 &&
    std::chrono::steady_clock::now() < deadline)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  if (publisher->get_subscription_count() == 0) {
    RCLCPP_ERROR(node->get_logger(), "No subscriber found on '%s' after 5 seconds.", topic.c_str());
    return false;
  }

  // 一次性发布后短暂等待，让 ROS 2 有时间完成消息传输。
  publisher->publish(message);
  RCLCPP_INFO(node->get_logger(), "Published one command to '%s'.", topic.c_str());
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  return true;
}
}  // namespace

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  // 移除 ROS 自身参数，后续只处理用户传入的发布命令及其参数。
  const auto arguments = rclcpp::remove_ros_arguments(argc, argv);
  const auto fail = [&]() {
      printUsage(argv[0]);
      rclcpp::shutdown();
      return 2;
    };

  if (arguments.size() < 2) {
    return fail();
  }

  auto node = std::make_shared<rclcpp::Node>("command_publisher");
  const auto & command = arguments[1];
  bool published = false;

  // 关节订阅者要求恰好六个关节角，按弧度填写。
  if (command == "joint" && arguments.size() == 8) {
    JointCmd message;
    for (std::size_t i = 0; i < 6; ++i) {
      double angle = 0.0;
      if (!parseDouble(arguments[i + 2], angle)) {
        std::cerr << "Invalid joint angle: " << arguments[i + 2] << '\n';
        rclcpp::shutdown();
        return fail();
      }
      message.joint_angles.push_back(angle);
    }
    auto publisher = node->create_publisher<JointCmd>("joint_command", 10);
    published = publishOnce(publisher, message, node, "joint_command");
  // 位姿参数依次为位置 xyz 和欧拉角 roll/pitch/yaw，欧拉角单位为弧度。
  } else if (command == "pose" && (arguments.size() == 8 || arguments.size() == 9)) {
    PoseCmd message;
    double * fields[] = {
      &message.x, &message.y, &message.z, &message.roll, &message.pitch, &message.yaw};
    for (std::size_t i = 0; i < 6; ++i) {
      if (!parseDouble(arguments[i + 2], *fields[i])) {
        std::cerr << "Invalid pose value: " << arguments[i + 2] << '\n';
        rclcpp::shutdown();
        return fail();
      }
    }
    if (arguments.size() == 9 && !parseBool(arguments[8], message.cartesian_path)) {
      std::cerr << "Invalid cartesian value: " << arguments[8] << '\n';
      rclcpp::shutdown();
      return fail();
    }
    auto publisher = node->create_publisher<PoseCmd>("pose_command", 10);
    published = publishOnce(publisher, message, node, "pose_command");
  // 夹爪订阅者将 true 解释为打开，将 false 解释为关闭。
  } else if (command == "gripper" && arguments.size() == 3) {
    Bool message;
    if (arguments[2] == "open") {
      message.data = true;
    } else if (arguments[2] == "close") {
      message.data = false;
    } else {
      std::cerr << "Gripper action must be 'open' or 'close'.\n";
      rclcpp::shutdown();
      return fail();
    }
    auto publisher = node->create_publisher<Bool>("open_gripper", 10);
    published = publishOnce(publisher, message, node, "open_gripper");
  } else {
    return fail();
  }

  rclcpp::shutdown();
  return published ? 0 : 1;
}
