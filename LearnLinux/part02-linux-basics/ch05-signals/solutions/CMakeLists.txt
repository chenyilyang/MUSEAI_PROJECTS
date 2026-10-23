cmake_minimum_required(VERSION 3.16)
project(ch05_solutions CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(ex03_graceful ex03_graceful.cpp)
add_executable(ex04_reap8 ex04_reap8.cpp)
