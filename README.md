# static_task_scheduling

![example event parameter](https://github.com/Felix-Droop/static_task_scheduling/actions/workflows/cmake.yml/badge.svg?event=push)

Implementations of different static task scheduling algorithms to evaluate a proposed MILP based model for this problem.

## Implemented algorithms

* Original HEFT (https://ieeexplore.ieee.org/document/993206)
* Original CPOP (https://ieeexplore.ieee.org/document/993206)

## Setup

* Requires CMake and a C++20 capable compiler with ranges support (e.g. `g++-10` or newer)
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

```
SYNOPSIS
        static_task_scheduling -c <cluster_file> -t <tasks_file> [-p <topology>] [-d
                               <dependencies_file>] [-o <output_file>] [-v] [-m]

OPTIONS
        Input
            -c, --cluster <cluster_file>
                    File in .csv format that describes the cluster architecture. It should contain
                    exactly the fields bandwidth, performance, memory and num_cores.

            -t, --tasks <tasks_file>
                    File in .csv format that describes the tasks of the workflow. It should contain
                    exactly the fields workload, input_data_size, output_data_size, memory and
                    cardinality.

            -p, --topology <topology>
                    Desired topology of the workflow. If no dependency file is given, the
                    dependencies will be inferred from the task bags using this configuration. Must
                    be one of: epigenome, cybershake, ligo or montage. For the montage workflow
                    topology, a dependency file must be given.

            -d, --dependencies <dependencies_file>
                    File that contains the dependencies for the workflow tasks. Can either be in csv
                    format or in xml format. A csv file should contain exactly the fields from_id
                    and to_id. An xml file should model the schema at
                    https://pegasus.isi.edu/schema/dax-2.1.xsd. For files in xml format it is
                    assumed that the jobs in the file are specified in a level order of the DAG
                    implied by the task bags.

        Output
            -o, --output <output_file>
                    If given, the verbose output of this program is written to this file as plain
                    text.

            -v, --verbose
                    If given, all metrics and the full solution are printed to the command line.

        -m, --use-memory-requirements
                    If given, tasks are only scheduled onto cluster nodes with sufficient memory.
                    This is not part of the original HEFT and CPOP and is deactivated by default.
```

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
  
* Write verbose output to command line with `-v` and write the same verbose output to a file with `-o`:
  ```
  ./static_task_scheduling -c cluster.csv -t task_bags.csv -d dependencies.csv -v -o output.txt
  ```

