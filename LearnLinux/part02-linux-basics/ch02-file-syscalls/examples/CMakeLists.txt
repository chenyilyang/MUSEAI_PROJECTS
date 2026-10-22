cmake_minimum_required(VERSION 3.16)
project(ch02_examples CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(ex01_open_write_read ex01_open_write_read.cpp)
add_executable(ex02_errno ex02_errno.cpp)
add_executable(ex03_lseek ex03_lseek.cpp)
add_executable(ex04_nonblock ex04_nonblock.cpp)
