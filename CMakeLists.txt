cmake_minimum_required(VERSION 3.20)
project(zed_capture)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)

find_library(ZED_OC_LIB NAMES zed_open_capture PATHS /usr/local/lib)
add_definitions(-DSENSORS_MOD_AVAILABLE)
find_package(PkgConfig REQUIRED)

pkg_check_modules(HIDAPI REQUIRED hidapi-libusb)

#include_directories(${HIDAPI_INCLUDE_DIRS})
#link_directories(${HIDAPI_LIBRARY_DIRS})

add_executable(imu_node src/imu_node.cpp)
target_include_directories(imu_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
  ${HIDAPI_INCLUDE_DIRS}
  "/usr/local/include/zed-open-capture"
)
target_compile_features(imu_node PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
target_link_libraries(
  imu_node
  ${sensor_msgs_TARGETS}
  ${rclcpp_TARGETS}
  ${ZED_OC_LIB}
  #${USB_LIBRARIES}
  #${HIDAPI_LIBRARIES}
)

install(TARGETS imu_node
  DESTINATION lib/${PROJECT_NAME})

add_executable(stereo_cam_node src/stereo_cam_node.cpp)
target_include_directories(stereo_cam_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
  ${HIDAPI_INCLUDE_DIRS}
  "/usr/local/include/zed-open-capture"
)
target_compile_features(stereo_cam_node PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
target_link_libraries(
  stereo_cam_node
  ${sensor_msgs_TARGETS}
  ${rclcpp_TARGETS}
  ${cv_bridge_TARGETS}
  ${OpenCV_LIBS}
  ${ZED_OC_LIB}
  #${USB_LIBRARIES}
  #${HIDAPI_LIBRARIES}
)

install(TARGETS stereo_cam_node
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
