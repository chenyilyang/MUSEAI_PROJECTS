cmake_minimum_required(VERSION 3.16)
project(ch04_solutions CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(ex03_clamp_template ex03_clamp_template.cpp)
add_executable(ex04_minmax ex04_minmax.cpp)
