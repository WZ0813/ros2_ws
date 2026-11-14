cmake_minimum_required(VERSION 3.8)
project(my_robot_commander_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies (查找依赖包)
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)
find_package(example_interfaces REQUIRED)
find_package(my_26robot_interfaces REQUIRED)
# 💡 补充缺少的依赖
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(moveit_msgs REQUIRED)

# 1. 编译 test_moveit
add_executable(test_moveit src/test_moveit.cpp)
ament_target_dependencies(test_moveit 
  rclcpp 
  moveit_ros_planning_interface
)

# 2. 编译 commander
add_executable(commander src/commander_template.cpp)
ament_target_dependencies(commander
  rclcpp 
  moveit_ros_planning_interface 
  example_interfaces
  my_26robot_interfaces
  tf2
  tf2_geometry_msgs
  geometry_msgs
  moveit_msgs
)

# 💡 关键修复：必须把 commander 也加进 install 里面！
install(TARGETS
  test_moveit
  commander
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()