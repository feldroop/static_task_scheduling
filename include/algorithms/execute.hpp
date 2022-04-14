#pragma once

#include <functional>
#include <iostream>

#include <algorithms/algorithm.hpp>
#include <algorithms/cpop.hpp>
#include <algorithms/dbca.hpp>
#include <algorithms/heft.hpp>
#include <algorithms/rbca.hpp>
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
    schedule::schedule s(c, args.use_memory_requirements);

    switch (algo) {
        case algorithm::HEFT: s = algorithms::heft(c, w, args.use_memory_requirements);
        break;
        case algorithm::CPOP: s = algorithms::cpop(c, w, args.use_memory_requirements);
        break;
        case algorithm::RBCA: s = algorithms::rbca(c, w, args.use_memory_requirements);
        break; 
        case algorithm::DBCA: s = algorithms::dbca(c, w, args.use_memory_requirements);
        break;
    }

    std::string const algo_str = algorithms::to_string(algo);

    bool const valid = s.is_valid(w);

    io::handle_output(args, s, algo_str, valid);

    if (!args.verbose) {
        std::cout << algo_str << " makespan: " << s.get_makespan() << ' '
            << (valid ? "(" : "(NOT ") << "valid)\n";
    }
}

} // namespace algorithms
