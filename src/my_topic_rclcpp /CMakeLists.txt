cmake_minimum_required(VERSION 3.8)
project(my_topic_rclcpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)
find_package(example_interfaces REQUIRED)
find_package(my_26robot_interfaces REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(moveit_msgs REQUIRED)
find_package(trajectory_msgs REQUIRED)

add_executable(topic_publisher src/topic_publisher.cpp)
ament_target_dependencies(
  topic_publisher
  rclcpp 
  std_msgs 
  moveit_ros_planning_interface 
  example_interfaces
  my_26robot_interfaces
  tf2
  tf2_geometry_msgs
  geometry_msgs
  moveit_msgs
  trajectory_msgs)

install(TARGETS
  topic_publisher
  DESTINATION lib/${PROJECT_NAME}
)

add_executable(topic_subscribe src/topic_subscribe.cpp)
ament_target_dependencies(
  topic_subscribe
  rclcpp 
  std_msgs
   moveit_ros_planning_interface
   example_interfaces
  my_26robot_interfaces)
  
install(TARGETS
topic_subscribe
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
