cmake_minimum_required(VERSION 3.16)
project(ch04_examples CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(ex01_func_template ex01_func_template.cpp)
add_executable(ex02_deduction ex02_deduction.cpp)
add_executable(ex03_class_template ex03_class_template.cpp)
add_executable(ex04_overload_with_template ex04_overload_with_template.cpp)
