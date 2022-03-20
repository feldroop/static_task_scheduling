#pragma once

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <io/command_line_arguments.hpp>

namespace io {

template<typename T, typename... Args>
void handle_output(command_line_arguments const & args, T const & out_obj, Args&&... to_str_params) {
    if (!args.verbose && args.output.empty()) {
        return;
    }

    std::string out_str = out_obj.to_string(to_str_params...);

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

} // namespace io
