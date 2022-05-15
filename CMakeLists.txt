cmake_minimum_required (VERSION 3.14)

project (static_task_scheduling CXX)

if (MSVC)
    add_compile_options (/W4 /WX)
else ()
    add_compile_options (-Wall -Wextra -Wpedantic)
endif ()

set (CMAKE_CXX_STANDARD 20 CACHE INTERNAL "Force C++20")
set (CMAKE_CXX_STANDARD_REQUIRED ON CACHE INTERNAL "Force C++20")

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# dependency: clipp
add_subdirectory (lib/clipp)

# dependency: fast-cpp-csv-parser
add_library (csv STATIC lib/csv/csv.h)
set_target_properties (csv PROPERTIES LINKER_LANGUAGE CXX)

# dependency: pugixml
add_subdirectory (lib/pugixml)

# dependency: tabulate
add_library (tabulate STATIC lib/tabulate/single_include/tabulate/tabulate.hpp)
set_target_properties (tabulate PROPERTIES LINKER_LANGUAGE CXX)

# target executable
add_executable (static_task_scheduling src/static_task_scheduling.cpp)

target_include_directories (static_task_scheduling PRIVATE include)
target_include_directories (static_task_scheduling PRIVATE lib/csv)
target_include_directories (static_task_scheduling PRIVATE lib/tabulate/single_include/tabulate)

target_link_libraries (static_task_scheduling clipp::clipp)
target_link_libraries (static_task_scheduling csv)
target_link_libraries (static_task_scheduling tabulate)
target_link_libraries (static_task_scheduling pugixml::pugixml)
