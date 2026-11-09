cmake_minimum_required(VERSION 3.16)

project(tp_gstreamer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(monitor_sender src/monitor_sender.cpp)

target_compile_options(monitor_sender PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
