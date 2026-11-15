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
#include <serial_driver/serial_driver.hpp>
#include <trajectory_msgs/msg/joint_trajectory.hpp>

namespace my_26robot_serial
{
class JointAngleSerialNode : public rclcpp::Node
{
public:
  explicit JointAngleSerialNode(
    const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
  ~JointAngleSerialNode() override;

private:
  void getParams();
  void openSerialPort();
  void reopenPort();

  void receiveJointTrajectory(
    const trajectory_msgs::msg::JointTrajectory::SharedPtr msg);
  void sendTrajectorySample();
  void writeJointAngles(const std::array<double, 6> & angles);

  struct TrajectorySample
  {
    std::chrono::nanoseconds time_from_start;
    std::array<double, 6> positions;
  };

  void markRequestActive();
  void markSendActive();
  void markReceiveActive();
  bool hasRecentReceiveData() const;
  uint8_t currentReceiveStatusCode() const;

  std::unique_ptr<drivers::common::IoContext> owned_ctx_;
  std::unique_ptr<drivers::serial_driver::SerialDriver> serial_driver_;
  std::unique_ptr<drivers::serial_driver::SerialPortConfig> device_config_;

  std::string device_name_;
  std::string trajectory_topic_name_;

  double check_frequency_{0.5};
  std::chrono::nanoseconds heartbeat_timeout_{0};
  std::atomic<int64_t> last_request_time_ns_{0};
  std::atomic<int64_t> last_send_time_ns_{0};
  std::atomic<int64_t> last_receive_time_ns_{0};

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
