import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description() -> LaunchDescription:
    params = os.path.join(
        get_package_share_directory('my_26robot_serial'),
        'config', 'joint_angle_serial.yaml')

    return LaunchDescription([
        Node(
            package='my_26robot_serial',
            executable='my_26robot_serial_node',
            name='joint_angle_serial',
            parameters=[params],
            output='screen',
        ),
    ])
