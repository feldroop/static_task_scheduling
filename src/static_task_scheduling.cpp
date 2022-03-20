#include <iostream>
#include <fstream>

#include <algorithms/heft.hpp>
#include <cluster/cluster.hpp>
#include <io/handle_output.hpp>
#include <io/parse_command_line.hpp>
#include <io/read_csv.hpp>
#include <schedule/schedule.hpp>
#include <workflow/expand_task_bags.hpp>
#include <workflow/workflow.hpp>

int main(int argc, char *argv[]) {
    auto const args_option = io::parse_command_line(argc, argv);

    if (!args_option) {
        return -1;
    }

    auto const args = args_option.value();

    if (!args.output.empty()) {
        // truncate output file
        std::ofstream(args.output, std::ios::trunc);
    }

    auto const cluster_nodes = io::read_cluster_csv(args.cluster_input);
    cluster const c(std::move(cluster_nodes));

    io::handle_output<cluster>(args, c);

    auto const task_bags = io::read_task_bag_csv(args.task_bag_input);
    auto const [tasks, input_data_sizes, output_data_sizes] = expand_task_bags(task_bags);

    auto const dependencies = io::read_dependency_csv(args.dependency_input);

    workflow::workflow const w(tasks, input_data_sizes, output_data_sizes, dependencies);
    
    io::handle_output(args, w, c.best_node_performance());

    schedule::schedule const s = algorithms::heft(c, w);

    io::handle_output(args, s, "HEFT", s.is_valid(w));

    if (!args.verbose) {
        std::cout << "HEFT makespan: " << s.get_makespan() << '\n';
    }

    return 0;
}
