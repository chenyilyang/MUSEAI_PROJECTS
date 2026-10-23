cmake_minimum_required(VERSION 3.16)
project(ch01_thread_examples CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

find_package(Threads REQUIRED)

add_executable(ex01_thread_basics ex01_thread_basics.cpp)
add_executable(ex02_args_and_ref ex02_args_and_ref.cpp)
add_executable(ex03_lifecycle ex03_lifecycle.cpp)
add_executable(bad_01_forgot_join bad_01_forgot_join.cpp)
add_executable(ex04_pthread ex04_pthread.cpp)
add_executable(ex05_race_glimpse ex05_race_glimpse.cpp)

foreach(t ex01_thread_basics ex02_args_and_ref ex03_lifecycle bad_01_forgot_join ex04_pthread ex05_race_glimpse)
  target_link_libraries(${t} Threads::Threads)
endforeach()
