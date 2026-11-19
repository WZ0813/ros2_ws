cmake_minimum_required(VERSION 3.8)
project(my_26robot_commander_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies (查找依赖包)
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)
find_package(example_interfaces REQUIRED)
find_package(my_26robot_interfaces REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(moveit_msgs REQUIRED)
find_package(trajectory_msgs REQUIRED)
find_package(std_msgs REQUIRED)

# 编译 commander
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
  trajectory_msgs
)

# 4. 编译 point2point
add_executable(point2point src/point2point.cpp)
ament_target_dependencies(point2point
  rclcpp
  moveit_ros_planning_interface
  moveit_msgs
  my_26robot_interfaces
  std_msgs
)

# 编译只发送规划轨迹最后一个点的节点。
add_executable(point2point_final src/point2point_final.cpp)
ament_target_dependencies(point2point_final
  rclcpp
  moveit_ros_planning_interface
  moveit_msgs
  my_26robot_interfaces
  std_msgs
  geometry_msgs
)



# 💡 关键修复：必须把 commander 也加进 install 里面！
install(TARGETS
  commander
  point2point
  point2point_final
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()