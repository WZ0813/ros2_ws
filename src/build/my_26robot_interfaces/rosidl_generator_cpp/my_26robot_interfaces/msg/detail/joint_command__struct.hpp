// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from my_26robot_interfaces:msg/JointCommand.idl
// generated code does not contain a copyright notice

#ifndef MY_26ROBOT_INTERFACES__MSG__DETAIL__JOINT_COMMAND__STRUCT_HPP_
#define MY_26ROBOT_INTERFACES__MSG__DETAIL__JOINT_COMMAND__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__my_26robot_interfaces__msg__JointCommand __attribute__((deprecated))
#else
# define DEPRECATED__my_26robot_interfaces__msg__JointCommand __declspec(deprecated)
#endif

namespace my_26robot_interfaces
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct JointCommand_
{
  using Type = JointCommand_<ContainerAllocator>;

  explicit JointCommand_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_init;
  }

  explicit JointCommand_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_init;
    (void)_alloc;
  }

  // field types and members
  using _joint_angles_type =
    std::vector<double, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<double>>;
  _joint_angles_type joint_angles;

  // setters for named parameter idiom
  Type & set__joint_angles(
    const std::vector<double, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<double>> & _arg)
  {
    this->joint_angles = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    my_26robot_interfaces::msg::JointCommand_<ContainerAllocator> *;
  using ConstRawPtr =
    const my_26robot_interfaces::msg::JointCommand_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<my_26robot_interfaces::msg::JointCommand_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<my_26robot_interfaces::msg::JointCommand_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      my_26robot_interfaces::msg::JointCommand_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<my_26robot_interfaces::msg::JointCommand_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      my_26robot_interfaces::msg::JointCommand_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<my_26robot_interfaces::msg::JointCommand_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<my_26robot_interfaces::msg::JointCommand_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<my_26robot_interfaces::msg::JointCommand_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__my_26robot_interfaces__msg__JointCommand
    std::shared_ptr<my_26robot_interfaces::msg::JointCommand_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__my_26robot_interfaces__msg__JointCommand
    std::shared_ptr<my_26robot_interfaces::msg::JointCommand_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const JointCommand_ & other) const
  {
    if (this->joint_angles != other.joint_angles) {
      return false;
    }
    return true;
  }
  bool operator!=(const JointCommand_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct JointCommand_

// alias to use template instance with default allocator
using JointCommand =
  my_26robot_interfaces::msg::JointCommand_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace my_26robot_interfaces

#endif  // MY_26ROBOT_INTERFACES__MSG__DETAIL__JOINT_COMMAND__STRUCT_HPP_
