cmake_minimum_required(VERSION 3.16)
project(ch05_safety_examples CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

find_package(Threads REQUIRED)

add_executable(bad_01_race bad_01_race.cpp)
add_executable(bad_02_deadlock bad_02_deadlock.cpp)
add_executable(ex01_deadlock_fixed ex01_deadlock_fixed.cpp)
add_executable(ex02_tsan_clean_counter ex02_tsan_clean_counter.cpp)

foreach(t bad_01_race bad_02_deadlock ex01_deadlock_fixed ex02_tsan_clean_counter)
  target_link_libraries(${t} Threads::Threads)
endforeach()
