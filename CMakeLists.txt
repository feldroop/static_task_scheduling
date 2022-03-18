cmake_minimum_required(VERSION 3.16)

project (static_task_scheduling CXX)

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 20 CACHE INTERNAL "Force C++20")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE INTERNAL "Force C++20")

include_directories(include)
include_directories(lib)

add_executable(static_task_scheduling src/static_task_scheduling.cpp)
