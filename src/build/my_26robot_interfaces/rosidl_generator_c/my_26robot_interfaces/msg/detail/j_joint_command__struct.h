// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from my_26robot_interfaces:msg/JJointCommand.idl
// generated code does not contain a copyright notice

#ifndef MY_26ROBOT_INTERFACES__MSG__DETAIL__J_JOINT_COMMAND__STRUCT_H_
#define MY_26ROBOT_INTERFACES__MSG__DETAIL__J_JOINT_COMMAND__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__struct.h"

/// Struct defined in msg/JJointCommand in the package my_26robot_interfaces.
typedef struct my_26robot_interfaces__msg__JJointCommand
{
  std_msgs__msg__Header header;
  double joint_angles[6];
} my_26robot_interfaces__msg__JJointCommand;

// Struct for a sequence of my_26robot_interfaces__msg__JJointCommand.
typedef struct my_26robot_interfaces__msg__JJointCommand__Sequence
{
  my_26robot_interfaces__msg__JJointCommand * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} my_26robot_interfaces__msg__JJointCommand__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // MY_26ROBOT_INTERFACES__MSG__DETAIL__J_JOINT_COMMAND__STRUCT_H_
