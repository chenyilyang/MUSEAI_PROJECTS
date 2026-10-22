cmake_minimum_required(VERSION 3.16)
project(ch06_solutions CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(ex03_datablock ex03_datablock.cpp)
add_executable(ex04_add_move ex04_add_move.cpp)
