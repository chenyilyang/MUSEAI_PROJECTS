cmake_minimum_required(VERSION 3.16)
project(taskqueue_logger_solutions CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Werror)

find_package(Threads REQUIRED)

add_executable(queue_check queue_check.cpp)
add_executable(logger_stress logger_stress.cpp)
add_executable(pool_check pool_check.cpp)

foreach(t queue_check logger_stress pool_check)
  target_link_libraries(${t} Threads::Threads)
endforeach()
