#pragma once 

#include <cmath>

namespace util {

template <typename T>
bool epsilon_eq(T const & t0, T const & t1, double const epsilon = 0.0000000001) {
    return std::abs(t0 - t1) < epsilon;
}

template <typename T>
bool epsilon_less(T const & t0, T const & t1, double const epsilon = 0.0000000001) {
    return t0 + epsilon < static_cast<double>(t1) 
        || static_cast<double>(t0) < t1 + epsilon;
}

} // namespace util
