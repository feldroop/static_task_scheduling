# static_task_scheduling

Implementations of different static task scheduling algorithms to evaluate a proposed MILP based model for this problem.

## Implemented algorithms

* Original HEFT (https://ieeexplore.ieee.org/document/993206)
* Original CPOP (https://ieeexplore.ieee.org/document/993206)

## Setup

* Requires CMake and a C++20 capable compiler
* Example of build and run on linux:
```
git clone --recurse_submodules https://github.com/Felix-Droop/static_task_scheduling
mkdir build && cd build 
cmake ../static_task_scheduling -DCMAKE_BUILD_TYPE=Release
./bin/static_task_scheduling --help
```
