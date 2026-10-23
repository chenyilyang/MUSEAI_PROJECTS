cmake_minimum_required(VERSION 3.16)
project(ch02_sync1_examples CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

find_package(Threads REQUIRED)

add_executable(ex01_mutex_counter ex01_mutex_counter.cpp)
add_executable(ex02_raii_locks ex02_raii_locks.cpp)
add_executable(ex03_rwlock ex03_rwlock.cpp)
add_executable(ex04_lock_cost ex04_lock_cost.cpp)

foreach(t ex01_mutex_counter ex02_raii_locks ex03_rwlock ex04_lock_cost)
  target_link_libraries(${t} Threads::Threads)
endforeach()
