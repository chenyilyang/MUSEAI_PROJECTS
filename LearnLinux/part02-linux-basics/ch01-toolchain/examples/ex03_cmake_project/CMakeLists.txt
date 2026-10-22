cmake_minimum_required(VERSION 3.16)
project(demo_multifile CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

# 一个可执行文件 = 全部源文件在链接期拼到一起
add_executable(demo main.cpp greet.cpp calc.cpp)
