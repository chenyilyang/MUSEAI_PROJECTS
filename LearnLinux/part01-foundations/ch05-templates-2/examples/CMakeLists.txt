cmake_minimum_required(VERSION 3.16)
project(ch05_examples CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(ex01_specialization ex01_specialization.cpp)
add_executable(ex02_partial_spec ex02_partial_spec.cpp)
add_executable(ex03_nontype_param ex03_nontype_param.cpp)
add_executable(ex04_variadic_print ex04_variadic_print.cpp)
