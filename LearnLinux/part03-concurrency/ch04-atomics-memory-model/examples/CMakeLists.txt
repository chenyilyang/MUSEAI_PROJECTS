cmake_minimum_required(VERSION 3.16)
project(ch04_atomics_examples CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

find_package(Threads REQUIRED)

add_executable(ex01_atomic_counter ex01_atomic_counter.cpp)
add_executable(ex02_spinlock ex02_spinlock.cpp)
add_executable(ex03_publish_subscribe ex03_publish_subscribe.cpp)
add_executable(ex04_lockfree_stack ex04_lockfree_stack.cpp)

foreach(t ex01_atomic_counter ex02_spinlock ex03_publish_subscribe ex04_lockfree_stack)
  target_link_libraries(${t} Threads::Threads)
endforeach()
