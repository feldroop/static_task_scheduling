#pragma once

#include <string>

namespace algorithms {

enum class algorithm {
    HEFT, CPOP
};

std::string to_string(algorithm const algo) {
    std::string s;

    switch (algo) {
        case algorithm::HEFT: s = "HEFT";
        break;
        case algorithm::CPOP: s = "CPOP";
        break;
    }

    return s;
}

} // namespace algorithms
