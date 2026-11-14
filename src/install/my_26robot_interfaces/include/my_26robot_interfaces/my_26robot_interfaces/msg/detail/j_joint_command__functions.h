// generated from rosidl_generator_c/resource/idl__functions.h.em
// with input from my_26robot_interfaces:msg/JJointCommand.idl
// generated code does not contain a copyright notice

#ifndef MY_26ROBOT_INTERFACES__MSG__DETAIL__J_JOINT_COMMAND__FUNCTIONS_H_
#define MY_26ROBOT_INTERFACES__MSG__DETAIL__J_JOINT_COMMAND__FUNCTIONS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "rosidl_runtime_c/visibility_control.h"
#include "my_26robot_interfaces/msg/rosidl_generator_c__visibility_control.h"

#include "my_26robot_interfaces/msg/detail/j_joint_command__struct.h"

/// Initialize msg/JJointCommand message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * my_26robot_interfaces__msg__JJointCommand
 * )) before or use
 * my_26robot_interfaces__msg__JJointCommand__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_my_26robot_interfaces
bool
my_26robot_interfaces__msg__JJointCommand__init(my_26robot_interfaces__msg__JJointCommand * msg);

/// Finalize msg/JJointCommand message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_my_26robot_interfaces
void
my_26robot_interfaces__msg__JJointCommand__fini(my_26robot_interfaces__msg__JJointCommand * msg);

/// Create msg/JJointCommand message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * my_26robot_interfaces__msg__JJointCommand__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_my_26robot_interfaces
my_26robot_interfaces__msg__JJointCommand *
my_26robot_interfaces__msg__JJointCommand__create();

/// Destroy msg/JJointCommand message.
/**
 * It calls
 * my_26robot_interfaces__msg__JJointCommand__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_my_26robot_interfaces
void
my_26robot_interfaces__msg__JJointCommand__destroy(my_26robot_interfaces__msg__JJointCommand * msg);

/// Check for msg/JJointCommand message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_my_26robot_interfaces
bool
my_26robot_interfaces__msg__JJointCommand__are_equal(const my_26robot_interfaces__msg__JJointCommand * lhs, const my_26robot_interfaces__msg__JJointCommand * rhs);

/// Copy a msg/JJointCommand message.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source message pointer.
 * \param[out] output The target message pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer is null
 *   or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_my_26robot_interfaces
bool
my_26robot_interfaces__msg__JJointCommand__copy(
  const my_26robot_interfaces__msg__JJointCommand * input,
  my_26robot_interfaces__msg__JJointCommand * output);

/// Initialize array of msg/JJointCommand messages.
/**
 * It allocates the memory for the number of elements and calls
 * my_26robot_interfaces__msg__JJointCommand__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_my_26robot_interfaces
bool
my_26robot_interfaces__msg__JJointCommand__Sequence__init(my_26robot_interfaces__msg__JJointCommand__Sequence * array, size_t size);

/// Finalize array of msg/JJointCommand messages.
/**
 * It calls
 * my_26robot_interfaces__msg__JJointCommand__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_my_26robot_interfaces
void
my_26robot_interfaces__msg__JJointCommand__Sequence__fini(my_26robot_interfaces__msg__JJointCommand__Sequence * array);

/// Create array of msg/JJointCommand messages.
/**
 * It allocates the memory for the array and calls
 * my_26robot_interfaces__msg__JJointCommand__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_my_26robot_interfaces
my_26robot_interfaces__msg__JJointCommand__Sequence *
my_26robot_interfaces__msg__JJointCommand__Sequence__create(size_t size);

/// Destroy array of msg/JJointCommand messages.
/**
 * It calls
 * my_26robot_interfaces__msg__JJointCommand__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_my_26robot_interfaces
void
my_26robot_interfaces__msg__JJointCommand__Sequence__destroy(my_26robot_interfaces__msg__JJointCommand__Sequence * array);

/// Check for msg/JJointCommand message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_my_26robot_interfaces
bool
my_26robot_interfaces__msg__JJointCommand__Sequence__are_equal(const my_26robot_interfaces__msg__JJointCommand__Sequence * lhs, const my_26robot_interfaces__msg__JJointCommand__Sequence * rhs);

/// Copy an array of msg/JJointCommand messages.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source array pointer.
 * \param[out] output The target array pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer
 *   is null or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_my_26robot_interfaces
bool
my_26robot_interfaces__msg__JJointCommand__Sequence__copy(
  const my_26robot_interfaces__msg__JJointCommand__Sequence * input,
  my_26robot_interfaces__msg__JJointCommand__Sequence * output);

#ifdef __cplusplus
}
#endif

#endif  // MY_26ROBOT_INTERFACES__MSG__DETAIL__J_JOINT_COMMAND__FUNCTIONS_H_
