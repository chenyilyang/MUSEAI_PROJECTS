cmake_minimum_required(VERSION 3.16)
project(ch04_solutions CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(ex03_practice_daemon ex03_practice_daemon.cpp)
add_executable(ex04_log_dir ex04_log_dir.cpp)
