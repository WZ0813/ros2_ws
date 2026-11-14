// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from my_26robot_interfaces:msg/JointCommand.idl
// generated code does not contain a copyright notice

#ifndef MY_26ROBOT_INTERFACES__MSG__DETAIL__JOINT_COMMAND__TRAITS_HPP_
#define MY_26ROBOT_INTERFACES__MSG__DETAIL__JOINT_COMMAND__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "my_26robot_interfaces/msg/detail/joint_command__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace my_26robot_interfaces
{

namespace msg
{

inline void to_flow_style_yaml(
  const JointCommand & msg,
  std::ostream & out)
{
  out << "{";
  // member: joint_angles
  {
    if (msg.joint_angles.size() == 0) {
      out << "joint_angles: []";
    } else {
      out << "joint_angles: [";
      size_t pending_items = msg.joint_angles.size();
      for (auto item : msg.joint_angles) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const JointCommand & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: joint_angles
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.joint_angles.size() == 0) {
      out << "joint_angles: []\n";
    } else {
      out << "joint_angles:\n";
      for (auto item : msg.joint_angles) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const JointCommand & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace my_26robot_interfaces

namespace rosidl_generator_traits
{

[[deprecated("use my_26robot_interfaces::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const my_26robot_interfaces::msg::JointCommand & msg,
  std::ostream & out, size_t indentation = 0)
{
  my_26robot_interfaces::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use my_26robot_interfaces::msg::to_yaml() instead")]]
inline std::string to_yaml(const my_26robot_interfaces::msg::JointCommand & msg)
{
  return my_26robot_interfaces::msg::to_yaml(msg);
}

template<>
inline const char * data_type<my_26robot_interfaces::msg::JointCommand>()
{
  return "my_26robot_interfaces::msg::JointCommand";
}

template<>
inline const char * name<my_26robot_interfaces::msg::JointCommand>()
{
  return "my_26robot_interfaces/msg/JointCommand";
}

template<>
struct has_fixed_size<my_26robot_interfaces::msg::JointCommand>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<my_26robot_interfaces::msg::JointCommand>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<my_26robot_interfaces::msg::JointCommand>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // MY_26ROBOT_INTERFACES__MSG__DETAIL__JOINT_COMMAND__TRAITS_HPP_
