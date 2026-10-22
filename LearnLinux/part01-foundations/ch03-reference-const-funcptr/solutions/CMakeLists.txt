cmake_minimum_required(VERSION 3.16)
project(ch03_solutions CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(ex03_clamp ex03_clamp.cpp)
add_executable(ex04_transform ex04_transform.cpp)
