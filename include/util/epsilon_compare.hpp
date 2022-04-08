#pragma once 

#include <cmath>

namespace util {

template <typename T>
bool epsilon_eq(T const & t0, T const & t1, double const epsilon = 0.0000000001) {
    return std::abs(t0 - t1) < epsilon;
}

template <typename T>
bool epsilon_less(T const & t0, T const & t1, double const epsilon = 0.0000000001) {
    return !epsilon_eq(t0, t1, epsilon)
        && t0 < t1;
}

template <typename T>
bool epsilon_less_or_eq(T const & t0, T const & t1, double const epsilon = 0.0000000001) {
    return epsilon_eq(t0, t1, epsilon)
        || t0 < t1;
}

template <typename T>
bool epsilon_greater(T const & t0, T const & t1, double const epsilon = 0.0000000001) {
    return !epsilon_eq(t0, t1, epsilon)
        && t0 > t1;
}

template <typename T>
bool epsilon_greater_or_eq(T const & t0, T const & t1, double const epsilon = 0.0000000001) {
    return epsilon_eq(t0, t1, epsilon)
        || t0 > t1;
}

} // namespace util
