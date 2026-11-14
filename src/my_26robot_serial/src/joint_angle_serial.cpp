#include "my_26robot_serial/joint_angle_serial.hpp"

#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>
#include <vector>

#include "my_26robot_serial/packet.hpp"

namespace my_26robot_serial
{
// ============================ 构造函数 ============================
JointAngleSerialNode::JointAngleSerialNode(const rclcpp::NodeOptions & options)
: Node("joint_angle_serial", options),
  // 单向只写，1 个 io 线程足够
  owned_ctx_{new drivers::common::IoContext(2)},
  serial_driver_{new drivers::serial_driver::SerialDriver(*owned_ctx_)}
{
  RCLCPP_INFO(get_logger(), "Start JointAngleSerialNode!");

  // 1) 读参数
  getParams();

  // 2) 打开串口
  try {
    openSerialPort();
  } catch (const std::exception & ex) {
    RCLCPP_ERROR(
      get_logger(), "Error creating serial port: %s - %s",
      device_name_.c_str(), ex.what());
    throw ex;
  }

  // 3) 心跳时间戳初始化（避免刚启动就误判超时）
  check_frequency_ = declare_parameter<double>("check_frequency", 0.5);
   // 超时阈值：一个检查周期（1/check_frequency 秒），超过即认为对端失联
  heartbeat_timeout_ = std::chrono::duration_cast<std::chrono::nanoseconds>(
    std::chrono::duration<double>(1.0 / check_frequency_));
  markSendActive();// 初始化发送数据的时间戳
  markReceiveActive();// 初始化接收数据的时间戳

  // 4) 健康检查定时器
  // 定时器：每个检查周期检测一次收发是否超时
  status_checker_ = this->create_wall_timer(
    std::chrono::milliseconds(static_cast<int>(1000.0 / check_frequency_)),
    [this]() {
      const auto now_ns = this->now().nanoseconds();
      const auto req_age_ns =
        now_ns - last_request_time_ns_.load(std::memory_order_relaxed);
      const auto send_age_ns =
        now_ns - last_send_time_ns_.load(std::memory_order_relaxed);

      if (send_age_ns > heartbeat_timeout_.count()) {
            // RCLCPP_ERROR(this->get_logger(), "Long time not send data!");
            // rclcpp::shutdown();
            // exit(0);
          }
      if (receive_age_ns > heartbeat_timeout_.count()) {
              RCLCPP_ERROR(this->get_logger(), "Long time not recieve data!");
          }
    });

  // verifySerialPortTransmission();// 取消注释，以验证串口发送是否正常

  // 5) 创建订阅者
  subscription_ = this->create_subscription<my_26robot_interfaces::msg::JointCommand>(
    "/send_pack", rclcpp::SensorDataQoS(),
    std::bind(&JointAngleSerialNode::infantrySendData, this, std::placeholders::_1));
  RCLCPP_INFO(get_logger(), "Start send pack!");
}

// ============================ 析构函数 ============================
JointAngleSerialNode::~JointAngleSerialNode()
{
  // 先关串口，再等 io_context 退出
  if (serial_driver_->port()->is_open()) {
    serial_driver_->port()->close();
  }
  if (owned_ctx_) {
    owned_ctx_->waitForExit();
  }
}

// ============================ 参数读取 ============================
// 设备名、波特率、流控、校验位、停止位，做字符串到枚举的映射和校验
void JointAngleSerialNode::getParams()
{
  using FlowControl = drivers::serial_driver::FlowControl;
  using Parity = drivers::serial_driver::Parity;
  using StopBits = drivers::serial_driver::StopBits;

  // 基础参数
  device_name_ = declare_parameter<std::string>("device_name", "/dev/ttyACM0");
  topic_name_ = declare_parameter<std::string>("topic_name", "joint_command");
  const int baud_rate = declare_parameter<int>("baud_rate", 115200);

  uint32_t baud_rate{};
  auto fc = FlowControl::NONE;
  auto pt = Parity::NONE;
  auto sb = StopBits::ONE;

  // 串口设备名，默认 /dev/ttyACM0
  try {
    device_name_ = declare_parameter<std::string>("device_name", "/dev/ttyACM0");
  } catch (rclcpp::ParameterTypeException & ex) {
    RCLCPP_ERROR(get_logger(), "The device name provided was invalid");
    throw ex;
  }

  // 波特率
  try {
    baud_rate = declare_parameter<int>("baud_rate", 0);
  } catch (rclcpp::ParameterTypeException & ex) {
    RCLCPP_ERROR(get_logger(), "The baud_rate provided was invalid");
    throw ex;
  }

  // 流控：none / hardware / software
  const auto fc_string = declare_parameter<std::string>("flow_control", "none");
  if (fc_string == "none") {
    fc = FlowControl::NONE;
  } else if (fc_string == "hardware") {
    fc = FlowControl::HARDWARE;
  } else if (fc_string == "software") {
    fc = FlowControl::SOFTWARE;
  } else {
    throw std::invalid_argument(
      "The flow_control parameter must be one of: none, software, or hardware.");
  }

  // 校验位：none / odd / even
  const auto pt_string = declare_parameter<std::string>("parity", "none");
  if (pt_string == "none") {
    pt = Parity::NONE;
  } else if (pt_string == "odd") {
    pt = Parity::ODD;
  } else if (pt_string == "even") {
    pt = Parity::EVEN;
  } else {
    throw std::invalid_argument(
      "The parity parameter must be one of: none, odd, or even.");
  }

  // 停止位：1 / 1.5 / 2
  const auto sb_string = declare_parameter<std::string>("stop_bits", "1");
  if (sb_string == "1" || sb_string == "1.0") {
    sb = StopBits::ONE;
  } else if (sb_string == "1.5") {
    sb = StopBits::ONE_POINT_FIVE;
  } else if (sb_string == "2" || sb_string == "2.0") {
    sb = StopBits::TWO;
  } else {
    throw std::invalid_argument(
      "The stop_bits parameter must be one of: 1, 1.5, or 2.");
  }

  // 组装串口配置对象
  device_config_ = std::make_unique<drivers::serial_driver::SerialPortConfig>(
    baud_rate, fc, pt, sb);
}

// ============================ 打开串口 ============================
void JointAngleSerialNode::openSerialPort()
{
  serial_driver_->init_port(device_name_, *device_config_);
  if (!serial_driver_->port()->is_open()) {
    serial_driver_->port()->open();
  }
}

// ==================== 重开串口 ====================
// 在 ROS 仍运行时循环重试：关闭 → 打开 → 成功返回；
// 失败则睡 1 秒继续重试
void JointAngleSerialNode::reopenPort()
{
  while (rclcpp::ok()) {
    RCLCPP_WARN(get_logger(), "Attempting to reopen port");
    try {
      if (serial_driver_->port()->is_open()) {
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

// ============================ 心跳标记 ============================
// 记录"刚刚收到过数据"的时间戳
void JointAngleSerialNode::markRequestActive()
{
  last_request_time_ns_.store(
    this->now().nanoseconds(), std::memory_order_relaxed);
}
// 记录"刚刚发送过数据"的时间戳（原子操作，接收/发送/定时器多线程可见）
void JointAngleSerialNode::markSendActive()
{
  last_send_time_ns_.store(
    this->now().nanoseconds(), std::memory_order_relaxed);
}

// 判断最近是否收到过数据（未超过超时阈值）
bool JointAngleSerialNode::hasRecentReceiveData() const
{
  // 检查是否最近有接收数据
  const auto now_ns = this->now().nanoseconds();
  const auto receive_age_ns = now_ns - last_receive_time_ns_.load(std::memory_order_relaxed);
  return receive_age_ns <= heartbeat_timeout_.count();
}

// 把"是否有近期数据"映射成状态码，供下位机判断上位机是否在线
uint8_t JointAngleSerialNode::currentReceiveStatusCode() const
{
  // 返回当前接收数据的状态码（正常/超时）
  return hasRecentReceiveData() ? kReceiveOkCode : kReceiveTimeoutCode;
}

// ============================ 发送回调 ============================
void JointAngleSerialNode::sendJointAngles(
  const my_26robot_interfaces::msg::JointCommand::SharedPtr msg)
{
  markRequestActive();
  
  JointAngleSendPacket packet ;
  const auto receive_status = currentReceiveStatusCode();

  // msg为空指针判断为测试模式
    if(receive_status == kReceiveOkCode && msg!= nullptr)
    {
      packet.joint1 = msg->joint1;
      packet.joint2 = msg->joint2;
      packet.joint3 = msg->joint3;
      packet.joint4 = msg->joint4;
      packet.joint5 = msg->joint5;
      packet.joint6 = msg->joint6;
      packet.if_recieve_data = receive_status;
     
      // 对 sizeof(packet)-1 个字节计算 CRC（最后 2 字节是 CRC 位）
      crc16::Append_CRC16_Check_Sum(reinterpret_cast<uint8_t *>(&packet), sizeof(packet)-1);
      std::vector<uint8_t> data = toVector(packet);   // 结构体 → 字节流
      serial_driver_->port()->send(data);
      }
      else if(receive_status == kReceiveTimeoutCode)
      {
        // 超时：发全 0，但状态码写"超时"
        packet.joint1 = 0;
        packet.joint2 = 0;
        packet.joint3 = 0;
        packet.joint4 = 0;
        packet.joint5 = 0;
        packet.joint6 = 0;
        packet.if_recieve_data = receive_status;
        
        crc16::Append_CRC16_Check_Sum(reinterpret_cast<uint8_t *>(&packet), sizeof(packet)-1);
        std::vector<uint8_t> data = toVector(packet);
        serial_driver_->port()->send(data);
        // rclcpp::shutdown();
        // exit(0);
      }
      // 计算并发布延迟（从消息 header.stamp 到现在的秒数 * 1000）
      std_msgs::msg::Float64 latency;
      latency.data = (this->now() - msg->header.stamp).seconds() * 1000.0;
      RCLCPP_DEBUG_STREAM(get_logger(), "Total latency: " + std::to_string(latency.data) + "ms");
      latency_pub_->publish(latency);
    } catch (const std::exception & ex) {
      RCLCPP_ERROR(get_logger(), "Error while sending data: %s", ex.what());
      reopenPort();
    }
}

// =============================== 接收线程 ====================================
// 流程：读 1 字节帧头 → 是 0xBE 就读剩余长度 → 拼成完整包
//       → 转结构体 → CRC 校验 → 发布 ROS 话题 + 广播 TF
// 异常处理：帧头不对/CRC 错累计到 invaild_count，超过 10 次重开串口
void JointAngleSerialNode::sentryReceiveData()
{
  RCLCPP_WARN(get_logger(), "sentryReceiveData!");
  std::vector<uint8_t> header(1);        // 只放帧头 1 字节
  std::vector<uint8_t> data;
  data.reserve(sizeof(SentryReceivePacket));

  while (rclcpp::ok()) 
  {
    try {
      // 先读 1 字节帧头
      serial_driver_->port()->receive(header);
      // 预分配除帧头外的剩余长度
      data.resize(sizeof(SentryReceivePacket)-1);
      // 读剩余字节（注意：这里在判断帧头之前就读了，帧头不对时数据已被消耗）
      serial_driver_->port()->receive(data);
      // 把帧头插回最前面，拼成完整一帧
      data.insert(data.begin(), header[0]);
      markReceiveActive();
      
      // printSerialPortContent(data);//取消注释，以打印串口数据
      if (header[0] == 0xBE) {
        // printSerialPortContent(data);//取消注释，以打印串口数据

        // 字节流 → 结构体（即前面分析的 fromVector）
        JointReceivePacket packet = fromVector<JointReceivePacket>(data);
        // RCLCPP_WARN(get_logger(),"packet size %ld",sizeof(packet)); // 取消注释，以查看发送包长度
        // CRC16 校验整包
        bool crc_ok =
          crc16::Verify_CRC16_Check_Sum(reinterpret_cast<const uint8_t *>(&packet), sizeof(packet));
        if (crc_ok) {
        // 用系统时间 + 偏移量作为时间戳（上下位机时钟对齐）
        timestamp_offset_ = this->get_parameter("timestamp_offset").as_double();
        rclcpp::Time now_time = rclcpp::Clock().now() + rclcpp::Duration::from_seconds(timestamp_offset_);
        my_26robot_interfaces::msg::RecieveData pub_pack;
        pub_pack.header.stamp = now_time;
        // 逐字段填充 ROS 消息
        pub_pack.joint1 = packet.joint1;
        pub_pack.joint2 = packet.joint2;
        pub_pack.joint3 = packet.joint3;
        pub_pack.joint4 = packet.joint4;
        pub_pack.joint5 = packet.joint5;
        pub_pack.joint6 = packet.joint6;
        
        recieve_pub_->publish(pub_pack);
        // RCLCPP_INFO(get_logger(), "publish recieve_pack!");

        invaild_count = 0;   // 成功解析一次就清空错误计数
        } else {
          RCLCPP_ERROR(get_logger(), "CRC error!");
        }
      } else {
        // 帧头不是 0xBE：节流告警 + 累计错误
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 20, "Invalid header: %02X", header[0]);
        invaild_count++;
        if(invaild_count > 10){
          reopenPort();
        }
      }
    } catch (const std::exception & ex) {
      // 串口异常：ROS 还在运行就告警并重开串口
      if (rclcpp::ok()) {
        RCLCPP_ERROR_THROTTLE(
          rclcpp::get_logger(), *get_clock(), 20, "Error while receiving data: %s", ex.what());
        reopenPort();
      }
    }
  }
}



#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(my_26robot_serial::JointAngleSerialNode)