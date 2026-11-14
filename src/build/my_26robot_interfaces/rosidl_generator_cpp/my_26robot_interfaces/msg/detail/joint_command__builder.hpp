// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from my_26robot_interfaces:msg/JointCommand.idl
// generated code does not contain a copyright notice

#ifndef MY_26ROBOT_INTERFACES__MSG__DETAIL__JOINT_COMMAND__BUILDER_HPP_
#define MY_26ROBOT_INTERFACES__MSG__DETAIL__JOINT_COMMAND__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "my_26robot_interfaces/msg/detail/joint_command__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace my_26robot_interfaces
{

namespace msg
{

namespace builder
{

class Init_JointCommand_joint_angles
{
public:
  Init_JointCommand_joint_angles()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::my_26robot_interfaces::msg::JointCommand joint_angles(::my_26robot_interfaces::msg::JointCommand::_joint_angles_type arg)
  {
    msg_.joint_angles = std::move(arg);
    return std::move(msg_);
  }

private:
  ::my_26robot_interfaces::msg::JointCommand msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::my_26robot_interfaces::msg::JointCommand>()
{
  return my_26robot_interfaces::msg::builder::Init_JointCommand_joint_angles();
}

}  // namespace my_26robot_interfaces

#endif  // MY_26ROBOT_INTERFACES__MSG__DETAIL__JOINT_COMMAND__BUILDER_HPP_
