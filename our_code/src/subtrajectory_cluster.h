#pragma once

#include <CGAL/Search_traits_2.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <algorithm>
#include <limits>
#include <set>
#include <type_traits>

#include "trajectory.h"
#include "utility.h"

namespace frechet {

template<metric_space m_space>
class subtrajectory_cluster {

public:
    using space = m_space;

    using trajectory_t = trajectory_collection<space>;

    using subtrajectory_t = trajectory_t::subtrajectory_t;
    using reference = subtrajectory_t&;
    using const_reference = const subtrajectory_t&;

    using distance_function_t = space::distance_function_t;
    using distance_t = distance_function_t::distance_t;

    using cluster_t = std::vector<subtrajectory_t>;
    using size_t = cluster_t::size_type;

private:
    struct subtrajectory_compare {

        bool operator()(const subtrajectory_t &a, const subtrajectory_t &b) const {
            return a.second < b.first;
        }

    };

public:
    subtrajectory_cluster() {}

    // Create a new subtrajectory cluster with the given reference subtrajectory.
    subtrajectory_cluster(const_reference reference_subtrajectory) : reference_subtrajectory(reference_subtrajectory) {}

    void set_reference_trajectory(const_reference ref) {
        reference_subtrajectory = ref;
    }

    void push_back(const_reference subtrajectory) {
        subtrajectories.push_back(subtrajectory);
    }

    void clear() {
        subtrajectories.clear();
    }

    bool empty() const {
        return subtrajectories.empty();
    }

    size_t size() const {
        return subtrajectories.size();
    }

    reference back() {
        return subtrajectories.back();
    }

    const_reference back() const {
        return subtrajectories.back();
    }

    reference operator[](size_t index) {
        return subtrajectories[index];
    }

    const_reference operator[](size_t index) const {
        return subtrajectories[index];
    }

    bool contains(subtrajectory_t subt) const {
        for (const auto &s: subtrajectories) {
            if (s == subt) {
                return true;
            }
        }
        return false;
    }

    const_reference get_reference_subtrajectory() const {
        return reference_subtrajectory;
    }

    const cluster_t& get_subtrajectories() const {
        return subtrajectories;
    }

    template<typename Fun>
    void erase_subtrajectories_if(const Fun &fun){
        subtrajectories.erase(std::remove_if(subtrajectories.begin(), subtrajectories.end(), fun), subtrajectories.end());
    }

    //
    // Cluster scoring
    //
    
    size_t number_of_vertices() const {
        size_t result = 0;
        for (auto &subt: subtrajectories) {
            // index of last vertex - index of first vertex + 1
            // the `+ 1` is necessary, since the subtrajectory includes its last vertex
            result += subt.second - subt.first + 1;
        }
        return result;
    }

private:
    cluster_t subtrajectories;
    subtrajectory_t reference_subtrajectory;

};

}
