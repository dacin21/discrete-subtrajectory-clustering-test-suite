#pragma once

#include <boost/property_map/property_map.hpp>
#include <chrono>
#include <iostream>
#include <optional>
#include <type_traits>

#include <ankerl/unordered_dense.h>

#include "metric_space.h"
#include "trajectory.h"

namespace frechet {

// A boost property map interface for `trajectory_collection<m_space>`.
template<metric_space m_space>
class trajectory_property_map_adapter {

public:
    using trajectory_t = trajectory_collection<m_space>;

    using value_type = m_space::point_t;
    using reference = const value_type&;
    using key_type = trajectory_t::index_t;
    using category = boost::lvalue_property_map_tag;

    trajectory_property_map_adapter(const trajectory_t &data) : data(data) {}

    reference operator[](const key_type &key) const {
        return data[key];
    }

private:
    const trajectory_t &data;

};

template<typename T>
const T::value_type& get(const T &map, const typename T::key_type &key) {
    return map[key];
}


// Additional concepts

template<typename T>
concept has_CGAL_kernel = requires {
    typename T::kernel;
};

// Some more stuff

namespace internal {

template<typename pair_type>
struct pair_hash {
    using is_avalanching = void;

    [[nodiscard]] uint64_t operator()(const pair_type &s) const {
        using namespace ankerl::unordered_dense::detail::wyhash;
        const auto hash_a = hash(s.first);
        // TODO: might want to consider different ways to combine 
        return std::rotl(hash_a, 1) + hash_a + hash(s.second);
    }
};

template<typename T, typename Compare>
bool optimize(std::optional<T> &cur, T const&cand){
    if(!cur || Compare{}(cand, *cur)){
        cur = cand;
        return true;
    }
    return false;
}
template<typename T>
bool maximize(std::optional<T> &cur, T const&cand){
    return optimize<T, std::greater<>>(cur, cand);
}
template<typename T>
bool minimize(std::optional<T> &cur, T const&cand){
    return optimize<T, std::less<>>(cur, cand);
}

struct Timer{
    using clock_t = std::chrono::steady_clock;
    using time_point_t = decltype(clock_t::now());

    Timer() : start(clock_t::now()) {}

    double seconds() const {
        const auto now = clock_t::now();
        const auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count();
        return nanoseconds * 1e-9;
    }

    time_point_t start;
};


template<typename T>
class prefix_sum{
public:
    using value_type = T;
    using storage_t = std::vector<T>;
    using index_t = std::size_t;

    prefix_sum(const storage_t &values) : data(values.size() + 1) {
        std::partial_sum(values.begin(), values.end(), data.begin() + 1);
    }

    // sum over [l, r)
    value_type sum(index_t l, index_t r) const {
        return data[r] - data[l];
    }

private:
    storage_t data;
};

} // namespace internal

} // namespace frechet

