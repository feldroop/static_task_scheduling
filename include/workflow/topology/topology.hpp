#pragma once 

#include <stdexcept>
#include <string>

namespace workflow::topology {

enum class topology {
    epigenome, cybershake, ligo, montage
};

topology from_string(std::string const & s) {
    if (s == "epigenome") {
        return topology::epigenome;
    } else if (s == "cybershake") {
        return topology::cybershake;
    } else if (s == "ligo") {
        return topology::ligo;
    } else if (s == "montage") {
        return topology::montage;
    } else {
        throw std::runtime_error("The given topology has an invalid or unknown value.");
    }
}

} // namespace workflow::topology
