# static_task_scheduling

Implementations of different static task scheduling algorithms to evaluate a proposed MILP based model for this problem.

## Implemented algorithms

* Original HEFT (https://ieeexplore.ieee.org/document/993206)
* Original CPOP (https://ieeexplore.ieee.org/document/993206)

## Setup

* Requires CMake and a C++20 capable compiler
* Example of build and run on linux:
```
git clone --recurse-submodules https://github.com/Felix-Droop/static_task_scheduling
mkdir build && cd build 
cmake ../static_task_scheduling -DCMAKE_BUILD_TYPE=Release
make
./bin/static_task_scheduling --help
```
## Usage

Look at the help menu for a detailed description of all possible flags. The main
differences are in the ways how task dependencies are read or generated for the 
input workflow.

### Examples

* Get dependencies from a .csv file with `-d` flag:
```
./static_task_scheduling -c cluster.csv -t task_bags.csv -d dependencies.csv
```

* Get dependencies from an .xml file with `-d` flag:
```
./static_task_scheduling -c cluster.csv -t task_bags.csv -d workflow_spec.xml
```

* Infer dependencies for a given workflow topology with the `-p` flag:
```
./static_task_scheduling -c cluster.csv -t epigenome_bags.csv -p epigenome
```
This works for `epigenome`, `cybershake` and `ligo` workflows.

* Get complex dependencies from an .xml file with additional processing with `-d` and `-p`:
```
./static_task_scheduling -c cluster.csv -t epigenome_bags.csv -d montage.xml -p montage
```
This is currently only needed for `montage` workflows.
