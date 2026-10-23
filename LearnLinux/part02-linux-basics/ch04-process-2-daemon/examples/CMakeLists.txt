cmake_minimum_required(VERSION 3.16)
project(ch04_examples CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(ex01_session_group ex01_session_group.cpp)
add_executable(ex02_daemon ex02_daemon.cpp)
add_executable(ex03_environ ex03_environ.cpp)
