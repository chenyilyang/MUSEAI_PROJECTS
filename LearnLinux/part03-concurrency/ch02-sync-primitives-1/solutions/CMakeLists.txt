cmake_minimum_required(VERSION 3.16)
project(ch02_sync1_solutions CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

find_package(Threads REQUIRED)

add_executable(ex03_safe_counter ex03_safe_counter.cpp)
add_executable(ex04_striped_counter ex04_striped_counter.cpp)
target_link_libraries(ex03_safe_counter Threads::Threads)
target_link_libraries(ex04_striped_counter Threads::Threads)
