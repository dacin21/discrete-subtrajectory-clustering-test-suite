#pragma once

#include <array>
#include <cmath>
#include <ostream>

#include <CGAL/Dimension.h>
#include <CGAL/Simple_cartesian.h>

#include "metric_space.h"

namespace frechet {

template<typename real_t, unsigned n>
    requires (n > 0)
struct point_Rn {
    std::array<real_t, n> coordinates;
};

template<typename real_t, unsigned n>
bool operator==(const point_Rn<real_t, n> &a, const point_Rn<real_t, n> &b) {
    bool result = true;
    for (size_t i = 0; i < n; ++i) {
        result &= (a.coordinates[i] == b.coordinates[i]);
    }
    return result;
}

template<typename real_t, unsigned n>
std::ostream& operator<<(std::ostream &str, const point_Rn<real_t, n> &point) {
    str << "(" << point.coordinates[0];
    for (size_t i = 1; i < n; ++i) {
        str << ", " << point.coordinates[i];
    }
    str << ")";
    return str;
}

template<typename real_t, unsigned n>
struct euclidean_distance {
    using point_t = point_Rn<real_t, n>;
    using distance_t = real_t;

    distance_t operator()(const point_t &a, const point_t &b) const {
        distance_t sum_squares = 0;
        for (size_t i = 0; i < n; ++i) {
            sum_squares += (a.coordinates[i] - b.coordinates[i]) * (a.coordinates[i] - b.coordinates[i]);
        }
        return std::sqrt(sum_squares);
    }
};

template<typename real_t, unsigned n>
struct euclidean_space {
    using point_t = point_Rn<real_t, n>;
    using distance_function_t = euclidean_distance<real_t, n>;
};

using test_space_1d = euclidean_space<double, 1>;
using test_space_2d = euclidean_space<double, 2>;

using cgal_test_space_2d = CGAL_metric_space<CGAL::Simple_cartesian<double>, CGAL::Dimension_tag<2>>;

}
