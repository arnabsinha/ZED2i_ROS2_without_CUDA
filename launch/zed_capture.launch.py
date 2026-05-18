from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([

        Node(
            package='zed_capture',
            executable='stereo_cam_node',
            name='stereo_camera',
            output='screen'
        ),

        Node(
            package='zed_capture',
            executable='imu_node',
            name='imu_sensor',
            output='screen'
        )

    ])
