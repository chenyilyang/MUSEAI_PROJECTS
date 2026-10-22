cmake_minimum_required(VERSION 3.16)
project(ch03_examples CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(ex01_fork_basic ex01_fork_basic.cpp)
add_executable(ex02_fork_exec ex02_fork_exec.cpp)
add_executable(ex03_wait_status ex03_wait_status.cpp)
add_executable(ex04_zombie_orphan ex04_zombie_orphan.cpp)
