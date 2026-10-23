cmake_minimum_required(VERSION 3.16)
project(ch06_threadpool_solutions CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Werror)

find_package(Threads REQUIRED)

add_executable(ex02_pool_on_blocking_queue ex02_pool_on_blocking_queue.cpp)
add_executable(ex03_jthread_lifecycle ex03_jthread_lifecycle.cpp)
target_compile_features(ex03_jthread_lifecycle PRIVATE cxx_std_20)

target_link_libraries(ex02_pool_on_blocking_queue Threads::Threads)
target_link_libraries(ex03_jthread_lifecycle Threads::Threads)
