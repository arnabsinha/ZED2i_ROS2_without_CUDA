#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>

#include "sensorcapture.hpp"
#include <unistd.h>
#include <iostream>



class ImuNode : public rclcpp::Node {
public:
    ImuNode() : Node("imu_node"), sens_(sl_oc::VERBOSITY::INFO) {

        auto devs = sens_.getDeviceList(true);
        if (devs.size() == 0) {
            RCLCPP_ERROR(this->get_logger(), "No ZED camera found");
            rclcpp::shutdown();
        }

        if (!sens_.initializeSensors(devs[0])) {
            RCLCPP_ERROR(this->get_logger(), "Sensor init failed");
            rclcpp::shutdown();
        }
        pub_ = this->create_publisher<sensor_msgs::msg::Imu>("~/data", 1000);

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(4),  // ~250 Hz
            std::bind(&ImuNode::read_imu, this));
    }

private:
    void init_sensor() {

    }
    void read_imu() {
        const sl_oc::sensors::data::Imu imuData = sens_.getLastIMUData(10);

        if (imuData.valid == sl_oc::sensors::data::Imu::NEW_VAL) {

            auto msg = sensor_msgs::msg::Imu();

            msg.header.stamp = this->get_clock()->now();
	    msg.header.frame_id = "imu";

            msg.linear_acceleration.x = imuData.aX;
            msg.linear_acceleration.y = imuData.aY;
            msg.linear_acceleration.z = imuData.aZ;

            msg.angular_velocity.x = imuData.gX;
            msg.angular_velocity.y = imuData.gY;
            msg.angular_velocity.z = imuData.gZ;

	    //std::cout<<imuData.aX<<" "<<imuData.aY<<" "<<imuData.aZ<<std::endl;

            pub_->publish(msg);
        }
    }

    sl_oc::sensors::SensorCapture sens_;
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
	//auto imu_ = std::make_shared<ImuNode>();
    rclcpp::spin(std::make_shared<ImuNode>());
	//rclcpp::spin(imu_);
    rclcpp::shutdown();
    return 0;
}
