cmake_minimum_required(VERSION 3.16)

project(tp_gstreamer LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(monitor_sender src/monitor_sender.cpp)

target_compile_options(monitor_sender PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_link_libraries(monitor_sender PRIVATE PkgConfig::GSTREAMER)
