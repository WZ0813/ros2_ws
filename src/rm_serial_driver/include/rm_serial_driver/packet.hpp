// Copyright (c) 2022 ChenJun
// Licensed under the Apache-2.0 License.

#ifndef RM_SERIAL_DRIVER__PACKET_HPP_
#define RM_SERIAL_DRIVER__PACKET_HPP_

#include <algorithm>
#include <cstdint>
#include <vector>
namespace rm_serial_driver
{
// @brief 哨兵收发数据包结构体
struct ArmReceivePacket
{
  uint8_t header = 0xBE;
  float joint1;
  float joint2;
  float joint3;
  float joint4;
  float joint5;
  float joint6;
  uint8_t mode;//AUTO_AIM: 0, BUFF: 1
  uint16_t checksum = 0;
} __attribute__((packed));//packed可以，好东西

struct ArmSendPacket
{
  uint8_t header = 0xBE;  
  float joint1;
  float joint2;
  float joint3;
  float joint4;
  float joint5;
  float joint6;
  uint16_t checksum = 0;
  uint8_t tail = 0xED;
} __attribute__((packed));



// std::vector<uint8_t> ===> ReceivePacket
template <typename PacketType>
inline PacketType fromVector(const std::vector<uint8_t> & data)
{
  PacketType packet;
  std::copy(data.begin(), data.end(), reinterpret_cast<uint8_t *>(&packet));
  return packet;
}
// SendPacket ===> std::vector<uint8_t>
template <typename PacketType>
inline std::vector<uint8_t> toVector(const PacketType & data)
{
  std::vector<uint8_t> packet(sizeof(PacketType));
  std::copy(
    reinterpret_cast<const uint8_t *>(&data),
    reinterpret_cast<const uint8_t *>(&data) + sizeof(PacketType), packet.begin());
  return packet;
}

}

#endif  // RM_SERIAL_DRIVER__PACKET_HPP_

