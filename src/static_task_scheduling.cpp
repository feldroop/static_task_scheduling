#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include <algorithms/algorithm.hpp>
#include <algorithms/handle_execution.hpp>
#include <cluster/cluster.hpp>
#include <io/handle_output.hpp>
#include <io/parse_command_line.hpp>
#include <io/read_dependency_file.hpp>
#include <workflow/expand_task_bags.hpp>
#include <workflow/topology/infer_dependencies.hpp>
#include <workflow/topology/remove_bag_dependencies.hpp>
#include <workflow/topology/topology.hpp>
#include <workflow/workflow.hpp>

int main(int argc, char * argv[]) {
    auto const args_option = io::parse_command_line(argc, argv);

    if (!args_option) {
        return -1;
    }

    auto const args = args_option.value();

    if (!args.output.empty()) {
        // truncate output file
        std::ofstream(args.output, std::ios::trunc);
    }

    std::cout << std::fixed << std::setprecision(2);

    auto const cluster_nodes = io::read_cluster_csv(args.cluster_input);
    cluster::cluster const c(std::move(cluster_nodes));

    io::handle_output<cluster::cluster>(args, c);

    auto const task_bags = io::read_task_bag_csv(args.task_bag_input);
    auto const [tasks, input_data_sizes, output_data_sizes] = expand_task_bags(task_bags);

    workflow::topology::topology const top = workflow::topology::from_string(args.topology);
    std::vector<workflow::task_dependency> dependencies;

    if (args.dependency_input.empty()) {
        dependencies = workflow::topology::infer_dependencies(top, task_bags);
    } else {
        dependencies = io::read_dependency_file(args.dependency_input);
    }

    if (top == workflow::topology::topology::montage) {
        // remove specific edges from the workflow which our model can't handle
        workflow::topology::remove_bag_dependencies(dependencies, 0, 4, task_bags);
    }

    workflow::workflow const w(tasks, input_data_sizes, output_data_sizes, dependencies);

    io::handle_output(args, w, c.best_performance());

    for (auto const & algo : algorithms::ALL) {
        algorithms::handle_execution(algo, args, c, w);
    }

    return 0;
}
