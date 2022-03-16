#include <iostream>
#include <vector>

#include <algorithms/heft.hpp>
#include <cluster.hpp>
#include <schedule/schedule.hpp>
#include <workflow.hpp>

cluster create_example_cluster() {
    std::vector<size_t> memories{
        1, 1, 1
    };

    std::vector<size_t> num_cores{
        1, 1, 1
    };
    
    std::vector<double> core_performances{
        1.0, 0.5, 2.0
    };

    std::vector<double> bandwidths{
        1.0, 1.0, 1.0
    };

    return cluster(memories, num_cores, core_performances, bandwidths);
}

workflow create_example_workflow() {
    std::vector<double> computation_costs{
        100.0,
        50.0, 50.0, 50.0, 50.0,
        40.0, 40.0, 40.0, 40.0,
        80.0
    };

    std::vector<size_t> memory_requirements{
        0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0
    };

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

    std::vector<double> weights{
        2.0, 2.0, 2.0, 2.0,
        4.0, 4.0, 4.0, 4.0,
        5.0, 5.0, 5.0, 5.0
    };

    return workflow(computation_costs, memory_requirements, from_ids, to_ids, weights);
}

int main() {
    // illustrative example
    cluster const c = create_example_cluster();
    workflow const w = create_example_workflow();

    schedule::schedule const s = algorithms::heft(c, w);

    std::cout << "[sequential makespan " 
              << w.get_sequential_makespan(c.best_node_performance())
              << "]\n";

    s.print();

    return 0;
}
