#include <iostream>
#include <vector>

#include <algorithms/heft.hpp>
#include <cluster.hpp>
#include <io/command_line_interface.hpp>
#include <schedule/schedule.hpp>
#include <workflow.hpp>

cluster create_example_cluster() {
    std::vector<size_t> memories{
        50, 100, 200
    };

    std::vector<size_t> num_cores{
        1, 1, 1
    };
    
    std::vector<double> core_performances{
        10.0, 5.0, 20.0
    };

    std::vector<double> bandwidths{
        5.0, 5.0, 5.0
    };

    return cluster(memories, num_cores, core_performances, bandwidths);
}

workflow create_example_workflow() {
    // tasks
    std::vector<double> computation_costs{
        1000.0,
        500.0, 500.0, 500.0, 500.0,
        400.0, 400.0, 400.0, 400.0,
        800.0
    };

    std::vector<size_t> memory_requirements{
        1,
        1, 1, 1, 1,
        1, 1, 1, 1,
        1
    };

    // discreptancy with Somayeh's input data?
    std::vector<double> input_data_sizes{
        0.0,
        10.0, 10.0, 10.0, 10.0,
        20.0, 20.0, 20.0, 20.0,
        25.0
    };

    std::vector<double> output_data_sizes{
        10.0,
        20.0, 20.0, 20.0, 20.0,
        25.0, 25.0, 25.0, 25.0,
        0.0
    };

    // edge dependencies
    std::vector<workflow::task_id> from_ids{
        0, 0, 0, 0,
        1, 2, 3, 4,
        5, 6, 7, 8
    };

    std::vector<workflow::task_id> to_ids{
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 9, 9, 9
    };

    return workflow(
        computation_costs, 
        memory_requirements, 
        input_data_sizes,
        output_data_sizes,
        from_ids, 
        to_ids
    );
}

int main() {
    command_line_interface cli{};

    // illustrative example
    cluster const c = create_example_cluster();
    workflow const w = create_example_workflow();

    schedule::schedule const s = algorithms::heft(c, w);

    std::cout << "[sequential makespan: " 
              << w.get_sequential_makespan(c.best_node_performance())
              << "]\n";

    s.print();

    std::cout << "[schedule " << (s.is_valid(w) ? "is " : "not ") << "valid]\n";

    return 0;
}
