cmake_minimum_required(VERSION 3.8)
project(my_26robot_serial)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#######################
## Find dependencies ##
#######################

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

# 显式引入，保证 rclcpp_components_register_node 可用
find_package(rclcpp_components REQUIRED)
find_package(my_26robot_interfaces REQUIRED)

###########
## Build ##
###########

# ★ 改成你实际存在的源文件
ament_auto_add_library(${PROJECT_NAME} SHARED
  src/joint_angle_serial.cpp
  src/crc.cpp
)

# 组件需要链接 rclcpp_components
target_link_libraries(${PROJECT_NAME}
  rclcpp_components::component
)

# ★ PLUGIN 改成 joint_angle_serial.hpp 里实际定义的类名
rclcpp_components_register_node(${PROJECT_NAME}
  PLUGIN my_26robot_serial::JointAngleSerialNode
  EXECUTABLE ${PROJECT_NAME}_node
)

#############
## Testing ##
#############

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  list(APPEND AMENT_LINT_AUTO_EXCLUDE
    ament_cmake_copyright
    ament_cmake_uncrustify
  )
  ament_lint_auto_find_test_dependencies()
endif()

#############
## Install ##
#############

ament_auto_package(
  INSTALL_TO_SHARE
  config
  launch
)