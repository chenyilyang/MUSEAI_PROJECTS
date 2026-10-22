cmake_minimum_required(VERSION 3.16)
project(ch05_solutions CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(ex03_int_specialization ex03_int_specialization.cpp)
add_executable(ex04_sum_all ex04_sum_all.cpp)
