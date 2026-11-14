// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from my_26robot_interfaces:msg/JJointCommand.idl
// generated code does not contain a copyright notice

#ifndef MY_26ROBOT_INTERFACES__MSG__DETAIL__J_JOINT_COMMAND__STRUCT_HPP_
#define MY_26ROBOT_INTERFACES__MSG__DETAIL__J_JOINT_COMMAND__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__my_26robot_interfaces__msg__JJointCommand __attribute__((deprecated))
#else
# define DEPRECATED__my_26robot_interfaces__msg__JJointCommand __declspec(deprecated)
#endif

namespace my_26robot_interfaces
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct JJointCommand_
{
  using Type = JJointCommand_<ContainerAllocator>;

  explicit JJointCommand_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      std::fill<typename std::array<double, 6>::iterator, double>(this->joint_angles.begin(), this->joint_angles.end(), 0.0);
    }
  }

  explicit JJointCommand_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_alloc, _init),
    joint_angles(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      std::fill<typename std::array<double, 6>::iterator, double>(this->joint_angles.begin(), this->joint_angles.end(), 0.0);
    }
  }

  // field types and members
  using _header_type =
    std_msgs::msg::Header_<ContainerAllocator>;
  _header_type header;
  using _joint_angles_type =
    std::array<double, 6>;
  _joint_angles_type joint_angles;

  // setters for named parameter idiom
  Type & set__header(
    const std_msgs::msg::Header_<ContainerAllocator> & _arg)
  {
    this->header = _arg;
    return *this;
  }
  Type & set__joint_angles(
    const std::array<double, 6> & _arg)
  {
    this->joint_angles = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    my_26robot_interfaces::msg::JJointCommand_<ContainerAllocator> *;
  using ConstRawPtr =
    const my_26robot_interfaces::msg::JJointCommand_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<my_26robot_interfaces::msg::JJointCommand_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<my_26robot_interfaces::msg::JJointCommand_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      my_26robot_interfaces::msg::JJointCommand_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<my_26robot_interfaces::msg::JJointCommand_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      my_26robot_interfaces::msg::JJointCommand_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<my_26robot_interfaces::msg::JJointCommand_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<my_26robot_interfaces::msg::JJointCommand_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<my_26robot_interfaces::msg::JJointCommand_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__my_26robot_interfaces__msg__JJointCommand
    std::shared_ptr<my_26robot_interfaces::msg::JJointCommand_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__my_26robot_interfaces__msg__JJointCommand
    std::shared_ptr<my_26robot_interfaces::msg::JJointCommand_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const JJointCommand_ & other) const
  {
    if (this->header != other.header) {
      return false;
    }
    if (this->joint_angles != other.joint_angles) {
      return false;
    }
    return true;
  }
  bool operator!=(const JJointCommand_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct JJointCommand_

// alias to use template instance with default allocator
using JJointCommand =
  my_26robot_interfaces::msg::JJointCommand_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace my_26robot_interfaces

#endif  // MY_26ROBOT_INTERFACES__MSG__DETAIL__J_JOINT_COMMAND__STRUCT_HPP_
