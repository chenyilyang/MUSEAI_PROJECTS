cmake_minimum_required(VERSION 3.16)
project(log_collector_starter CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(log_collector log_collector.cpp)
