from launch import LaunchDescription
from launch.actions import RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.substitutions import PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from moveit_configs_utils import MoveItConfigsBuilder


def generate_launch_description():
    moveit_config = MoveItConfigsBuilder(
        "my_robot", package_name="my_26robot_moveit_config"
    ).to_moveit_configs()
    robot_description = moveit_config.robot_description
    controller_config = PathJoinSubstitution(
        [
            FindPackageShare("my_26robot_bringup"),
            "config",
            "ros2_controllers.yaml",
        ]
    )
    rviz_config = PathJoinSubstitution(
        [
            FindPackageShare("my_26robot_moveit_config"),
            "config",
            "moveit.rviz",
        ]
    )
    joint_state_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=[
            "joint_state_broadcaster",
            "--controller-manager",
            "/controller_manager",
            "--controller-manager-timeout",
            "30",
        ],
        output="screen",
    )
    arm_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=[
            "arm_controller",
            "--controller-manager",
            "/controller_manager",
            "--controller-manager-timeout",
            "30",
        ],
        output="screen",
    )
    gripper_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=[
            "gripper_controller",
            "--controller-manager",
            "/controller_manager",
            "--controller-manager-timeout",
            "30",
        ],
        output="screen",
    )

    return LaunchDescription(
        [
            Node(
                package="robot_state_publisher",
                executable="robot_state_publisher",
                name="robot_state_publisher",
                parameters=[robot_description],
                output="screen",
            ),
            Node(
                package="controller_manager",
                executable="ros2_control_node",
                parameters=[controller_config],
                remappings=[("~/robot_description", "/robot_description")],
                output="screen",
            ),
            Node(
                package="tf2_ros",
                executable="static_transform_publisher",
                name="world_to_base",
                arguments=[
                    "--x",
                    "0",
                    "--y",
                    "0",
                    "--z",
                    "0",
                    "--roll",
                    "0",
                    "--pitch",
                    "0",
                    "--yaw",
                    "0",
                    "--frame-id",
                    "world_link",
                    "--child-frame-id",
                    "base_link",
                ],
                output="screen",
            ),
            joint_state_spawner,
            RegisterEventHandler(
                OnProcessExit(
                    target_action=joint_state_spawner,
                    on_exit=[arm_spawner],
                )
            ),
            RegisterEventHandler(
                OnProcessExit(
                    target_action=arm_spawner,
                    on_exit=[gripper_spawner],
                )
            ),
            Node(
                package="moveit_ros_move_group",
                executable="move_group",
                parameters=[moveit_config.to_dict()],
                output="screen",
            ),
            Node(
                package="my_26robot_commander_cpp",
                executable="commander",
                parameters=[moveit_config.to_dict()],
                output="screen",
            ),
            Node(
                package="my_26robot_commander_cpp",
                executable="point2point",
                parameters=[moveit_config.to_dict()],
                output="screen",
            ),
            Node(
                package="rviz2",
                executable="rviz2",
                arguments=[
                    "-d",
                    rviz_config,
                ],
                parameters=[moveit_config.to_dict()],
                output="screen",
            ),
        ]
    )
