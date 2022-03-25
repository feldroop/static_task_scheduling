#pragma once

#include <functional>
#include <iostream>

#include <algorithms/algorithm.hpp>
#include <algorithms/cpop.hpp>
#include <algorithms/heft.hpp>
#include <cluster/cluster.hpp>
#include <io/command_line_arguments.hpp>
#include <io/handle_output.hpp>
#include <schedule/schedule.hpp>
#include <workflow/workflow.hpp>

namespace algorithms {

void execute(
    io::command_line_arguments const & args,
    cluster::cluster const & c,
    workflow::workflow const & w,
    algorithm const algo
) {
    schedule::schedule s(c);

    switch (algo) {
        case algorithm::HEFT: s = algorithms::heft(c, w);
        break;
        case algorithm::CPOP: s = algorithms::cpop(c, w);
        break;
    }

    std::string const algo_str = algorithms::to_string(algo);

    io::handle_output(args, s, algo_str, s.is_valid(w));

    if (!args.verbose) {
        std::cout << algo_str << " makespan: " << s.get_makespan() << '\n';
    }
}

} // namespace algorithms
