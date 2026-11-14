cmake_minimum_required(VERSION 3.8)
project(example_topic_rclcpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(topic_publisher_01 src/topic_publisher_01.cpp)
ament_target_dependencies(
  topic_publisher_01 
  rclcpp 
  std_msgs)

install(TARGETS
  topic_publisher_01
  DESTINATION lib/${PROJECT_NAME}
)

add_executable(topic_subscribe_01 src/topic_subscribe_01.cpp)
ament_target_dependencies(
  topic_subscribe_01 
  rclcpp 
  std_msgs)
install(TARGETS
topic_subscribe_01
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
