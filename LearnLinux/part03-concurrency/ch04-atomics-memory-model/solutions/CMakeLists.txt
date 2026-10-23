cmake_minimum_required(VERSION 3.16)
project(ch04_atomics_solutions CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

find_package(Threads REQUIRED)

add_executable(ex03_run_once ex03_run_once.cpp)
add_executable(ex04_ticket_dispenser ex04_ticket_dispenser.cpp)
target_link_libraries(ex03_run_once Threads::Threads)
target_link_libraries(ex04_ticket_dispenser Threads::Threads)
