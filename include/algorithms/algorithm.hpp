#pragma once

#include <array>
#include <functional>
#include <string>

#include <algorithms/cpop.hpp>
#include <algorithms/dbca.hpp>
#include <algorithms/heft.hpp>
#include <algorithms/rbca.hpp>
#include <cluster/cluster.hpp>
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
    bool const use_memory_requirements,
    bool const verbose
) {
    switch (algo) {
        case algorithm::HEFT: return [&c, &w, use_memory_requirements, verbose] () {
            return algorithms::heft(c, w, use_memory_requirements, verbose);
        };
        case algorithm::CPOP: return [&c, &w, use_memory_requirements, verbose] () {
            return algorithms::cpop(c, w, use_memory_requirements, verbose);
        };
        case algorithm::RBCA: return [&c, &w, use_memory_requirements, verbose] () {
            return algorithms::rbca(c, w, use_memory_requirements, verbose);
        };
        case algorithm::DBCA: return [&c, &w, use_memory_requirements, verbose] () {
            return algorithms::dbca(c, w, use_memory_requirements, verbose);
        };
        default:
            throw std::runtime_error("Internal bug: unknown algorithm.");
    }
}

} // namespace algorithms
