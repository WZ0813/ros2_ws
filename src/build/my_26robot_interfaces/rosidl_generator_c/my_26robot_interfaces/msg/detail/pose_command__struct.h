// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from my_26robot_interfaces:msg/PoseCommand.idl
// generated code does not contain a copyright notice

#ifndef MY_26ROBOT_INTERFACES__MSG__DETAIL__POSE_COMMAND__STRUCT_H_
#define MY_26ROBOT_INTERFACES__MSG__DETAIL__POSE_COMMAND__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

/// Struct defined in msg/PoseCommand in the package my_26robot_interfaces.
typedef struct my_26robot_interfaces__msg__PoseCommand
{
  double x;
  double y;
  double z;
  double roll;
  double pitch;
  double yaw;
  bool cartesian_path;
} my_26robot_interfaces__msg__PoseCommand;

// Struct for a sequence of my_26robot_interfaces__msg__PoseCommand.
typedef struct my_26robot_interfaces__msg__PoseCommand__Sequence
{
  my_26robot_interfaces__msg__PoseCommand * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} my_26robot_interfaces__msg__PoseCommand__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // MY_26ROBOT_INTERFACES__MSG__DETAIL__POSE_COMMAND__STRUCT_H_
