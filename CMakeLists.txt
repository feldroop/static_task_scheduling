cmake_minimum_required(VERSION 3.16)

project (static_task_scheduling CXX)

set(CMAKE_CXX_STANDARD 20 CACHE INTERNAL "Force C++20")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE INTERNAL "Force C++20")

include_directories(include)

add_subdirectory(src)
