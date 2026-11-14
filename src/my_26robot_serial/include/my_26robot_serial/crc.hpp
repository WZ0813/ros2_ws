#ifndef MY_26ROBOT_SERIAL__CRC_HPP_
#define MY_26ROBOT_SERIAL__CRC_HPP_

#include <cstdint>

namespace my_26robot_serial
{
namespace crc16
{

// 对指定长度的数据计算 CRC16；initial_value 默认使用协议初始值 0xFFFF。
uint16_t Get_CRC16_Check_Sum(
  const uint8_t * data, uint32_t length, uint16_t initial_value = 0xFFFF);

// 校验末尾两个字节中的 CRC 是否与前面的数据一致。
uint32_t Verify_CRC16_Check_Sum(const uint8_t * data, uint32_t length);

// 计算 CRC 并写入缓冲区末尾两个字节，length 包含数据和 CRC 字段。
void Append_CRC16_Check_Sum(uint8_t * data, uint32_t length);

}  // namespace crc16
}  // namespace my_26robot_serial

#endif  // MY_26ROBOT_SERIAL__CRC_HPP_
