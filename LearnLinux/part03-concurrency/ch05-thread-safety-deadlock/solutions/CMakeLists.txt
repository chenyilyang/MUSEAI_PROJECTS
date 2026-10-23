cmake_minimum_required(VERSION 3.16)
project(ch05_safety_solutions CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

find_package(Threads REQUIRED)

add_executable(ex03_deadlock_two_fixes ex03_deadlock_two_fixes.cpp)
add_executable(ex04_word_frequency ex04_word_frequency.cpp)
target_link_libraries(ex03_deadlock_two_fixes Threads::Threads)
target_link_libraries(ex04_word_frequency Threads::Threads)
