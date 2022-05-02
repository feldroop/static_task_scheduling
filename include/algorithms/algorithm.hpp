#pragma once

#include <array>
#include <functional>
#include <string>

#include <algorithms/cpop.hpp>
#include <algorithms/dbca.hpp>
#include <algorithms/heft.hpp>
#include <algorithms/rbca.hpp>
#include <cluster/cluster.hpp>
#include <io/command_line_arguments.hpp>
#include <schedule/schedule.hpp>
#include <workflow/workflow.hpp>

namespace algorithms {

enum class algorithm {
    HEFT, CPOP, RBCA, DBCA
};

std::array<algorithm, 4> constexpr ALL = {
    algorithm::HEFT,
    algorithm::CPOP,
    algorithm::RBCA,
    algorithm::DBCA
};

std::string to_string(algorithm const algo) {
    std::string s;

    switch (algo) {
        case algorithm::HEFT: s = "HEFT";
        break;
        case algorithm::CPOP: s = "CPOP";
        break;
        case algorithm::RBCA: s = "RBCA";
        break;
        case algorithm::DBCA: s = "DBCA";
    }

    return s;
}

std::function<schedule::schedule()> to_function(
    algorithm const algo,
    cluster::cluster const & c,
    workflow::workflow const & w,
    io::command_line_arguments const & args
) {
    switch (algo) {
        case algorithm::HEFT: return [&] () {
            return algorithms::heft(c, w, args);
        };
        case algorithm::CPOP: return [&] () {
            return algorithms::cpop(c, w, args);
        };
        case algorithm::RBCA: return [&] () {
            return algorithms::rbca(c, w, args);
        };
        case algorithm::DBCA: return [&] () {
            return algorithms::dbca(c, w, args);
        };
        default:
            throw std::runtime_error("Internal bug: unknown algorithm.");
    }
}

} // namespace algorithms
