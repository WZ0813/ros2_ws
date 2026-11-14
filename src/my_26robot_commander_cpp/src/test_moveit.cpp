// 引入 ROS 2 的 C++ 客户端库基础头文件
#include <rclcpp/rclcpp.hpp>
// 引入 MoveIt 的运动规划接口头文件（核心）
#include <moveit/move_group_interface/move_group_interface.h>
/// @brief 
/// @param argc 
/// @param argv 
/// @return 
int main(int argc, char **argv)
{
    // 1. 初始化 ROS 2 通信（解析命令行参数，初始化底层环境）
    rclcpp::init(argc, argv);

    // 2. 创建一个名为 "test_moveit" 的节点，并交给智能指针 node 管理
    auto node = std::make_shared<rclcpp::Node>("test_moveit");

    // 3. 创建一个单线程执行器（Executor），用于处理节点的回调函数（比如接收话题、服务响应）
    rclcpp::executors::SingleThreadedExecutor executor;
    
    // 4. 把刚才创建的节点交给执行器管理
    executor.add_node(node);
    
    // 5. 开启一个后台子线程，让执行器在这个子线程里不停地“自旋”（spin）。
    // 这样主线程就不会被阻塞，可以继续往下执行代码。
    // 注意：MoveGroupInterface 在底层需要与 MoveIt 通信，必须有节点在 spin 才能收到响应。
    auto spinner = std::thread([&executor]() { executor.spin(); });

   //6. 核心步骤：实例化 MoveGroup 接口对象。
    // 将当前的 node 传入，并指定我们要控制的“规划组（Planning Group）”名称，这里叫 "arm"。
    // 这行代码执行后，程序就会去连接后台正在运行的 move_group 节点。
    auto arm = moveit::planning_interface::MoveGroupInterface(node, "arm");
    arm.setMaxVelocityScalingFactor(1.0);
    arm.setMaxAccelerationScalingFactor(1.0);

    auto gripper = moveit::planning_interface::MoveGroupInterface(node, "gripper");

 /*    // Named goal （命名目标：指的是在 SRDF 文件中提前定义好的一组关节角度/姿态）

    // 1. 设置规划的起点为机械臂“当前实际”的状态。
    // 极其重要：如果不加这一句，MoveIt 可能会用上一次记忆的旧位置作为起点，
    // 导致规划出的轨迹与真实情况脱节，执行时直接报错。
    arm.setStartStateToCurrentState();

    // 2. 设置目标姿态为名为 "pose_1" 的预设状态。
    // 注意：这个 "pose_1" 必须是你之前在 MoveIt Setup Assistant 里配置过的，
    // 或者手动在 .srdf 文件里写好的 <group_state>。
    // 如果找不到这个名字，规划会直接失败。
    arm.setNamedTarget("post_1");

    // 3. 声明一个 Plan 类型的对象 plan1。
    // 这个对象相当于一个“容器”，用来存放 MoveIt 接下来计算出的完整的运动轨迹
    //（包含每个关节在不同时间点的位置、速度、加速度等信息）。
    moveit::planning_interface::MoveGroupInterface::Plan plan1;

    // 4. 呼叫 MoveIt 的大脑进行“路径规划”（Plan）。
    // 它会让后台的 move_group 节点计算一条从起点到 "pose_1" 且不撞到障碍物的路径，
    // 并把结果存入 plan1 中。
    // 这里的逻辑是：如果返回的错误码是 SUCCESS（成功），那么 success1 就为 true。
    bool success1 = (arm.plan(plan1) == moveit::core::MoveItErrorCode::SUCCESS);

    // 5. 判断是否规划成功。如果成功，就让机械臂真正动起来。
    if (success1) {
        // 将 plan1 中的轨迹发送给底层的控制器（ros2_control）去执行，电机开始转动。
        arm.execute(plan1);
    } 
    else {
         RCLCPP_ERROR(node->get_logger(), "规划失败！找不到去往 pose_1 的路径。");
     }
*/  //。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。
    // Joint Goal

 /*   std::vector<double> joints = { 1.5, 0.5, 0.0, 1.5, 0.0, -0.7 };

    arm.setStartStateToCurrentState();
    arm.setJointValueTarget(joints);

    moveit::planning_interface::MoveGroupInterface::Plan plan1;
    bool success1 = (arm.plan(plan1) == moveit::core::MoveItErrorCode::SUCCESS);

    if (success1) {
        arm.execute(plan1);
    }
*/
    // -------------------------------------------------------------------------------------------------------------
    // Pose Goal

    tf2::Quaternion q;
    q.setRPY(3.14, 0.0, 0.0);
    q = q.normalize();

    geometry_msgs::msg::PoseStamped target_pose;
    target_pose.header.frame_id = "base_link";
    target_pose.pose.position.x = 0.0;
    target_pose.pose.position.y = -0.7;
    target_pose.pose.position.z = 0.4;
    target_pose.pose.orientation.x = q.getX();
    target_pose.pose.orientation.y = q.getY();
    target_pose.pose.orientation.z = q.getZ();
    target_pose.pose.orientation.w = q.getW();

    arm.setStartStateToCurrentState();
    arm.setPoseTarget(target_pose);

    moveit::planning_interface::MoveGroupInterface::Plan plan1;
    bool success1 = (arm.plan(plan1) == moveit::core::MoveItErrorCode::SUCCESS);

    if (success1) {
        arm.execute(plan1);
    }
    else {
        RCLCPP_ERROR(node->get_logger(), "规划失败！找不到去往目标位姿的路径。");
    }
    //----------------------------------------------------------------------------------------------------------------
    // Cartesian Path
    // 1. 准备路径点（Waypoints）
    // 创建一个位姿向量，用于存储机械臂末端需要经过的“路标点”
    std::vector<geometry_msgs::msg::Pose> waypoints;

    // 获取机械臂当前末端执行器的位姿（作为起点）
    geometry_msgs::msg::Pose pose1 = arm.getCurrentPose().pose;

    // 在当前 X 轴坐标的基础上减去 0.2 米（即向左移动 20 厘米），计算出目标位姿
    pose1.position.x += -0.2;

    // 将计算好的目标位姿加入路点向量中。注意：这里只有终点，底层会以当前位置为起点
    waypoints.push_back(pose1);

    waypoints.push_back(pose1);
    geometry_msgs::msg::Pose pose2 = pose1;
    pose2.position.y += 0.2;
    waypoints.push_back(pose2);
    geometry_msgs::msg::Pose pose3 = pose2;
    pose3.position.y += -0.2;
    pose3.position.z += 0.2;
    waypoints.push_back(pose3);

    // 2. 声明轨迹容器
    // 创建一个 RobotTrajectory 对象，用来接收规划器计算出的完整运动轨迹（包含关节角度、速度、时间等）
    moveit_msgs::msg::RobotTrajectory trajectory;

    // 3. 计算笛卡尔路径（核心函数）
    // 调用 computeCartesianPath 进行笛卡尔空间直线规划
    // - waypoints: 包含目标位姿的数组
    // - 0.01: (eef_step) 末端步长，表示每隔 1 厘米计算一次逆运动学（IK）解
    // - trajectory: 规划成功后，生成的轨迹会保存在这里
    // 返回值 fraction: 表示路径的完成比例（0.0 到 1.0 之间的浮点数）。
    // ⚠️ 注意：不同版本的 MoveIt 2 API 可能要求传入 jump_threshold 参数，例如：computeCartesianPath(waypoints, 0.01, 0.0, trajectory);
    double fraction = arm.computeCartesianPath(waypoints, 0.01,  0.0,trajectory);

    // 4. 判断并执行
    // 如果规划返回的完成比例等于 1，说明完全成功。
    // ⚠️ 严重隐患：在 C++ 中千万不要直接用 `== 1` 判断浮点数是否相等！
    // 因为浮点数精度问题，即使完全成功，返回值也可能是 0.9999999。
    if (fraction == 1) {
        // 如果路径规划完整成功，则让机械臂执行该轨迹
        arm.execute(trajectory);
    }

    // 7. 关闭 ROS 2 通信，结束程序
    rclcpp::shutdown();
    spinner.join();  // 等待子线程结束，确保资源释放干净
    return 0;
}