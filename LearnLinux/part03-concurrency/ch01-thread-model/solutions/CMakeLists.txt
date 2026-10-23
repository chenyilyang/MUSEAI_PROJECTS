cmake_minimum_required(VERSION 3.16)
project(ch01_thread_solutions CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

find_package(Threads REQUIRED)

add_executable(ex03_parallel_sum ex03_parallel_sum.cpp)
add_executable(ex04_pthread_sum ex04_pthread_sum.cpp)
target_link_libraries(ex03_parallel_sum Threads::Threads)
target_link_libraries(ex04_pthread_sum Threads::Threads)
