#pragma once

#include <cmath>
#include <iostream>

#include "frechet_distance.h"
#include "free_space_graph.h"
#include "kdtree_range_search.h"
#include "metric_space.h"
#include "profiling.h"
#include "subtrajectory_cluster.h"
#include "subtrajectory_routine_rightstep.h"
#include "trajectory.h"

namespace frechet {

namespace internal {

template<metric_space m_space>
class curve_simplification{

public:
    using space = m_space;
    using range_search_t = kd_tree_range_search<space>;

    using trajectory_t = trajectory_collection<space>;
    using index_t = trajectory_t::index_t;

    using distance_function_t = space::distance_function_t;
    using distance_t = distance_function_t::distance_t;

    using rightstep_algo_t = subtrajectory_clustering_rightstep<space>;
    using cluster_summary_t  = rightstep_algo_t::cluster_summary_t;

    using weights_t = std::vector<index_t>;

    curve_simplification(const trajectory_t &trajectory, const distance_t &sq_distance, const double simplification_factor) :
        original_trajectory(trajectory), original_distance(sq_distance), simplification_factor(simplification_factor),
        simplified_trajectory(), original_leftmost_index(), point_weights_(), range_search_(compute_simplification()) {}

    std::optional<cluster_summary_t> unsimplify(std::optional<cluster_summary_t> cluster) const {
        if (cluster) {
            std::cerr << "    Unsimplify [" << cluster->left_column << "," << cluster->right_column << "] to ";
            cluster->left_column = original_leftmost_index[cluster->left_column];
            cluster->right_column = original_leftmost_index[cluster->right_column] + point_weights_[cluster->right_column] - 1;
            std::cerr << "[" << cluster->left_column << "," << cluster->right_column << "]\n";
        }
        return cluster;
    }

    const weights_t& point_weights() const {
        return point_weights_;
    }
    const trajectory_t& trajectory() const {
        return simplified_trajectory;
    }
    range_search_t& range_search() {
        return range_search_;
    }
    double sq_distance() const {
        // Reconstructing the actual cluster on the original curve only works
        // if we reduce the sq_distance on the simplified curve.
        const auto remaining_factor = 1.0 - simplification_factor;
        return original_distance * remaining_factor * remaining_factor;
    }

private:
    const trajectory_t& compute_simplification(){
        size_t l = 0;
        for(size_t i=0; i <= original_trajectory.total_size(); ++i){
            if (i == original_trajectory.total_size() || !are_points_compatible(l, i)){
                // Compress the points in [l, i) to a single one.
                // Keep deleted points to remember that the trajectory was split into multiple pieces.
                simplified_trajectory.push_back(original_trajectory[l], original_trajectory.get_id_at(l), true);
                point_weights_.push_back(i - l);
                original_leftmost_index.push_back(l);

                l = i;
            }
        }
        return simplified_trajectory;
    }

    bool are_points_compatible(index_t i, index_t j){
        if (original_trajectory.get_id_at(i) != original_trajectory.get_id_at(j)) {
            return false;
        }
        auto const sq_distance = distance_function_t{}(original_trajectory[i], original_trajectory[j]);
        return sq_distance <= simplification_sq_distance();
    }
    double simplification_sq_distance() const {
        // If we simplify both curves by `distance * factor/2`, the maximum total error is `distance * factor`.
        const auto factor_halved = simplification_factor / 2;
        return original_distance * factor_halved * factor_halved;
    }

    const trajectory_t &original_trajectory;
    const distance_t original_distance;
    const double simplification_factor;

    trajectory_t simplified_trajectory;
    weights_t original_leftmost_index, point_weights_;

    range_search_t range_search_;
};

} // namespace internal

} // namespace frechet
