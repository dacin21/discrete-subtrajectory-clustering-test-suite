#pragma once

#include <cstddef>
#include <iostream>
#include <limits>
#include <numeric>
#include <vector>

#include "metric_space.h"

namespace frechet {

template<metric_space space>
class trajectory_collection {

public:
    using metric_space = space;

    using point_t = metric_space::point_t;

    using distance_function_t = metric_space::distance_function_t;
    using distance_t = distance_function_t::distance_t;

    using index_t = std::size_t;
    using subtrajectory_t = std::pair<index_t, index_t>;
    using id_t = unsigned int;

    constexpr static id_t deleted_id = std::numeric_limits<id_t>::max();
    constexpr static size_t no_point = std::numeric_limits<size_t>::max();

    void push_back(const point_t &p, id_t id, [[maybe_unused]] bool allow_deleted = false) {
        assert(allow_deleted || (id != deleted_id));
        vertices.push_back(p);
        trajectory_id.push_back(id);
        original_trajectory_id.push_back(id);
        actual_size++;
        if (id != deleted_id) {
            if (trajectory_size.size() <= id) {
                trajectory_size.resize(id + 1);
                first_point_per_trajectory.resize(id + 1, no_point);
                num_deleted_vertices_per_trajectory.resize(id + 1);
            }
            trajectory_size[id]++;

            if (first_point_per_trajectory[id] == no_point){
                first_point_per_trajectory[id] = trajectory_id.size() - 1;
            }
        }
    }

    id_t get_id_at(index_t index) const {
        return trajectory_id[index];
    }

    bool is_point_deleted(index_t index) const {
        return get_id_at(index) == deleted_id;
    }

    // only works if points are sorted by trajectory id
    index_t get_index_in_trajectory(index_t index) const {
        return index - first_point_per_trajectory[get_id_at(index)];
    }

    point_t& operator[](index_t index) {
        return vertices[index];
    }

    const point_t& operator[](index_t index) const {
        return vertices[index];
    }

    std::size_t get_actual_size() const {
        return actual_size;
    }

    std::size_t total_size() const {
        return vertices.size();
    }

    std::size_t get_trajectory_size(id_t id) const {
        return trajectory_size[id];
    }

    std::size_t num_trajectories() const {
        return trajectory_size.size();
    }

    bool is_sorted_by_trajectory_id() const {
        // we need to ignore deleted points -> cannot just use std::is_sorted(...).
        id_t previous_id = std::numeric_limits<id_t>::min();
        for (const auto id : trajectory_id) {
            if (id == deleted_id) continue;
            if (id < previous_id) {
                return false;
            }
            previous_id = id;
        }
        return true;
    }

    distance_t uncovered_fraction_of_id(id_t id) const {
        return 1.0 - ((double)num_deleted_vertices_per_trajectory[id])/(double)trajectory_size[id];
    }

    distance_t compute_uncovered_fraction_sum() const {
        distance_t result = 0;
        for (size_t id = 0; id < trajectory_size.size(); ++id) {
            result += uncovered_fraction_of_id(id);
        }
        return result;
    }

    // Compute the sum of fractions of uncovered vertices per trajectory given the collection of clusters `all_clusters`.
    //
    // `all_clusters` should be a collection of `subtrajectory_cluster<metric_space>`.
    template<typename cluster_collection>
    distance_t compute_uncovered_fraction_sum_with_point_clusters(const cluster_collection &all_clusters) const {
        return compute_uncovered_fraction_sum_impl(all_clusters, false);
    }
    // Ignores clusters with empty reference subtrajectory.
    template<typename cluster_collection>
    distance_t compute_uncovered_fraction_sum_without_point_clusters(const cluster_collection &all_clusters) const {
        return compute_uncovered_fraction_sum_impl(all_clusters, true);
    }

    index_t get_first_non_deleted_point() const {
        assert(trajectory_id.size() > 0);
        index_t result = 0;
        while (result < trajectory_id.size() && trajectory_id[result] == deleted_id) {
            ++result;
        }
        return result;
    }

    void delete_point(index_t index) {
        assert(index < vertices.size());
        assert(trajectory_id[index] != deleted_id);
        const auto modified_id = trajectory_id[index];
        trajectory_id[index] = deleted_id;
        num_deleted_vertices_per_trajectory[modified_id]++;
        actual_size--;
    }

    void delete_subtrajectory(subtrajectory_t index) {
        assert(index.first < vertices.size());
        assert(index.second < vertices.size());
        for (auto i = index.first; i <= index.second; ++i) {
            delete_point(i);
        }
    }

    void strip_ids() {
        std::fill(trajectory_id.begin(), trajectory_id.end(), 0);
    }

    const std::vector<point_t>& get_point_vector() const {
        return vertices;
    }

private:
    std::vector<point_t> vertices;
    std::vector<id_t> trajectory_id;
    std::vector<id_t> original_trajectory_id;

    std::vector<size_t> trajectory_size;
    std::vector<size_t> num_deleted_vertices_per_trajectory;

    std::vector<size_t> first_point_per_trajectory;

    std::size_t actual_size = 0;

    // `all_clusters` should be a collection of `subtrajectory_cluster<metric_space>`.
    template<typename cluster_collection>
    distance_t compute_uncovered_fraction_sum_impl(const cluster_collection &all_clusters, const bool ignore_point_clusters) const {
        std::vector<size_t> num_covered_of_id;
        num_covered_of_id.resize(trajectory_size.size());
        for (const auto& cluster: all_clusters) {
            const auto ref = cluster.get_reference_subtrajectory();
            if (ignore_point_clusters && ref.first == ref.second) {
                continue;
            }
            for (const auto& subt: cluster.get_subtrajectories()) {
                const auto id = original_trajectory_id[subt.first];
                num_covered_of_id[id] += subt.second - subt.first + 1;
            }
        }
        // Agawal's coverage score
        // distance_t result = 0;
        // for (size_t id = 0; id < trajectory_size.size(); ++id) {
        //     result += 1.0 - (double)num_covered_of_id[id]/(double)trajectory_size[id];
        // }
        // return result;

        // Simplified coverage score
        return (distance_t) (total_size() - std::accumulate(num_covered_of_id.begin(), num_covered_of_id.end(), (size_t)0)) / (distance_t) total_size();
    }
};

} // End of namespace `frechet`
