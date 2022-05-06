#pragma once

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <io/command_line_arguments.hpp>
#include <schedule/schedule.hpp>
#include <workflow/workflow.hpp>

namespace io {

void handle_output_str(command_line_arguments const & args, std::string const & out_str) {
    if (!args.verbose && args.output.empty()) {
        return;
    }

    if (args.verbose) {
        std::cout << out_str;
        std::cout.flush();
    }

    if (!args.output.empty()) {
        std::ofstream fout(args.output, std::ios::app | std::ios::out);

        if (fout.fail() || !fout.is_open()) {
            throw std::runtime_error("Could not open the output file " + args.output);
        }

        fout << out_str;
    }
}

template<typename T, typename... Args>
void handle_output_obj(command_line_arguments const & args, T const & out_obj, Args&&... to_str_params) {
    std::string out_str = out_obj.to_string(to_str_params...);

    handle_output_str(args, out_str);
}

void handle_computed_schedule_output(
    std::string const & algo_str,
    std::string const formatted_cpu_time,
    command_line_arguments const & args,
    schedule::schedule const & sched,
    workflow::workflow const & w
) {
    bool const valid = sched.is_valid(w);

    io::handle_output_obj(args, sched, algo_str, valid);
    io::handle_output_str(args, algo_str + " -- CPU running time: " + formatted_cpu_time + "\n\n");

    if (!args.verbose) {
        std::cout << algo_str << " makespan: " << sched.get_makespan() << ' '
            << (valid ? "(" : "(NOT ") << "valid) -- CPU running time: " 
            << formatted_cpu_time << '\n';
    }
}

} // namespace io
