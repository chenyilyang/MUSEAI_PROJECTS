cmake_minimum_required(VERSION 3.16)
project(ch03_sync2_solutions CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

find_package(Threads REQUIRED)

add_executable(ex03_blocking_queue ex03_blocking_queue.cpp)
add_executable(ex04_start_gate ex04_start_gate.cpp)
target_link_libraries(ex03_blocking_queue Threads::Threads)
target_link_libraries(ex04_start_gate Threads::Threads)
