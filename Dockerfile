FROM osrf/ros:foxy-desktop

RUN apt-get update && apt-get install -y \
    git \
    cmake \
    build-essential \
    sudo \
    wget \
    curl \
    vim \
    pkg-config \
    tar \
    libeigen3-dev \
    libboost-all-dev \
    libssl-dev \
    libgl1-mesa-dev \
    libglew-dev \
    libepoxy-dev \
    libopencv-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Python tools + colcon (already partly present in base)
RUN apt-get update && apt-get install -y \
    python3-pip \
    python3-colcon-common-extensions \
    && rm -rf /var/lib/apt/lists/*

# Python dependencies (as in your base image history)
RUN pip3 install --no-cache-dir \
    opencv-python==4.2.0.34 \
    numpy
# ----------------------------
# Thirdparty dependencies
# ----------------------------
WORKDIR /root/ORB_SLAM3/Thirdparty

# Catch2 (required by Pangolin tests/tools)
RUN git clone https://github.com/catchorg/Catch2.git && \
    cd Catch2 && \
    git checkout v3.4.0 && \
    cmake -B build -DBUILD_TESTING=OFF && \
    cmake --build build -j$(nproc) && \
    cmake --install build

# Pangolin
RUN git clone --recursive https://github.com/stevenlovegrove/Pangolin.git

WORKDIR /root/ORB_SLAM3/Thirdparty/Pangolin

# Build Pangolin
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j$(nproc) && \
    cmake --install build

RUN ldconfig


# ----------------------------
# ORB-SLAM3 setup
# ----------------------------
WORKDIR /root

RUN rm -rf ORB_SLAM3 && git clone https://github.com/zang09/ORB-SLAM3-STEREO-FIXED.git ORB_SLAM3

WORKDIR /root/ORB_SLAM3

# build ORB-SLAM3
RUN chmod +x build.sh && ./build.sh

# ----------------------------
# ROS2 workspace (ORB-SLAM3 wrapper)
# ----------------------------
WORKDIR /root/colcon_ws/src
RUN mkdir -p /root/colcon_ws/src

RUN git clone https://github.com/zang09/ORB_SLAM3_ROS2.git orbslam3_ros2

WORKDIR /root/colcon_ws

# build ROS2 package
RUN /bin/bash -c "source /opt/ros/foxy/setup.bash && colcon build --symlink-install --packages-select orbslam3_ros2 || true"

# ----------------------------
# Environment setup
# ----------------------------
RUN echo "source /opt/ros/foxy/setup.bash" >> ~/.bashrc && \
    echo "source /root/colcon_ws/install/setup.bash" >> ~/.bashrc && \
    echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib" >> ~/.bashrc

# ----------------------------
# Default runtime
# ----------------------------
WORKDIR /root
CMD ["/bin/bash"]
