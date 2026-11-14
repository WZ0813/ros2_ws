#include <rclcpp/rclcpp.hpp>
#include <moveit/move_group_interface/move_group_interface.h>
#include <example_interfaces/msg/bool.hpp>
#include <my_26robot_interfaces/msg/joint_command.hpp>
#include <my_26robot_interfaces/msg/pose_command.hpp>
#include <trajectory_msgs/msg/joint_trajectory.hpp>

#include <tf2/LinearMath/Quaternion.h>
#include <thread>

using MoveGroupInterface = moveit::planning_interface::MoveGroupInterface;
using Bool = example_interfaces::msg::Bool;
using JointCmd = my_26robot_interfaces::msg::JointCommand;
using PoseCmd = my_26robot_interfaces::msg::PoseCommand;
using namespace std::placeholders;

class Commander
{
public:
    Commander(std::shared_ptr<rclcpp::Node> node)
    {
        node_ = node;
        
        //movelt接口
        arm_ = std::make_shared<MoveGroupInterface>(node_, "arm");
        arm_->setMaxVelocityScalingFactor(1.0);
        arm_->setMaxAccelerationScalingFactor(1.0);

        gripper_ = std::make_shared<MoveGroupInterface>(node_, "gripper");
        
        //话题订阅
        gripper_sub_ = node_->create_subscription<Bool>(
            "open_gripper", 10, std::bind(&Commander::OpenGripperCallback, this, _1));
        joint_cmd_sub_ = node_->create_subscription<JointCmd>(
            "joint_command", 10, std::bind(&Commander::jointCmdCallback, this, _1));
        pose_cmd_sub_ = node_->create_subscription<PoseCmd>(
             "pose_command", 10, std::bind(&Commander::poseCmdCallback, this, _1));
        
    }

    void goToNamedTarget(const std::string &name)
    {
        arm_->setStartStateToCurrentState();
        arm_->setNamedTarget(name);
        planAndExecute(arm_, true);
    }

    void goToJointTarget(const std::vector<double> &joints)
    {
        arm_->setStartStateToCurrentState();
        arm_->setJointValueTarget(joints);
        planAndExecute(arm_, true);
    }

    void goToPoseTarget(double x, double y, double z, 
                        double roll, double pitch, double yaw, bool cartesian_path = false)
    {
        tf2::Quaternion q;
        q.setRPY(roll, pitch, yaw);
        q.normalize();

        geometry_msgs::msg::PoseStamped target_pose;
        target_pose.header.frame_id = "base_link"; 
        
        target_pose.pose.position.x = x;
        target_pose.pose.position.y = y;
        target_pose.pose.position.z = z;
        
        target_pose.pose.orientation.x = q.getX();
        target_pose.pose.orientation.y = q.getY();
        target_pose.pose.orientation.z = q.getZ();
        target_pose.pose.orientation.w = q.getW();

        arm_->setStartStateToCurrentState();

        if (!cartesian_path) 
        {
            arm_->setPoseTarget(target_pose);
            planAndExecute(arm_, true);
        }
        else 
        {
            std::vector<geometry_msgs::msg::Pose> waypoints;
            waypoints.push_back(target_pose.pose);
            moveit_msgs::msg::RobotTrajectory trajectory;

            double fraction = arm_->computeCartesianPath(waypoints, 0.01, 0.0, trajectory);

            if (fraction >= 0.99) {
                trajectory_publisher_->publish(trajectory.joint_trajectory);
                arm_->execute(trajectory);
            } else {
                RCLCPP_WARN(node_->get_logger(), "笛卡尔直线规划仅完成 %.2f%%，存在奇异点或碰撞风险！", fraction * 100.0);
            }
        }
    }

    void openGripper()
    {
        gripper_->setStartStateToCurrentState();
        gripper_->setNamedTarget("gripper_open");
        planAndExecute(gripper_);
    }

    void closeGripper()
    {
        gripper_->setStartStateToCurrentState();
        gripper_->setNamedTarget("gripper_catch");
        planAndExecute(gripper_);
    }

private:
    void planAndExecute(
        const std::shared_ptr<MoveGroupInterface> &interface,
        bool forward_trajectory = false)
    {
        MoveGroupInterface::Plan plan;
        bool success = (interface->plan(plan) == moveit::core::MoveItErrorCode::SUCCESS);

        if (success) {
            if (forward_trajectory) {
                trajectory_publisher_->publish(plan.trajectory_.joint_trajectory);
            }
            interface->execute(plan);
        } else {
            // 💡 补充：规划失败时打印日志，方便排错
            RCLCPP_ERROR(node_->get_logger(), "规划失败！无法到达目标。");
        }
    }

    void OpenGripperCallback(const Bool &msg)
    {
        if (msg.data) {
            openGripper();
        } else {
            closeGripper();
        }
    }
    void jointCmdCallback(const JointCmd &msg)
    {
        if (msg.joint_angles.size() == 6) {
            goToJointTarget(msg.joint_angles);
        } else {
            RCLCPP_ERROR(
                node_->get_logger(),
                "joint_command requires exactly 6 joint angles; received %zu",
                msg.joint_angles.size());
        }
    }
    void poseCmdCallback(const PoseCmd &msg)
    {
        goToPoseTarget(msg.x, msg.y, msg.z, msg.roll, msg.pitch, msg.yaw, msg.cartesian_path);
    }


    std::shared_ptr<rclcpp::Node> node_;
    std::shared_ptr<MoveGroupInterface> arm_;
    std::shared_ptr<MoveGroupInterface> gripper_;
    
    // 修复后的变量声明
    rclcpp::Subscription<Bool>::SharedPtr gripper_sub_;
    rclcpp::Subscription<JointCmd>::SharedPtr joint_cmd_sub_;
    rclcpp::Subscription<PoseCmd>::SharedPtr pose_cmd_sub_;
    rclcpp::Publisher<trajectory_msgs::msg::JointTrajectory>::SharedPtr
        trajectory_publisher_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    
    auto node = std::make_shared<rclcpp::Node>(
        "commander",
        rclcpp::NodeOptions().automatically_declare_parameters_from_overrides(true)
    );
    
    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(node);
    
    std::thread spinner([&executor]() { executor.spin(); });

    auto commander = Commander(node);
    
    // 💡 测试区（你可以在这里加上你的测试代码）：
    // commander.goToNamedTarget("home"); 
    // commander.openGripper();

    // 等待退出信号
    if (spinner.joinable()) {
        spinner.join();
    }
    
    return 0;
}