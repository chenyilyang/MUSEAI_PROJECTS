cmake_minimum_required(VERSION 3.16)
project(ch03_sync2_examples CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

find_package(Threads REQUIRED)

add_executable(ex01_condvar_handoff ex01_condvar_handoff.cpp)
add_executable(ex02_bounded_queue ex02_bounded_queue.cpp)
add_executable(ex03_posix_sem ex03_posix_sem.cpp)
add_executable(bad_01_lost_wakeup bad_01_lost_wakeup.cpp)

foreach(t ex01_condvar_handoff ex02_bounded_queue ex03_posix_sem bad_01_lost_wakeup)
  target_link_libraries(${t} Threads::Threads)
endforeach()
