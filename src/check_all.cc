#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <cstring>
#include <cstdint>

// Helper to convert frame intervals to FPS
void print_fps(const v4l2_frmivalenum& frmival) {
    if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
        double fps = static_cast<double>(frmival.discrete.denominator) / frmival.discrete.numerator;
        std::cout << "        - " << fps << " fps" << std::endl;
    } else {
        // Stepwise/Continuous range
        double max_fps = static_cast<double>(frmival.stepwise.min.denominator) / frmival.stepwise.min.numerator;
        double min_fps = static_cast<double>(frmival.stepwise.max.denominator) / frmival.stepwise.max.numerator;
        std::cout << "        - Range: " << min_fps << " to " << max_fps << " fps" << std::endl;
    }
}

int main() {
    int fd = open("/dev/video0", O_RDWR);
    if (fd == -1) {
        perror("Open device");
        return 1;
    }

    // 1. Loop through Pixel Formats
    v4l2_fmtdesc fmt = {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.index = 0;

    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == 0) {
        std::cout << "[Format] " << fmt.description << std::endl;

        // 2. Loop through Frame Sizes for this format
        v4l2_frmsizeenum frmsize = {};
        frmsize.pixel_format = fmt.pixelformat;
        frmsize.index = 0;

        while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
            if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                uint32_t w = frmsize.discrete.width;
                uint32_t h = frmsize.discrete.height;
                std::cout << "    > Resolution: " << w << "x" << h << std::endl;

                // 3. Loop through Frame Intervals for this size/format
                v4l2_frmivalenum frmival = {};
                frmival.pixel_format = fmt.pixelformat;
                frmival.width = w;
                frmival.height = h;
                frmival.index = 0;

                while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == 0) {
                    print_fps(frmival);
                    frmival.index++;
                }
            } else {
                std::cout << "    > Stepwise: " << frmsize.stepwise.min_width << "x" << frmsize.stepwise.min_height 
                          << " to " << frmsize.stepwise.max_width << "x" << frmsize.stepwise.max_height << std::endl;
            }
            frmsize.index++;
        }
        fmt.index++;
        std::cout << "---------------------------------------" << std::endl;
    }

    close(fd);
    return 0;
}

