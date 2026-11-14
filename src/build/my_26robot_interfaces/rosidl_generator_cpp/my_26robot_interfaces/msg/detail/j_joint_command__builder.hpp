// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from my_26robot_interfaces:msg/JJointCommand.idl
// generated code does not contain a copyright notice

#ifndef MY_26ROBOT_INTERFACES__MSG__DETAIL__J_JOINT_COMMAND__BUILDER_HPP_
#define MY_26ROBOT_INTERFACES__MSG__DETAIL__J_JOINT_COMMAND__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "my_26robot_interfaces/msg/detail/j_joint_command__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace my_26robot_interfaces
{

namespace msg
{

namespace builder
{

class Init_JJointCommand_joint_angles
{
public:
  explicit Init_JJointCommand_joint_angles(::my_26robot_interfaces::msg::JJointCommand & msg)
  : msg_(msg)
  {}
  ::my_26robot_interfaces::msg::JJointCommand joint_angles(::my_26robot_interfaces::msg::JJointCommand::_joint_angles_type arg)
  {
    msg_.joint_angles = std::move(arg);
    return std::move(msg_);
  }

private:
  ::my_26robot_interfaces::msg::JJointCommand msg_;
};

class Init_JJointCommand_header
{
public:
  Init_JJointCommand_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_JJointCommand_joint_angles header(::my_26robot_interfaces::msg::JJointCommand::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_JJointCommand_joint_angles(msg_);
  }

private:
  ::my_26robot_interfaces::msg::JJointCommand msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::my_26robot_interfaces::msg::JJointCommand>()
{
  return my_26robot_interfaces::msg::builder::Init_JJointCommand_header();
}

}  // namespace my_26robot_interfaces

#endif  // MY_26ROBOT_INTERFACES__MSG__DETAIL__J_JOINT_COMMAND__BUILDER_HPP_
