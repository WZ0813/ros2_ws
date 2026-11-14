#ifndef MY_26ROBOT_SERIAL__JOINT_ANGLE_SERIAL_NODE_HPP_
#define MY_26ROBOT_SERIAL__JOINT_ANGLE_SERIAL_NODE_HPP_

#include <atomic>
#include <array>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <io_context/io_context.hpp>
#include <rclcpp/rclcpp.hpp>
// ★ 修正：原来这里自 include 了自己，改成串口驱动头
#include <serial_driver/serial_driver.hpp>

#include <trajectory_msgs/msg/joint_trajectory.hpp>

namespace my_26robot_serial
{

// 关节角串口下发节点（rm_serial_driver 风格的单向发送版）。
// 结构对照 rm_serial_driver：
//   - owned_ctx_ / serial_driver_ / device_config_ 字段位置一致
//   - getParams() / openSerialPort() / reopenPort() 划分一致
//   - 心跳时间戳 + status_checker 定时器一致（改为检测"发超时"）
//   - 发送失败 catch -> reopenPort() 一致
class JointAngleSerialNode : public rclcpp::Node
{
public:
  explicit JointAngleSerialNode(
    const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
  ~JointAngleSerialNode() override;

private:
  // ---------- 初始化 / 配置 ----------
  void getParams();          // 读 ROS 参数，组装 SerialPortConfig
  void openSerialPort();     // init_port + open
  void reopenPort();         // close -> open，失败 1s 重试（对应 rm 同名函数）

  // ---------- 业务回调 ----------
  void receiveJointTrajectory(
    const trajectory_msgs::msg::JointTrajectory::SharedPtr msg);
  void sendTrajectorySample();
  void writeJointAngles(const std::array<double, 6> & angles);

  struct TrajectorySample
  {
    std::chrono::nanoseconds time_from_start;
    std::array<double, 6> positions;
  };

  // ---------- 心跳 ----------
  void markRequestActive();  // 收到订阅消息时刷新
  void markSendActive();     // 串口发送成功后刷新

  // ---------- 成员（声明顺序 = 初始化顺序，与 rm 保持一致）----------
  std::unique_ptr<drivers::common::IoContext> owned_ctx_;
  std::unique_ptr<drivers::serial_driver::SerialDriver> serial_driver_;
  std::unique_ptr<drivers::serial_driver::SerialPortConfig> device_config_;

  std::string device_name_;
  std::string trajectory_topic_name_;

  double check_frequency_{0.5};
  std::chrono::nanoseconds heartbeat_timeout_{};
  std::atomic<int64_t> last_request_time_ns_{0};
  std::atomic<int64_t> last_send_time_ns_{0};

  // 多线程执行器下防止两帧 write 交错
  std::mutex write_mutex_;

  std::mutex trajectory_mutex_;
  std::vector<TrajectorySample> trajectory_samples_;
  std::chrono::steady_clock::time_point trajectory_start_time_{};
  bool trajectory_active_{false};

  rclcpp::Subscription<trajectory_msgs::msg::JointTrajectory>::SharedPtr
    trajectory_subscription_;
  rclcpp::TimerBase::SharedPtr trajectory_timer_;
  rclcpp::TimerBase::SharedPtr status_checker_;
};

}  // namespace my_26robot_serial

#endif  // MY_26ROBOT_SERIAL__JOINT_ANGLE_SERIAL_NODE_HPP_
