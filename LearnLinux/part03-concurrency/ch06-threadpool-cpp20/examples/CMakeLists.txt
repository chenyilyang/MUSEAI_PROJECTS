cmake_minimum_required(VERSION 3.16)
project(ch06_threadpool_examples CXX)

# 基线仍是 C++11；C++20 专题目标用 target_compile_features 单独抬到 C++20，
# 不污染基线目标——这正是"基线 + 专题"在本教程里的落地方式。
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Werror)

find_package(Threads REQUIRED)

add_executable(ex01_thread_pool ex01_thread_pool.cpp)

add_executable(ex02_jthread ex02_jthread.cpp)
add_executable(ex03_sem_latch_barrier ex03_sem_latch_barrier.cpp)
add_executable(ex04_coroutine_peek ex04_coroutine_peek.cpp)
foreach(t ex02_jthread ex03_sem_latch_barrier ex04_coroutine_peek)
  target_compile_features(${t} PRIVATE cxx_std_20)
endforeach()
# GCC 在 14 之前需要显式打开协程开关（-std=c++20 不自动带）。
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(ex04_coroutine_peek PRIVATE -fcoroutines)
endif()

foreach(t ex01_thread_pool ex02_jthread ex03_sem_latch_barrier ex04_coroutine_peek)
  target_link_libraries(${t} Threads::Threads)
endforeach()
