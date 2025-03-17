
#pragma once

#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

#include "kdtree_range_search.h"
#include "metric_space.h"
#include "recycling_object_pool.h"
#include "subtrajectory_cluster.h"
#include "free_space_graph.h"
#include "trajectory.h"
#include "profiling.h"
#include "utility.h"

namespace frechet {

// This class implements the subroutine for finding sets of maximal non-overlapping subtrajectories maximizing "some" objective function.
// The subroutine uses the algorithm by Buchin et al., also implemented in `subtrajectory_bbgll.h`.
// The implementation in this class respects trajectory boundaries and also ignores vertices marked as deleted,
// if multiple trajectories are supported by the trajectory traits `t_traits` (i.e., if `t_traits.supports_multiple_trajectories == true`).
// Otherwise, reference trajectories cross trajectory boundaries and do not ignore deleted vertices.
template<metric_space m_space>
class subtrajectory_clustering_bbgll {

public:
    using space = m_space;
    using free_space_graph_t = sparse_free_space_graph<space>;
    using range_search_t = kd_tree_range_search<space>;

    using point_t = space::point_t;
    using distance_function_t = space::distance_function_t;
    using distance_t = distance_function_t::distance_t;

    using trajectory_t = trajectory_collection<space>;
    using index_t = trajectory_t::index_t;
    using subtrajectory_t = trajectory_t::subtrajectory_t;

    using subtrajectory_cluster_t = subtrajectory_cluster<space>;

private:

    struct length_objective {
        using value_type = long;
        constexpr static value_type lower_bound = -1;

        value_type operator()(const subtrajectory_cluster_t &cluster) const {
            const auto ref = cluster.get_reference_subtrajectory();
            return ref.second - ref.first;
        }
    };

    struct cardinality_objective {
        using value_type = long;
        constexpr static value_type lower_bound = -1;

        value_type operator()(const subtrajectory_cluster_t &cluster) const {
            return cluster.size();
        }
    };

    template<typename objective_function>
    struct best_cluster_store {

    public:
        using value_type = objective_function::value_type;

        best_cluster_store() : best_cluster(&clusters[0]),
                               temp_cluster(&clusters[1]) {}

        bool test_for_improvement() {
            value_type temp_value = objective_function()(*temp_cluster);
            if (temp_value > best_value) {
                std::swap(best_cluster, temp_cluster);
                best_value = temp_value;
                return true;
            }
            return false;
        }

        void reset() {
            clusters[0].clear();
            clusters[1].clear();
            best_value = objective_function::lower_bound;
        }

        subtrajectory_cluster_t& get_best_cluster() {
            return *best_cluster;
        }

        subtrajectory_cluster_t& get_temp_cluster() {
            return *temp_cluster;
        }

    private:
        std::array<subtrajectory_cluster_t, 2> clusters;
        subtrajectory_cluster_t* best_cluster = &clusters[0];
        subtrajectory_cluster_t* temp_cluster = &clusters[1];
        value_type best_value = objective_function::lower_bound;
    };

public:

    subtrajectory_clustering_bbgll(const trajectory_t &trajectory,
                                   range_search_t &range_search) : trajectory(trajectory),
                                                                   search(range_search) {}

    // Find the longest cluster of at least `target_size` trajectories with query distance `distance`.
    // Here, "length" refers to the number of segments in the reference trajectory.
    // That is, reference trajectory `[a,b]` has length `b-a`.
    subtrajectory_cluster_t find_longest_cluster_of_target_size_by_cardinality(const index_t &target_size,
                                                                               const distance_t &distance) {
        return find_longest_cluster_of_target_size<length_objective>(target_size, distance);
    }

    // Find the cluster with the most trajectories of length `target_length`
    // with distance at most `distance` from the reference trajectory.
    // Here, `target_length` refers to the number of segments in the reference trajectory.
    subtrajectory_cluster_t find_max_cardinality_cluster_of_fixed_length(const index_t &target_length,
                                                                         const distance_t &distance) {
        TIME_BEGIN();

        index_t right_column = trajectory.get_first_non_deleted_point();
        index_t left_column = right_column;
        free_space_graph_t free_space{right_column};
        populate_column(free_space, right_column, distance);
        best_cluster_store<cardinality_objective> clusters;
        do {
            advance_with_fixed_length(target_length, distance, free_space, left_column, right_column);

            if (right_column >= trajectory.total_size()) {
                break;
            }

            auto &temp_cluster = clusters.get_temp_cluster();
            temp_cluster.clear();
            free_space.query_subtrajectories_respecting_ids(trajectory, temp_cluster);
            clusters.test_for_improvement();

            left_column++;
            free_space.delete_column();
            
        } while (left_column < trajectory.total_size());

        TIME_END(total_precluster);

        return clusters.get_best_cluster();
    }

    // Find the cluster with the longest reference trajectory among those clusters with maximum cardinality.
    // Here, "longest" refers to the number of segments in the reference trajectory.
    subtrajectory_cluster_t find_max_cardinality_cluster_maximizing_length(const distance_t &distance, index_t min_length = 0) {
        // Find the size of the largest `distance`-neighborhood of any point on the trajectory.
        // This is the maximum cardinality of any cluster.
        index_t M = 0;
        if (min_length == 0) {
            for (index_t idx = 0; idx < trajectory.total_size(); ++idx) {
                if (trajectory.get_id_at(idx) != trajectory_t::deleted_id) {
                    M = std::max(M, search.search(idx, distance).size());
                }
            }
        } else {
            M = find_max_cardinality_cluster_of_fixed_length(min_length, distance).size();
        }

        if (M == 0) {
            assert(min_length > 0);
            return {};
        }

        // Now find the longest cluster among those with cardinality `M`,
        // which is the longest cluster among those with maximum cardinality.
        return find_longest_cluster_of_target_size_by_cardinality(M, distance);
    }

private:
    const trajectory_t &trajectory;
    range_search_t &search;

    void populate_column(free_space_graph_t &free_space,
                         const index_t &column_idx,
                         const distance_t &query_distance) {
        TIME_BEGIN();
        int zeroes = 0;
        for (const auto idx: search.search(column_idx, query_distance)) {
            free_space.add_zero(idx);
            zeroes++;
        }
        assert(zeroes > 0);
        TIME_END(populate_column);
    }

    void advance_with_fixed_length(const index_t &target_length,
                                   const distance_t &distance,
                                   free_space_graph_t &free_space,
                                   index_t &left_column,
                                   index_t &right_column) {
        while (right_column - left_column != target_length) {
            // Move right boundary to be at `target_length` from left boundary
            const bool skipped_deleted_vertex = advance_to_next_right_column(distance, free_space, right_column);
            if (right_column >= trajectory.total_size()) {
                    break;
            }
            // Now ensure that `left_column` and `right_column` are in the same trajectory
            if (skipped_deleted_vertex || trajectory.get_id_at(right_column) != trajectory.get_id_at(left_column)) {
                free_space.advance_left_column_to_right();
                left_column = right_column;
            }
        }
    }

    // Advance `right_column` to the next column that is not deleted.
    // Returns `true` if a deleted column was skipped; `false` otherwise.
    bool advance_to_next_right_column(const distance_t &distance,
                                      free_space_graph_t &free_space,
                                      index_t &right_column) {
        bool skipped_deleted_vertex = false;

        do {
            right_column++;
            if (right_column >= trajectory.total_size()) {
                return skipped_deleted_vertex;
            }
            if (trajectory.get_id_at(right_column) == trajectory_t::deleted_id) {
                skipped_deleted_vertex = true;
            }
        } while(trajectory.get_id_at(right_column) == trajectory_t::deleted_id);
        free_space.new_column(right_column);
        populate_column(free_space, right_column, distance);

    return skipped_deleted_vertex;
}

template<typename length_objective_function>
subtrajectory_cluster_t find_longest_cluster_of_target_size(const index_t &target_size,
                                                            const distance_t &distance) {
    TIME_BEGIN();

    index_t right_column = trajectory.get_first_non_deleted_point();
    index_t left_column = right_column;
    free_space_graph_t free_space{right_column};
    populate_column(free_space, right_column, distance);
    best_cluster_store<length_objective_function> clusters;
    bool skipped_deleted_vertex = false;
    do {
        auto& temp_cluster = clusters.get_temp_cluster();
        temp_cluster.clear();
        free_space.query_subtrajectories_respecting_ids(trajectory, temp_cluster, target_size);
        if (temp_cluster.size() < target_size && left_column != right_column) {
            free_space.delete_column();
            left_column++;
        } else {
            if (temp_cluster.size() >= target_size) {
                clusters.test_for_improvement();
            }
            skipped_deleted_vertex = advance_to_next_right_column(distance, free_space, right_column);
            if (right_column >= trajectory.total_size()) {
                break;
            }
            // If the reference trajectory spans trajectories with different ids,
            // advance the left border of the sweep until the ids are equal.
            if (skipped_deleted_vertex ||
                    trajectory.get_id_at(right_column) != trajectory.get_id_at(left_column)) {
                free_space.advance_left_column_to_right();
                left_column = right_column;
            }
            skipped_deleted_vertex = false;
        }
    } while (left_column < trajectory.total_size());

    TIME_END(total_precluster);

    // TODO: maybe a better way to return the best cluster?
    return clusters.get_best_cluster();
}

};

}
