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

# Convert ROS1 bag file to ROS2 bag
## Install conda
Follow the documentation at [this site](https://docs.conda.io/projects/conda/en/stable/user-guide/install/linux.html)

## Install converter
```
conda create --name bagConv
conda activate bagConv
conda config --add channels conda-forge
conda config --set channel_priority strict
conda install -c conda-forge rosbags
```
## Run the converter
```
conda activate bagConv
rosbags-convert --src dataset-calib-cam1_512_16.bag --dst tum_dataset-calib-cam1_512_16
```
Run the bag file
```
ros2 bag play tum_dataset-calib-cam1_512_16
```
Open another console with Shift+Ctrl+T
```
ros2 topic list
ros2 topic echo /imu0
```

# Install ORBSLAM3 in and run TUM within Ubuntu24.04
Download the "Dockerfile", provided in the codebase and run the following command
```
docker build -t orbslam .
```
Next to run the dockerfile, at first change the "dataset" folder within the file "docker-compose.yml" at line 19
```
- /home/vision/tumvi/tumvi/calibrated/512_16:/dataset
```
to your folder of TUM dataset. Next run this file with the following command
```
xhost +local:docker
docker compose up
```
This should run the ORBSLAM in ROS2 environment. Now to run the dataset, do the following. Open a new terminal and run
```
docker exec -it adamas_orbslam3 bash
```
Now inside this terminal run the following commands
```
cd /dataset
rosbags-convert dataset-calib-cam1_512_16.bag --dst tum_dataset-calib-cam1_512_16
ros2 bag play tum_dataset-calib-cam1_512_16
```
