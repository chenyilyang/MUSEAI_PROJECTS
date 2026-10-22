cmake_minimum_required(VERSION 3.16)
project(ch02_solutions CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(ex03_alloc_array ex03_alloc_array.cpp)
add_executable(ex04_clone_array ex04_clone_array.cpp)
