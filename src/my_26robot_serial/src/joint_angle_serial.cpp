#include "my_26robot_serial/joint_angle_serial.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>
#include <vector>

#include "my_26robot_serial/packet.hpp"
#include "rclcpp_components/register_node_macro.hpp"

namespace my_26robot_serial
{
JointAngleSerialNode::JointAngleSerialNode(const rclcpp::NodeOptions & options)
: Node("joint_angle_serial", options),
  owned_ctx_{new drivers::common::IoContext(2)},
  serial_driver_{new drivers::serial_driver::SerialDriver(*owned_ctx_)}
{
  RCLCPP_INFO(get_logger(), "Start JointAngleSerialNode!");

  getParams();

  try {
    openSerialPort();
  } catch (const std::exception & ex) {
    RCLCPP_ERROR(
      get_logger(), "Error creating serial port: %s - %s",
      device_name_.c_str(), ex.what());
    throw;
  }

  check_frequency_ = declare_parameter<double>("check_frequency", 0.5);
  heartbeat_timeout_ = std::chrono::duration_cast<std::chrono::nanoseconds>(
    std::chrono::duration<double>(1.0 / std::max(check_frequency_, 0.001)));
  markSendActive();
  markReceiveActive();

  const auto timer_period_ms = std::max(1, static_cast<int>(1000.0 / std::max(check_frequency_, 1.0)));
  status_checker_ = this->create_wall_timer(
    std::chrono::milliseconds(timer_period_ms),
    [this]() {
      const auto now_ns = this->now().nanoseconds();
      const auto request_age_ns =
        now_ns - last_request_time_ns_.load(std::memory_order_relaxed);
      const auto receive_age_ns =
        now_ns - last_receive_time_ns_.load(std::memory_order_relaxed);

      if (request_age_ns > heartbeat_timeout_.count()) {
        RCLCPP_WARN_THROTTLE(
          get_logger(), *get_clock(), 2000,
          "No request message received for %.2f s.",
          static_cast<double>(request_age_ns) / 1e9);
      }
      if (receive_age_ns > heartbeat_timeout_.count()) {
        RCLCPP_WARN_THROTTLE(
          get_logger(), *get_clock(), 2000,
          "No recent serial traffic for %.2f s.",
          static_cast<double>(receive_age_ns) / 1e9);
      }
    });

  trajectory_subscription_ = this->create_subscription<trajectory_msgs::msg::JointTrajectory>(
    trajectory_topic_name_, rclcpp::SensorDataQoS(),
    std::bind(&JointAngleSerialNode::receiveJointTrajectory, this, std::placeholders::_1));

  trajectory_timer_ = this->create_wall_timer(
    std::chrono::milliseconds(timer_period_ms),
    std::bind(&JointAngleSerialNode::sendTrajectorySample, this));

  RCLCPP_INFO(get_logger(), "Ready to stream joint trajectory data.");
}

JointAngleSerialNode::~JointAngleSerialNode()
{
  if (serial_driver_ && serial_driver_->port() && serial_driver_->port()->is_open()) {
    serial_driver_->port()->close();
  }
  if (owned_ctx_) {
    owned_ctx_->waitForExit();
  }
}

void JointAngleSerialNode::getParams()
{
  using FlowControl = drivers::serial_driver::FlowControl;
  using Parity = drivers::serial_driver::Parity;
  using StopBits = drivers::serial_driver::StopBits;

  device_name_ = declare_parameter<std::string>("device_name", "/dev/ttyACM0");
  trajectory_topic_name_ = declare_parameter<std::string>("trajectory_topic", "joint_trajectory_command");

  const auto baud_rate = declare_parameter<int>("baud_rate", 115200);
  const auto fc_string = declare_parameter<std::string>("flow_control", "none");
  const auto pt_string = declare_parameter<std::string>("parity", "none");
  const auto sb_string = declare_parameter<std::string>("stop_bits", "1");

  auto fc = FlowControl::NONE;
  auto pt = Parity::NONE;
  auto sb = StopBits::ONE;

  if (fc_string == "none") {
    fc = FlowControl::NONE;
  } else if (fc_string == "hardware") {
    fc = FlowControl::HARDWARE;
  } else if (fc_string == "software") {
    fc = FlowControl::SOFTWARE;
  } else {
    throw std::invalid_argument("The flow_control parameter must be one of: none, software, or hardware.");
  }

  if (pt_string == "none") {
    pt = Parity::NONE;
  } else if (pt_string == "odd") {
    pt = Parity::ODD;
  } else if (pt_string == "even") {
    pt = Parity::EVEN;
  } else {
    throw std::invalid_argument("The parity parameter must be one of: none, odd, or even.");
  }

  if (sb_string == "1" || sb_string == "1.0") {
    sb = StopBits::ONE;
  } else if (sb_string == "1.5") {
    sb = StopBits::ONE_POINT_FIVE;
  } else if (sb_string == "2" || sb_string == "2.0") {
    sb = StopBits::TWO;
  } else {
    throw std::invalid_argument("The stop_bits parameter must be one of: 1, 1.5, or 2.");
  }

  device_config_ = std::make_unique<drivers::serial_driver::SerialPortConfig>(
    static_cast<uint32_t>(baud_rate), fc, pt, sb);
}

void JointAngleSerialNode::openSerialPort()
{
  serial_driver_->init_port(device_name_, *device_config_);
  if (!serial_driver_->port()->is_open()) {
    serial_driver_->port()->open();
  }
}

void JointAngleSerialNode::reopenPort()
{
  while (rclcpp::ok()) {
    RCLCPP_WARN(get_logger(), "Attempting to reopen port");
    try {
      if (serial_driver_ && serial_driver_->port() && serial_driver_->port()->is_open()) {
        serial_driver_->port()->close();
      }
      serial_driver_->port()->open();
      RCLCPP_INFO(get_logger(), "Successfully reopened port");
      return;
    } catch (const std::exception & ex) {
      RCLCPP_ERROR(get_logger(), "Error while reopening port: %s", ex.what());
      rclcpp::sleep_for(std::chrono::seconds(1));
    }
  }
}

void JointAngleSerialNode::markRequestActive()
{
  last_request_time_ns_.store(this->now().nanoseconds(), std::memory_order_relaxed);
}

void JointAngleSerialNode::markSendActive()
{
  last_send_time_ns_.store(this->now().nanoseconds(), std::memory_order_relaxed);
}

void JointAngleSerialNode::markReceiveActive()
{
  last_receive_time_ns_.store(this->now().nanoseconds(), std::memory_order_relaxed);
}

bool JointAngleSerialNode::hasRecentReceiveData() const
{
  const auto now_ns = this->now().nanoseconds();
  const auto receive_age_ns = now_ns - last_receive_time_ns_.load(std::memory_order_relaxed);
  return receive_age_ns <= heartbeat_timeout_.count();
}

uint8_t JointAngleSerialNode::currentReceiveStatusCode() const
{
  return hasRecentReceiveData() ? 66U : 100U;
}

void JointAngleSerialNode::receiveJointTrajectory(
  const trajectory_msgs::msg::JointTrajectory::SharedPtr msg)
{
  if (!msg) {
    return;
  }

  std::lock_guard<std::mutex> lock(trajectory_mutex_);
  trajectory_samples_.clear();
  trajectory_active_ = !msg->points.empty();
  trajectory_start_time_ = std::chrono::steady_clock::now();

  for (const auto & point : msg->points) {
    TrajectorySample sample;
    const auto duration = rclcpp::Duration(point.time_from_start);
    sample.time_from_start = std::chrono::nanoseconds(duration.nanoseconds());
    sample.positions.fill(0.0);

    const auto count = std::min<std::size_t>(6U, point.positions.size());
    for (std::size_t i = 0; i < count; ++i) {
      sample.positions[i] = point.positions[i];
    }

    trajectory_samples_.push_back(sample);
  }

  if (trajectory_samples_.empty()) {
    trajectory_active_ = false;
    return;
  }

  markRequestActive();
  markReceiveActive();
}

void JointAngleSerialNode::sendTrajectorySample()
{
  std::lock_guard<std::mutex> lock(trajectory_mutex_);
  if (!trajectory_active_ || trajectory_samples_.empty()) {
    return;
  }

  const auto elapsed = std::chrono::steady_clock::now() - trajectory_start_time_;
  auto it = std::find_if(
    trajectory_samples_.begin(), trajectory_samples_.end(),
    [elapsed](const TrajectorySample & sample) {
      return sample.time_from_start >= elapsed;
    });

  if (it == trajectory_samples_.end()) {
    it = std::prev(trajectory_samples_.end());
    trajectory_active_ = false;
  }

  writeJointAngles(it->positions);
}

void JointAngleSerialNode::writeJointAngles(const std::array<double, 6> & angles)
{
  std::lock_guard<std::mutex> lock(write_mutex_);

  JointAngleSendPacket packet{};
  packet.header = 0xBE;
  packet.joint1 = static_cast<float>(angles[0]);
  packet.joint2 = static_cast<float>(angles[1]);
  packet.joint3 = static_cast<float>(angles[2]);
  packet.joint4 = static_cast<float>(angles[3]);
  packet.joint5 = static_cast<float>(angles[4]);
  packet.joint6 = static_cast<float>(angles[5]);
  packet.if_recieve_data = currentReceiveStatusCode();
  packet.tail = 0xED;

  crc16::Append_CRC16_Check_Sum(
    reinterpret_cast<uint8_t *>(&packet), sizeof(packet) - sizeof(packet.tail));

  try {
    serial_driver_->port()->send(toVector(packet));
    markSendActive();
  } catch (const std::exception & ex) {
    RCLCPP_ERROR(get_logger(), "Error while sending data: %s", ex.what());
    reopenPort();
  }
}

}  // namespace my_26robot_serial

RCLCPP_COMPONENTS_REGISTER_NODE(my_26robot_serial::JointAngleSerialNode)