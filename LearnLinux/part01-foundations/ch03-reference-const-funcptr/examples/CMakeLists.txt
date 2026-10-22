cmake_minimum_required(VERSION 3.16)
project(ch03_examples CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(ex01_reference ex01_reference.cpp)
add_executable(ex02_const_pointer ex02_const_pointer.cpp)
add_executable(ex03_ref_params ex03_ref_params.cpp)
add_executable(ex04_func_pointer ex04_func_pointer.cpp)
