cmake_minimum_required(VERSION 3.16)
project(ch01_solutions CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(ex03_add_ten ex03_add_ten.cpp)
add_executable(ex04_max_by_pointer ex04_max_by_pointer.cpp)
