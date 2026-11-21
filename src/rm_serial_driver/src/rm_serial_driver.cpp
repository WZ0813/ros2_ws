// Copyright (c) 2022 ChenJun
// Licensed under the Apache-2.0 License.

// tf2 四元数（用于把 RPY 转成 ROS 的 quaternion 消息）
#include <tf2/LinearMath/Quaternion.h>

// ROS2 日志、QoS、工具函数
#include <rclcpp/logging.hpp>
#include <rclcpp/qos.hpp>
#include <rclcpp/utilities.hpp>
// 串口驱动库（io_context / serial_driver / serial_port）
#include <serial_driver/serial_driver.hpp>
// tf2 与 geometry_msgs 的转换工具（tf2::toMsg 等）
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

// C++ 系统头
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <unistd.h>  // access() 用于检查串口设备读写权限

// 项目内部头：CRC16 校验、串口包结构体、本节点类声明、日志封装
#include "rm_serial_driver/crc.hpp"
#include "rm_serial_driver/packet.hpp"
#include "rm_serial_driver/rm_serial_driver.hpp"

// 组件注册宏：缺少它时生成的可执行文件会静默空转，节点永远不会被创建
#include "rclcpp_components/register_node_macro.hpp"

#include <my_26robot_interfaces/msg/j_joint_command.hpp>
#include <my_26robot_interfaces/msg/pose_command.hpp>

namespace rm_serial_driver
{
// =========================================================== 构造函数 =====================================================
RMSerialDriver::RMSerialDriver(const rclcpp::NodeOptions & options)
: Node("rm_serial_driver", options),
  owned_ctx_{new IoContext(2)},
  serial_driver_{new drivers::serial_driver::SerialDriver(*owned_ctx_)}
{
  RCLCPP_INFO(get_logger(), "Start RMSerialDriver!");

  // 读取串口参数（设备名、波特率、流控、校验、停止位）
  getParams();

  timestamp_offset_ = this->declare_parameter("timestamp_offset", 0.0);

   // 创建发布者：
  //   /latency      —— 发布端到端延迟（毫秒）
  //   /recieve_pack —— 发布从下位机解析出的状态数据
  latency_pub_ = this->create_publisher<std_msgs::msg::Float64>("/latency", 10);
  send_ack_pub_ = this->create_publisher<std_msgs::msg::Header>(
    "/send_pack_ack", rclcpp::SensorDataQoS());
  joint_pub_ = this->create_publisher<my_26robot_interfaces::msg::JJointCommand>(
        "/recieve_pack", rclcpp::SensorDataQoS()
    );
    // 初始化串口并启动接收线程
  try {
    // ==================== cout 调试：定位串口打开失败的原因 ====================
    std::cout << "[serial DEBUG] 使用设备: " << device_name_ << std::endl;
    std::cout << "[serial DEBUG] 设备文件存在: "
              << (std::filesystem::exists(device_name_) ? "YES" : "NO (设备未连接/未上电/名称错误)")
              << std::endl;
    if (std::filesystem::exists(device_name_)) {
      int acc = access(device_name_.c_str(), R_OK | W_OK);
      if (acc == 0) {
        std::cout << "[serial DEBUG] 读写权限: OK" << std::endl;
      } else {
        std::cout << "[serial DEBUG] 读写权限: DENIED (当前用户不在 dialout 组? "
                     "试试 sudo usermod -aG dialout $USER 后重新登录)" << std::endl;
      }
    }
    // ==========================================================================
    serial_driver_->init_port(device_name_, *device_config_);
    if (!serial_driver_->port()->is_open())
    {
      serial_driver_->port()->open();
      receive_thread_ = std::thread(&RMSerialDriver::ArmReceiveData, this);
      RCLCPP_INFO(get_logger(), "Start receive pack!");
    }
  } catch (const std::exception & ex) {
    std::cout << "[serial DEBUG] 串口打开失败! 异常信息: " << ex.what() << std::endl;
    RCLCPP_ERROR(
      get_logger(), "Error creating serial port: %s - %s", device_name_.c_str(), ex.what());
    throw ex;
  } 
  // ================================================= 心跳/超时监控 ============================================
  check_frequency = declare_parameter<double>("check_frequency", 0.5);
  // 超时阈值：一个检查周期（1/check_frequency 秒），超过即认为对端失联
  heartbeat_timeout_ = std::chrono::duration_cast<std::chrono::nanoseconds>(
    std::chrono::duration<double>(1.0 / check_frequency));

  markSendActive();// 初始化发送数据的时间戳
  markReceiveActive();// 初始化接收数据的时间戳

  // 定时器：每个检查周期检测一次收发是否超时
  status_checker = this->create_wall_timer(
      std::chrono::milliseconds(int(1000/check_frequency)), 
      [this](){
          const auto now_ns = this->now().nanoseconds();
          const auto send_age_ns = now_ns - last_send_time_ns_.load(std::memory_order_relaxed);// 计算发送数据的时间间隔
          if (send_age_ns > heartbeat_timeout_.count()) {
            RCLCPP_ERROR(this->get_logger(), "Long time not send data!");
            // rclcpp::shutdown();
            // exit(0);
          }
          const auto receive_age_ns = now_ns - last_receive_time_ns_.load(std::memory_order_relaxed);// 计算接收数据的时间间隔
          if (receive_age_ns > heartbeat_timeout_.count()) {
              RCLCPP_ERROR(this->get_logger(), "Long time not recieve data!");
          }
      }
  );


  // verifySerialPortTransmission();// 取消注释，以验证串口发送是否正常

  // 创建订阅者：收到 /send_pack 后按运行模式调用对应发送函数
    target_sub_ = this->create_subscription<my_26robot_interfaces::msg::JJointCommand>(
    "/send_pack", rclcpp::SensorDataQoS(),
    std::bind(&RMSerialDriver::ArmSendData, this, std::placeholders::_1));

    RCLCPP_INFO(get_logger(), "Start send pack!");
}


// ================================================== 析构函数 ==============================================
// 顺序：先关串口（避免线程还在读写）→ 等线程 join → 等 io_context 退出
RMSerialDriver::~RMSerialDriver()
{  
  //先关闭串口，确保线程安全
  if (serial_driver_->port()->is_open()) {
    serial_driver_->port()->close();
  }

  if(send_thread_.joinable()){
    send_thread_.join();
  }
  if (receive_thread_.joinable()) {
    receive_thread_.join();
  }

  if (owned_ctx_) {
    owned_ctx_->waitForExit();
  }
}


// 记录"刚刚发送过数据"的时间戳（原子操作，接收/发送/定时器多线程可见）
void RMSerialDriver::markSendActive()
{
  // 更新发送数据的时间戳
  last_send_time_ns_.store(this->now().nanoseconds(), std::memory_order_relaxed);
}

// 记录"刚刚收到过数据"的时间戳
void RMSerialDriver::markReceiveActive()
{
  // 更新接收数据的时间戳
  last_receive_time_ns_.store(this->now().nanoseconds(), std::memory_order_relaxed);
}

// 判断最近是否收到过数据（未超过超时阈值）
bool RMSerialDriver::hasRecentReceiveData() const
{
  // 检查是否最近有接收数据
  const auto now_ns = this->now().nanoseconds();
  const auto receive_age_ns = now_ns - last_receive_time_ns_.load(std::memory_order_relaxed);
  return receive_age_ns <= heartbeat_timeout_.count();
}

// 把"是否有近期数据"映射成状态码，供下位机判断上位机是否在线
uint8_t RMSerialDriver::currentReceiveStatusCode() const
{
  // 返回当前接收数据的状态码（正常/超时）
  return hasRecentReceiveData() ? kReceiveOkCode : kReceiveTimeoutCode;
}

// ============================================ 接收线程 ========================================
// 流程：读 1 字节帧头 → 是 0xBE 就读剩余长度 → 拼成完整包
//       → 转结构体 → CRC 校验 → 发布 ROS 话题 + 广播 TF
// 异常处理：帧头不对/CRC 错累计到 invaild_count，超过 10 次重开串口
void RMSerialDriver::ArmReceiveData()
{
  RCLCPP_WARN(get_logger(), "ArmReceiveData!");
  std::vector<uint8_t> header(1);        // 只放帧头 1 字节
  std::vector<uint8_t> data;
  data.reserve(sizeof(ArmReceivePacket));

  while (rclcpp::ok()) {
    try {
      // 先读 1 字节帧头
      serial_driver_->port()->receive(header);
      // 预分配除帧头外的剩余长度
      data.resize(sizeof(ArmReceivePacket)-1);
      // 读剩余字节（注意：这里在判断帧头之前就读了，帧头不对时数据已被消耗）
      serial_driver_->port()->receive(data);
      // 把帧头插回最前面，拼成完整一帧
      data.insert(data.begin(), header[0]);
      markReceiveActive();
      
      printSerialPortContent(data);//取消注释，以打印串口数据
      if (header[0] == 0xBE) {
        // printSerialPortContent(data);//取消注释，以打印串口数据

        // 字节流 → 结构体（即前面分析的 fromVector）
        ArmReceivePacket packet = fromVector<ArmReceivePacket>(data);
        RCLCPP_DEBUG(get_logger(), "packet size %zu", sizeof(packet));

        timestamp_offset_ = this->get_parameter("timestamp_offset").as_double();
        my_26robot_interfaces::msg::JJointCommand pub_pack;
        pub_pack.header.stamp = this->now() + rclcpp::Duration::from_seconds(timestamp_offset_);
        pub_pack.header.frame_id = "base_link";
        pub_pack.joint_angles[0] = packet.joint1;
        pub_pack.joint_angles[1] = packet.joint2;
        pub_pack.joint_angles[2] = packet.joint3;
        pub_pack.joint_angles[3] = packet.joint4;
        pub_pack.joint_angles[4] = packet.joint5;
        pub_pack.joint_angles[5] = packet.joint6;
        pub_pack.mode = packet.mode;

        RCLCPP_INFO(
          get_logger(),
          "pub_pack: mode=%u angles=[%.6f, %.6f, %.6f, %.6f, %.6f, %.6f]",
          static_cast<unsigned int>(pub_pack.mode),
          pub_pack.joint_angles[0], pub_pack.joint_angles[1], pub_pack.joint_angles[2],
          pub_pack.joint_angles[3], pub_pack.joint_angles[4], pub_pack.joint_angles[5]);

        joint_pub_->publish(pub_pack);

        invaild_count = 0;
        
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
          get_logger(), *get_clock(), 20, "Error while receiving data: %s", ex.what());
        reopenPort();
      }
    }
  }
}

// ===================================================== 发送回调 ===============================================================
// 收到 /send_pack 后组包：若最近有收到下位机数据，正常填写；
// 否则填 0 + 超时状态码，告诉下位机"上位机失联"
void RMSerialDriver::ArmSendData(const my_26robot_interfaces::msg::JJointCommand::SharedPtr msg)
{
  markSendActive();
  try {
    ArmSendPacket packet;
    const auto receive_status = currentReceiveStatusCode();
    // msg为空指针判断为测试模式
    if(receive_status == kReceiveOkCode && msg!= nullptr)
    {
        packet.joint1 = msg->joint_angles[0];
        packet.joint2 = msg->joint_angles[1];
        packet.joint3 = msg->joint_angles[2];
        packet.joint4 = msg->joint_angles[3];
        packet.joint5 = msg->joint_angles[4];
        packet.joint6 = msg->joint_angles[5];

        // 对 sizeof(packet)-1 个字节计算 CRC（最后 2 字节是 CRC 位）
        crc16::Append_CRC16_Check_Sum(reinterpret_cast<uint8_t *>(&packet), sizeof(packet)-1);
        std::vector<uint8_t> data = toVector(packet);   // 结构体 → 字节流

        serial_driver_->port()->send(data);
        send_ack_pub_->publish(msg->header);
    }else if(receive_status == kReceiveTimeoutCode){
        // 超时：发全 0，但状态码写"超时"
        packet.joint1 = 0;
        packet.joint2 = 0;
        packet.joint3 = 0;
        packet.joint4 = 0;
        packet.joint5 = 0;
        packet.joint6 = 0;
    
        crc16::Append_CRC16_Check_Sum(reinterpret_cast<uint8_t *>(&packet), sizeof(packet)-1);
        std::vector<uint8_t> data = toVector(packet);

        serial_driver_->port()->send(data);
        // rclcpp::shutdown();
        // exit(0);

    }

    // 计算并发布延迟（从消息 header.stamp 到现在的秒数 * 1000）
    if (msg != nullptr) {
      std_msgs::msg::Float64 latency;
      latency.data = (this->now() - msg->header.stamp).seconds() * 1000.0;
      RCLCPP_DEBUG_STREAM(get_logger(), "Total latency: " + std::to_string(latency.data) + "ms");
      latency_pub_->publish(latency);
    }
  } catch (const std::exception & ex) {
    RCLCPP_ERROR(get_logger(), "Error while sending data: %s", ex.what());
    reopenPort();
  }
}


// ======================================================== 读取 串口 数据 ====================================================
// 设备名、波特率、流控、校验位、停止位，做字符串到枚举的映射和校验
void RMSerialDriver::getParams()
{
  using FlowControl = drivers::serial_driver::FlowControl;
  using Parity = drivers::serial_driver::Parity;
  using StopBits = drivers::serial_driver::StopBits;

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
  try {
    const auto fc_string = declare_parameter<std::string>("flow_control", "none");

    if (fc_string == "none") {
      fc = FlowControl::NONE;
    } else if (fc_string == "hardware") {
      fc = FlowControl::HARDWARE;
    } else if (fc_string == "software") {
      fc = FlowControl::SOFTWARE;
    } else {
      throw std::invalid_argument{
        "The flow_control parameter must be one of: none, software, or hardware."};
    }
  } catch (rclcpp::ParameterTypeException & ex) {
    RCLCPP_ERROR(get_logger(), "The flow_control provided was invalid");
    throw ex;
  }

  // 校验位：none / odd / even
  try {
    const auto pt_string = declare_parameter<std::string>("parity", "none");

    if (pt_string == "none") {
      pt = Parity::NONE;
    } else if (pt_string == "odd") {
      pt = Parity::ODD;
    } else if (pt_string == "even") {
      pt = Parity::EVEN;
    } else {
      throw std::invalid_argument{"The parity parameter must be one of: none, odd, or even."};
    }
  } catch (rclcpp::ParameterTypeException & ex) {
    RCLCPP_ERROR(get_logger(), "The parity provided was invalid");
    throw ex;
  }

  // 停止位：1 / 1.5 / 2
  try {
    const auto sb_string = declare_parameter<std::string>("stop_bits", "1");

    if (sb_string == "1" || sb_string == "1.0") {
      sb = StopBits::ONE;
    } else if (sb_string == "1.5") {
      sb = StopBits::ONE_POINT_FIVE;
    } else if (sb_string == "2" || sb_string == "2.0") {
      sb = StopBits::TWO;
    } else {
      throw std::invalid_argument{"The stop_bits parameter must be one of: 1, 1.5, or 2."};
    }
  } catch (rclcpp::ParameterTypeException & ex) {
    RCLCPP_ERROR(get_logger(), "The stop_bits provided was invalid");
    throw ex;
  }

  // 组装串口配置对象
  device_config_ =
    std::make_unique<drivers::serial_driver::SerialPortConfig>(baud_rate, fc, pt, sb);
}

// ==================== 重开串口 ====================
void RMSerialDriver::reopenPort()
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
      rclcpp::sleep_for(std::chrono::seconds(1));// 失败则睡 1 秒继续重试
    }
  }
}

// 打印串口数据
// 以十六进制（两位、空格分隔）打印一段字节流，调试用
void RMSerialDriver::printSerialPortContent(const std::vector<uint8_t> & data)
{
  for(long unsigned int i=0;i<data.size();i++){
    std::cout<< std::hex<<std::setw(2)<<std::setfill('0') 
            <<(unsigned int) (unsigned char)(data[i]) <<"  ";
    std::cout<< std::hex<<std::setw(2)<<std::setfill('0') << static_cast<int>(data[i]) <<"  ";
    printf("hex: %#x\n",data[i]); //输出带有前缀 0x 的十六进制数
  }
  std::cout<<std::endl;
}

// 调试用：用空消息起一个发送线程，周期性向下位机发测试包
void RMSerialDriver::verifySerialPortTransmission()
{
  RCLCPP_WARN(get_logger(), "Verify serial port transmission");
  auto msg = std::make_shared<my_26robot_interfaces::msg::JJointCommand>();
  try{
      send_thread_ = std::thread(&RMSerialDriver::ArmSendData,this,msg);
  } catch (const std::exception & ex) {
    RCLCPP_ERROR(get_logger(), "Failed to create send thread: %s", ex.what());
    throw ex;
    //throw；
  }

}

}

// 注册组件类，使 rm_serial_driver_node 可执行文件和 component container 能实例化本节点
RCLCPP_COMPONENTS_REGISTER_NODE(rm_serial_driver::RMSerialDriver)
