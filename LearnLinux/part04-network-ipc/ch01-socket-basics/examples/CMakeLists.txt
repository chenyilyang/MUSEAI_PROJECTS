cmake_minimum_required(VERSION 3.16)
project(ch01_socket_examples CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Werror)

find_package(Threads REQUIRED)

add_executable(ex01_byteorder ex01_byteorder.cpp)
add_executable(ex02_tcp_echo ex02_tcp_echo.cpp)
add_executable(ex03_send_recv_all ex03_send_recv_all.cpp)

foreach(t ex01_byteorder ex02_tcp_echo ex03_send_recv_all)
  target_link_libraries(${t} Threads::Threads)
endforeach()
