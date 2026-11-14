#ifndef MY_26ROBOT_SERIAL__PACKET_HPP_
#define MY_26ROBOT_SERIAL__PACKET_HPP_

#include "my_26robot_serial/joint_angle_serial.hpp"
#include "my_26robot_serial/crc.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

namespace my_26robot_serial
{

constexpr std::size_t kJointCount = 6;

/// @brief 单片机发送给 ROS 的反馈数据包。
struct JointReceivePacket
{
  uint8_t header = 0xBE;
  float joint1;
  float joint2;
  float joint3;
  float joint4;
  float joint5;
  float joint6;

  uint16_t checksum = 0;
} __attribute__((packed));

/// @brief ROS 发送给单片机的六轴角度数据包。
struct JointAngleSendPacket
{
  uint8_t header = 0xBE;
  float joint1;
  float joint2;
  float joint3;
  float joint4;
  float joint5;
  float joint6;
  uint8_t if_recieve_data;
  uint16_t checksum = 0;
  uint8_t tail = 0xED;
} __attribute__((packed));

// 帧格式：帧头 + 六个 float32 弧度角 + CRC16（低字节在前）+ 帧尾。
// 发送帧的 CRC 仅覆盖帧头和角度数据，不包含 CRC 字段与帧尾。
static_assert(sizeof(float) == sizeof(uint32_t), "The protocol requires 32-bit floats");
static_assert(std::numeric_limits<float>::is_iec559, "The protocol requires IEEE-754 floats");
static_assert(sizeof(JointReceivePacket) == 27, "Unexpected joint receive packet size");
static_assert(sizeof(JointAngleSendPacket) == 29, "Unexpected joint angle packet size");

// 将固定长度的串口字节数据解析为对应的数据包结构体。
template<typename PacketType>
inline PacketType fromVector(const std::vector<uint8_t> & data)
{
  if (data.size() != sizeof(PacketType)) {
    throw std::invalid_argument("Serial packet size does not match packet structure");
  }

  PacketType packet{};
  std::copy(data.begin(), data.end(), reinterpret_cast<uint8_t *>(&packet));
  return packet;
}

// 将六个角度填入紧凑数据包，并按协议计算 CRC16。
inline JointAngleSendPacket makeJointAnglePacket(
  const std::array<double, kJointCount> & angles)
{
  JointAngleSendPacket packet{};
  packet.header = 0xBE;
  packet.joint1 = static_cast<float>(angles[0]);
  packet.joint2 = static_cast<float>(angles[1]);
  packet.joint3 = static_cast<float>(angles[2]);
  packet.joint4 = static_cast<float>(angles[3]);
  packet.joint5 = static_cast<float>(angles[4]);
  packet.joint6 = static_cast<float>(angles[5]);
  packet.tail = 0xED;

  // 长度不包含帧尾；Append 函数会把 CRC 写入 checksum 字段。
  crc16::Append_CRC16_Check_Sum(
    reinterpret_cast<uint8_t *>(&packet), sizeof(packet) - sizeof(packet.tail));
  return packet;
}

// 将紧凑数据包复制为串口驱动使用的字节数组。
inline std::vector<uint8_t> toVector(const JointAngleSendPacket & packet)
{
  const auto * first = reinterpret_cast<const uint8_t *>(&packet);
  return std::vector<uint8_t>(first, first + sizeof(packet));
}

}  // namespace my_26robot_serial

#endif  // MY_26ROBOT_SERIAL__PACKET_HPP_
