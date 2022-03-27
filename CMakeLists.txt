cmake_minimum_required(VERSION 3.4)

project (static_task_scheduling CXX)

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 20 CACHE INTERNAL "Force C++20")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE INTERNAL "Force C++20")

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# dependency: clipp
add_subdirectory(lib/clipp)

# dependency: fast-cpp-csv-parser
add_library(csv_lib STATIC lib/csv/csv.h)
set_target_properties(csv_lib PROPERTIES LINKER_LANGUAGE CXX)

# dependency: pugixml
add_subdirectory(lib/pugixml)

add_executable(static_task_scheduling src/static_task_scheduling.cpp)

target_include_directories(static_task_scheduling PRIVATE include)
target_include_directories(static_task_scheduling PRIVATE lib/csv)

target_link_libraries(static_task_scheduling clipp::clipp)
target_link_libraries(static_task_scheduling csv_lib)
target_link_libraries(static_task_scheduling pugixml::pugixml)
