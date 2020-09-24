project(MoVIT-Pi)
cmake_minimum_required(VERSION 3.13)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS -Wall)
set(CMAKE_C_FLAGS -Wall)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Executables)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Executables)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Executables)


file(GLOB movitpi_cpp_srcs src/movit-pi/*.cpp)
#set(movitpi_c_srcs)
file(GLOB movitpi_c_srcs src/movit-pi/*.c)
file(GLOB movitpi_headers src/movit-pi/*.h)

message(STATUS "cpp: ${movitpi_cpp_srcs}")
message(STATUS "c: ${movitpi_c_srcs}")
message(STATUS "headers: ${movitpi_headers}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/movit-pi)
include_directories(/usr/local/include)

add_executable(movit-pi ${movitpi_cpp_srcs} ${movitpi_c_srcs} ${movitpi_headers})

link_directories(/usr/lib /usr/local/lib)
target_link_libraries(movit-pi wiringPi mosquitto wiringPi bcm2835 mosquittopp pthread)



# Tests (compile useful files only)
set(test-Alarm-srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/src/movit-pi/tests/test-Alarm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/movit-pi/Alarm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/movit-pi/Sensor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/movit-pi/Utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/movit-pi/SysTime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/movit-pi/PCA9536.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/movit-pi/I2Cdev.cpp

)
add_executable(test-Alarm ${test-Alarm-srcs} ${movitpi_headers})
target_link_libraries(test-Alarm wiringPi mosquitto wiringPi bcm2835 mosquittopp pthread)


# Tests (compile useful files only)
set(test-IMU-srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/src/movit-pi/tests/test-IMU.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/movit-pi/I2Cdev.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/movit-pi/MPU6050.cpp

)
add_executable(test-IMU ${test-IMU-srcs} ${movitpi_headers})
target_link_libraries(test-IMU wiringPi mosquitto wiringPi bcm2835 mosquittopp pthread)
