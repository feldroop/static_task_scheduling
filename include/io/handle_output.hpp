#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <io/command_line_arguments.hpp>
#include <schedule/schedule.hpp>
#include <workflow/workflow.hpp>

#include <tabulate.hpp>

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

void print_node_communication_matrix(
    command_line_arguments const & args,
    std::vector<std::vector<double>> const & node_communication,
    std::string const & algo_str
) {
    tabulate::Table outer_table;
    outer_table.add_row({"Node communications in " + algo_str + " schedule:"});

    tabulate::Table inner_table;

    tabulate::Table::Row_t header;
    header.push_back("source\\target");
    for (auto const i : std::ranges::iota_view{0ul, node_communication.size()}) {
        header.push_back(std::to_string(i));
    }

    inner_table.add_row(header);

    size_t node_id{0};
    for (auto const & data_row : node_communication) {
        tabulate::Table::Row_t out_row;

        out_row.push_back(std::to_string(node_id));
        ++node_id;

        for (auto const & data_transfer : data_row) {
            std::stringstream double_to_str;
            double_to_str << std::fixed << std::setprecision(2) << data_transfer;

            out_row.push_back(double_to_str.str());
        }

        inner_table.add_row(out_row);
    }

    outer_table.add_row({inner_table});

    io::handle_output_str(args, outer_table.str() += "\n\n");
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

    if (valid) {
        auto const node_communication = sched.compute_node_communication_matrix(w);
        print_node_communication_matrix(args, node_communication, algo_str);
    }
}

} // namespace io
