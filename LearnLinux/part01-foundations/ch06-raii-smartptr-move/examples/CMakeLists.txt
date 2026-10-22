cmake_minimum_required(VERSION 3.16)
project(ch06_examples CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(ex01_raii_file ex01_raii_file.cpp)
add_executable(ex02_unique_ptr ex02_unique_ptr.cpp)
add_executable(ex03_shared_weak ex03_shared_weak.cpp)
add_executable(ex04_move_semantics ex04_move_semantics.cpp)
