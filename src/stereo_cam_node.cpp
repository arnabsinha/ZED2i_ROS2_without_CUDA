#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <cstring>

#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>

#define WIDTH 1344
#define HEIGHT 376
#define DEVICE "/dev/video0"

struct Buffer {
    void* start;
    size_t length;
};

class StereoCamNode : public rclcpp::Node {
public:
    StereoCamNode() : Node("stereo_cam_node") {

        left_pub_ = this->create_publisher<sensor_msgs::msg::Image>("stereo/left_image_raw", 10);
        right_pub_ = this->create_publisher<sensor_msgs::msg::Image>("stereo/right_image_raw", 10);

        fd_ = open(DEVICE, O_RDWR);
        if (fd_ < 0) {
            perror("Cannot open device");
            rclcpp::shutdown();
        }

        // Set format
        v4l2_format fmt{};
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = WIDTH;
        fmt.fmt.pix.height = HEIGHT;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        fmt.fmt.pix.field = V4L2_FIELD_NONE;

        ioctl(fd_, VIDIOC_S_FMT, &fmt);

        // Request buffers
        v4l2_requestbuffers req{};
        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        ioctl(fd_, VIDIOC_REQBUFS, &req);

        buffers_ = new Buffer[req.count];

        for (unsigned int i = 0; i < req.count; i++) {
            v4l2_buffer buf{};
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

            ioctl(fd_, VIDIOC_QUERYBUF, &buf);

            buffers_[i].length = buf.length;
            buffers_[i].start = mmap(NULL, buf.length,
                                     PROT_READ | PROT_WRITE,
                                     MAP_SHARED,
                                     fd_, buf.m.offset);
        }

        for (unsigned int i = 0; i < req.count; i++) {
            v4l2_buffer buf{};
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

            ioctl(fd_, VIDIOC_QBUF, &buf);
        }

        int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioctl(fd_, VIDIOC_STREAMON, &type);

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(10),
            std::bind(&StereoCamNode::capture_frame, this));
    }

private:
    void capture_frame() {
	    struct v4l2_buffer buf{};
	    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	    buf.memory = V4L2_MEMORY_MMAP;

	    if (ioctl(fd_, VIDIOC_DQBUF, &buf) < 0) {
		RCLCPP_ERROR(this->get_logger(), "Failed to dequeue buffer.");
		return;
	    }

	    size_t step = WIDTH * 2;

	    try {
		cv::Mat yuyv(HEIGHT, WIDTH, CV_8UC2, buffers_[buf.index].start, step);

		if (yuyv.empty()) {
		    throw std::runtime_error("Empty YUYV frame captured");
		}

		cv::Mat bgr;
		cv::cvtColor(yuyv, bgr, cv::COLOR_YUV2BGR_YUYV);

		int half_width = bgr.cols / 2;

		cv::Mat left_img = bgr(cv::Rect(0, 0, half_width, bgr.rows));
		cv::Mat right_img = bgr(cv::Rect(half_width, 0, half_width, bgr.rows));

		auto stamp = this->now();

		auto left_msg = cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", left_img).toImageMsg();
		left_msg->header.stamp = stamp;
		left_msg->header.frame_id = "left_camera_frame";
		left_pub_->publish(*left_msg);

		auto right_msg = cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", right_img).toImageMsg();
		right_msg->header.stamp = stamp;
		right_msg->header.frame_id = "right_camera_frame";
		right_pub_->publish(*right_msg);
	    }
	    catch (const std::exception& e) {
		RCLCPP_ERROR(this->get_logger(), "Conversion error: %s", e.what());
	    }

	    if (ioctl(fd_, VIDIOC_QBUF, &buf) < 0) {
		RCLCPP_ERROR(this->get_logger(), "Failed to queue buffer.");
	    }
	}

    int fd_;
    Buffer* buffers_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr left_pub_;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr right_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};
int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<StereoCamNode>());
    rclcpp::shutdown();
    return 0;
}
