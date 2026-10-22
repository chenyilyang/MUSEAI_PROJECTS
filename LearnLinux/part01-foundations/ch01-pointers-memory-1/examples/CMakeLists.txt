cmake_minimum_required(VERSION 3.16)
project(ch01_examples CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(ex01_address ex01_address.cpp)
add_executable(ex02_deref ex02_deref.cpp)
add_executable(ex03_pointer_and_array ex03_pointer_and_array.cpp)
add_executable(ex04_pointer_arith ex04_pointer_arith.cpp)
