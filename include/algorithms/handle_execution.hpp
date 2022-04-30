#pragma once

#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <tuple>

#include <algorithms/algorithm.hpp>
#include <cluster/cluster.hpp>
#include <io/command_line_arguments.hpp>
#include <io/handle_output.hpp>
#include <schedule/schedule.hpp>
#include <workflow/workflow.hpp>

namespace algorithms {
// measures CPU time in clocks
std::tuple<schedule::schedule, std::clock_t> measure_execution(
    std::function<schedule::schedule()> const & func
) {
    std::clock_t const start = std::clock();
    schedule::schedule const s = func();
    std::clock_t const end = std::clock();

    return std::make_tuple(s, end - start);
}

std::string format_clocks(std::clock_t const clocks) {
    double const seconds = static_cast<double>(clocks) / static_cast<double>(CLOCKS_PER_SEC);
    
    std::stringstream out{};
    out << std::fixed << std::setprecision(2);

    if (seconds >= 1.0) {
        out << seconds << " seconds";
    } else if (seconds >= 0.001) {
        out << seconds * 1000.0 << " milliseconds";
    } else {
        out << seconds * 1000000.0 << " microseconds";
    }

    return out.str();
}

void handle_execution(
    algorithm const algo,
    io::command_line_arguments const & args,
    cluster::cluster const & c,
    workflow::workflow const & w
) {
    auto const func = algorithms::to_function(
        algo, c, w, args.use_memory_requirements, args.verbose
    );

    auto const [sched, cpu_time_clocks] = measure_execution(func);

    std::string const algo_str = algorithms::to_string(algo);
    bool const valid = sched.is_valid(w);

    io::handle_output(args, sched, algo_str, valid);

    if (!args.verbose) {
        std::cout << algo_str << " makespan: " << sched.get_makespan() << ' '
            << (valid ? "(" : "(NOT ") << "valid) -- cpu running time: " 
            << format_clocks(cpu_time_clocks) << '\n';
    }
}

} // namespace algorithms
