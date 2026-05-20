# ZED2i_ROS2_without_CUDA

## Installation of zed-open-capture
Go to 
```
https://github.com/stereolabs/zed-open-capture
```
and follow the following steps
```
sudo apt install build-essential cmake libusb-1.0-0-dev libhidapi-libusb0 libhidapi-dev 
```
```
git clone https://github.com/stereolabs/zed-open-capture.git
cd zed-open-capture
cd udev
bash install_udev_rule.sh
cd ..
mkdir build
cd build
cmake ..
make -j$(nproc)
```
```
sudo make install
sudo ldconfig
```


## Installation
```
cd ros2_ws/src
git clone https://github.com/arnabsinha/ZED2i_ROS2_without_CUDA zed_capture
cd ..
colcon build
```
## Running
```
cd ros2_ws
source install/setup.bash
ros2 launch zed_capture zed_capture.launch.py
```
## Recording

Open a new terminal with shift+ctrl+T
write the following to create a rosbag file to record the sensor data.
```
ros2 record /stereo/left_image_raw /stereo/right_image_raw /imu/data -O foderName
```
If you want to record only the IMU data do the following:
```
ros2 record /imu/data -O foderName
```
