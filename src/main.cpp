#include <vector>

#include <algorithms/heft.hpp>
#include <cluster/cluster.hpp>
#include <schedule/schedule.hpp>
#include <workflow/workflow.hpp>

using namespace std;

cluster create_example_cluster() {
    vector<size_t> memories{
        1, 1, 1
    };

    vector<size_t> num_cores{
        1, 1, 1
    };
    
    vector<double> core_performances{
        1.0, 0.5, 2.0
    };

    vector<double> bandwidths{
        1.0, 1.0, 1.0
    };

    return create_cluster(memories, num_cores, core_performances, bandwidths);
}

workflow create_example_workflow() {
    vector<double> computation_costs{
        100.0,
        50.0, 50.0, 50.0, 50.0,
        40.0, 40.0, 40.0, 40.0,
        80.0
    };

    vector<size_t> memory_requirements{
        0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0
    };

    vector<size_t> from_ids{
        0, 0, 0, 0,
        1, 2, 3, 4,
        5, 6, 7, 8
    };

    vector<size_t> to_ids{
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 9, 9, 9
    };

    vector<double> weights{
        2.0, 2.0, 2.0, 2.0,
        4.0, 4.0, 4.0, 4.0,
        5.0, 5.0, 5.0, 5.0
    };

    return create_workflow(computation_costs, memory_requirements, from_ids, to_ids, weights);
}

int main(int argc, char const *argv[]) {
    // illustrative example
    cluster const c = create_example_cluster();
    workflow const w = create_example_workflow();
    schedule const s = heft(c, w);

    print_schedule(s);

    return 0;
}
