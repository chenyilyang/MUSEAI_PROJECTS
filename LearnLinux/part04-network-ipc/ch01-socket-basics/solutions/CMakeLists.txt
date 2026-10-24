cmake_minimum_required(VERSION 3.16)
project(ch01_socket_solutions CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Werror)

find_package(Threads REQUIRED)

add_executable(ex02_multi_round_echo ex02_multi_round_echo.cpp)
add_executable(ex03_udp_echo ex03_udp_echo.cpp)

foreach(t ex02_multi_round_echo ex03_udp_echo)
  target_link_libraries(${t} Threads::Threads)
endforeach()
