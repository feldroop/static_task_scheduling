#pragma once

#include <iostream>
#include <string>

#include <io/command_line_arguments.hpp>

namespace io {

void issue_warning(command_line_arguments const & args, std::string const & str) {
    std::string const warning_str = "----- WARNING ---> " + str + '\n';
    
    handle_output_str(args, warning_str + '\n');

    if (!args.verbose) {
        std::cout << warning_str;
        std::cout.flush();
    }
}

} // namespace io
