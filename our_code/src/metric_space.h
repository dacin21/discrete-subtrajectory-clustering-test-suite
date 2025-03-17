#pragma once

#include <type_traits>

#include <CGAL/Dimension.h>
#include <CGAL/Search_traits_2.h>
#include <CGAL/Search_traits_3.h>
#include <CGAL/Search_traits_d.h>
#include <CGAL/Simple_cartesian.h>

namespace frechet {

// General concept to describe a metric space

template<typename T>
concept distance_function = requires {
    // `T` defines a type for points on which the distance is defined.
    typename T::point_t;
    // `T` defines a type that represents the distance
    typename T::distance_t;
    // An instance of `T` can be called to compute a distance.
    // The return type is of type `T::distance`
    requires std::is_invocable_r_v<typename T::distance_t,
                                   T,
                                   const typename T::point_t&,
                                   const typename T::point_t&>;
};

template<typename T>
concept metric_space = requires {
    // `T` defines a point type.
    typename T::point_t;
    // `T` defines a distance function.
    typename T::distance_function_t;
    // `T` and its distance function have the same underlying point type.
    requires std::is_same_v<typename T::point_t,
                            typename T::distance_function_t::point_t>;
    // `T`'s distance function is a distance function.
    requires distance_function<typename T::distance_function_t>;
};


// Metric spaces backed by CGAL-Kernels

template<typename T>
concept CGAL_metric_space_concept  = requires {
    requires metric_space<T>;

    // The CGAL Kernel this metric space wraps around
    typename T::kernel;
    // The kernels dimension
    typename T::dimension;
    requires (T::dimension::value == CGAL::Ambient_dimension<typename T::point_t,
                                                             typename T::kernel>::value);
    // Appropriate search traits for the kernel and dimension 
    typename T::search_traits;
    requires (T::search_traits::Dimension::value == T::dimension::value);
};


// Squared euclidean distance provided by the CGAL kernel `K`.
template<typename K, typename dimension_tag = typename K::Dimension>
class CGAL_squared_distance_function {

public:
    using point_t = K::Point_d;
    using distance_t = K::FT;

    distance_t operator()(const point_t &a, const point_t &b) const {
        return K::Squared_distance_d::operator()(a, b);
    }

};

// Spezialization for `Simple_cartesian` with dimesion 2
template<>
class CGAL_squared_distance_function<CGAL::Simple_cartesian<double>, CGAL::Dimension_tag<2>> {

public:
    using kernel = CGAL::Simple_cartesian<double>;
    using point_t = kernel::Point_2;
    using distance_t = double;

    distance_t operator()(const point_t &a, const point_t &b) const {
        return kernel::Compute_squared_distance_2{}(a, b);
    }

};

// Spezialization for `Simple_cartesian` with dimension 3
template<>
class CGAL_squared_distance_function<CGAL::Simple_cartesian<double>, CGAL::Dimension_tag<3>> {

public:
    using kernel = CGAL::Simple_cartesian<double>;
    using point_t = kernel::Point_3;
    using distance_t = double;

    distance_t operator()(const point_t &a, const point_t &b) const {
        return kernel::Compute_squared_distance_3{}(a, b);
    }

};

// A `metric_space` wrapping around a CGAL kernel `K` with dimension given by `dimension_tag`.
// Expects `dimension_tag` to be a specialization of `CGAL::Dimension_tag`.
// TODO: this assumes that `K` models `Kernel_d`. Might want to generalize further. Specializations for `Simple_cartesian` are below.
template<typename K, typename dimension_tag = typename K::Dimension>
class CGAL_metric_space {

public:
    using kernel = K;
    using dimension = dimension_tag;
    using search_traits = CGAL::Search_traits_d<kernel>;

    using point_t = K::Point_d;
    using distance_function_t = CGAL_squared_distance_function<K>;
};

template<>
class CGAL_metric_space<CGAL::Simple_cartesian<double>, CGAL::Dimension_tag<2>> {
public:
    using kernel = CGAL::Simple_cartesian<double>;
    using dimension = CGAL::Dimension_tag<2>;
    using search_traits = CGAL::Search_traits_2<kernel>;

    using point_t = kernel::Point_2;
    using distance_function_t = CGAL_squared_distance_function<kernel, dimension>;

};

template<>
class CGAL_metric_space<CGAL::Simple_cartesian<double>, CGAL::Dimension_tag<3>> {
public:
    using kernel = CGAL::Simple_cartesian<double>;
    using dimension = CGAL::Dimension_tag<3>;
    using search_traits = CGAL::Search_traits_3<kernel>;

    using point_t = kernel::Point_3;
    using distance_function_t = CGAL_squared_distance_function<kernel, dimension>;

};


}
