cmake_minimum_required(VERSION 3.16)
project(ch05_examples CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

add_executable(ex01_signal_vs_sigaction ex01_signal_vs_sigaction.cpp)
add_executable(ex02_sigchld_reap ex02_sigchld_reap.cpp)
add_executable(ex03_timer_tick ex03_timer_tick.cpp)
