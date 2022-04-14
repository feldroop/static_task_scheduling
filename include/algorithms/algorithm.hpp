#pragma once

#include <string>

namespace algorithms {

enum class algorithm {
    HEFT, CPOP, RBCA, DBCA
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

} // namespace algorithms
