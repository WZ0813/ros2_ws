// Copyright (c) 2022 ChenJun
// Licensed under the Apache-2.0 License.

#ifndef RM_SERIAL_DRIVER__RM_SERIAL_DRIVER_HPP_
#define RM_SERIAL_DRIVER__RM_SERIAL_DRIVER_HPP_

#include <tf2_ros/transform_broadcaster.h>

#include <geometry_msgs/msg/transform_stamped.hpp>
#include <my_26robot_interfaces/msg/j_joint_command.hpp>
#include <rclcpp/publisher.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp/subscription.hpp>
#include <serial_driver/serial_driver.hpp>
#include <std_msgs/msg/header.hpp>
#include <std_msgs/msg/float64.hpp>
#include <std_srvs/srv/trigger.hpp>
#include <visualization_msgs/msg/marker.hpp>

// C++ system
#include <cstdint>
#include <future>
#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace rm_serial_driver
{
  enum MODE
{
  SENTRY_MODE,
  INFANTRY_MODE,
  HERO_MODE
};
class RMSerialDriver : public rclcpp::Node
{
public:
  explicit RMSerialDriver(const rclcpp::NodeOptions & options);

  ~RMSerialDriver() override;

private:
  void getParams();

  void ArmReceiveData();

  void ArmSendData(const my_26robot_interfaces::msg::JJointCommand::SharedPtr msg);

  void reopenPort();

  void printSerialPortContent(const std::vector<uint8_t> & data);

  void verifySerialPortTransmission();

  void markSendActive();

  void markReceiveActive();

  bool hasRecentReceiveData() const;

  uint8_t currentReceiveStatusCode() const;

  // Serial port
  std::unique_ptr<IoContext> owned_ctx_;
  std::string device_name_;
  std::unique_ptr<drivers::serial_driver::SerialPortConfig> device_config_;
  std::unique_ptr<drivers::serial_driver::SerialDriver> serial_driver_;

  // Param client to set detect_colr
  using ResultFuturePtr = std::shared_future<std::vector<rcl_interfaces::msg::SetParametersResult>>;
  bool initial_set_param_ = false;
  uint8_t previous_receive_color_ = 0;
  rclcpp::AsyncParametersClient::SharedPtr detector_param_client_;
  ResultFuturePtr set_param_future_;

  // Service client to reset tracker
  rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr reset_tracker_client_;

  // Broadcast tf from odom_aim to gimbal_link
  double timestamp_offset_ = 0;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

  rclcpp::Subscription<my_26robot_interfaces::msg::JJointCommand>::SharedPtr target_sub_;

  // For debug usage
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr latency_pub_;
  rclcpp::Publisher<my_26robot_interfaces::msg::JJointCommand>::SharedPtr joint_pub_;
  rclcpp::Publisher<std_msgs::msg::Header>::SharedPtr send_ack_pub_;


  std::thread receive_thread_;
  std::thread send_thread_;

  std::atomic<std::int64_t> last_send_time_ns_{0};
  std::atomic<std::int64_t> last_receive_time_ns_{0};
  double check_frequency;
  rclcpp::TimerBase::SharedPtr status_checker;
  std::chrono::nanoseconds heartbeat_timeout_{0};/*  */
  int program_mode;
  int invaild_count = 0;

  static constexpr uint8_t kReceiveOkCode = 66;
  static constexpr uint8_t kReceiveTimeoutCode = 100;
};
}  // namespace rm_serial_driver

#endif  // RM_SERIAL_DRIVER__RM_SERIAL_DRIVER_HPP_
