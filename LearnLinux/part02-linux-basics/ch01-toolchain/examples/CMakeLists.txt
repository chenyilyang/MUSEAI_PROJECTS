cmake_minimum_required(VERSION 3.16)
project(ch01_examples CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

# ex01 与 ex04 由本文件构建；ex02、ex03 按正文的命令手工构建（本身即教学内容）
add_executable(hello ex01_build_stages/hello.cpp)
add_executable(buggy_sum ex04_gdb/buggy_sum.cpp)
add_executable(fixed_sum ex04_gdb/fixed_sum.cpp)
