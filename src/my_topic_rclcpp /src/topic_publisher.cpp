#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "example_interfaces/msg/bool.hpp"

class TopicPublisher02 : public rclcpp::Node
{
public:
    // 构造函数,有一个参数为节点名称
    TopicPublisher02(std::string name) : Node(name)
    {
        RCLCPP_INFO(this->get_logger(), "大家好，我是%s.", name.c_str());
        // 创建发布者
       command_publisher_ =
            this->create_publisher<example_interfaces::msg::Bool>("open_gripper", 10);
        // 创建定时器，500ms为周期，定时发布
        timer_ = this->create_wall_timer(std::chrono::milliseconds(500), 
                                            std::bind(&TopicPublisher02::timer_callback, this));
    
    }

private:
// 使用订阅者对应的消息类型，确保话题数据结构一致。
    
    void timer_callback()
    {
        // 创建消息
        example_interfaces::msg::Bool message;
        message.data = true;  // true：打开夹爪；false：关闭夹爪

        RCLCPP_INFO(this->get_logger(), "Publishing open_gripper: true");
        command_publisher_->publish(message);
        timer_->cancel();  // 只发一次
    }

    // 声名定时器指针
    rclcpp::TimerBase::SharedPtr timer_;
    // 声明话题发布者
    rclcpp::Publisher<example_interfaces::msg::Bool>::SharedPtr command_publisher_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    /*创建对应节点的共享指针对象*/
    auto node = std::make_shared<TopicPublisher02>("topic_publisher_02");
    /* 运行节点，并检测退出信号*/
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}