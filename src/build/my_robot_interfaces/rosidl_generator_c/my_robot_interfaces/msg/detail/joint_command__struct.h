// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from my_robot_interfaces:msg/JointCommand.idl
// generated code does not contain a copyright notice

#ifndef MY_ROBOT_INTERFACES__MSG__DETAIL__JOINT_COMMAND__STRUCT_H_
#define MY_ROBOT_INTERFACES__MSG__DETAIL__JOINT_COMMAND__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'joint_angles'
#include "rosidl_runtime_c/primitives_sequence.h"

/// Struct defined in msg/JointCommand in the package my_robot_interfaces.
typedef struct my_robot_interfaces__msg__JointCommand
{
  rosidl_runtime_c__double__Sequence joint_angles;
} my_robot_interfaces__msg__JointCommand;

// Struct for a sequence of my_robot_interfaces__msg__JointCommand.
typedef struct my_robot_interfaces__msg__JointCommand__Sequence
{
  my_robot_interfaces__msg__JointCommand * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} my_robot_interfaces__msg__JointCommand__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // MY_ROBOT_INTERFACES__MSG__DETAIL__JOINT_COMMAND__STRUCT_H_
