cmake_minimum_required(VERSION 3.16)
project(ch02_examples CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(ex01_double_pointer ex01_double_pointer.cpp)
add_executable(ex02_heap_stack ex02_heap_stack.cpp)
add_executable(ex03_nullptr ex03_nullptr.cpp)

# 反例单独构建：它演示的是悬垂指针的未定义行为，默认不参与普通构建流程。
# 学习时请用 AddressSanitizer 构建它：
#   g++ -std=c++11 -g -fsanitize=address bad_01_dangling.cpp -o bad_01_dangling
add_executable(bad_01_dangling bad_01_dangling.cpp)
