#pragma once

#include <iterator>
#include <iostream>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

#include "kdtree_range_search.h"
#include "metric_space.h"
#include "recycling_object_pool.h"
#include "subtrajectory_cluster.h"
#include "free_space_graph_incremental.h"
#include "free_space_graph.h"
#include "trajectory.h"
#include "profiling.h"
#include "utility.h"

namespace frechet {


struct rightstep_config {
    // Only consider O(n log n) candidate pathlets instead of O(n^2)
    // This yields a 2-approximation.
    bool tree_intervals_only = false;
    // When considering fechet distance d, merge consecutive points at distance <=factor*d.
    double curve_simplification_factor = 0.0;
    // Prefer creating many small subtrajectories over one big subtrajectory.
    // This is good for k-center, but bad for k-means.
    bool prefer_small_subtrajectories = true;

    // Cost of adding one additional pathlet to a cluster.
    // Set to 0 for k-means and to c3 * distance / c1 for k-centers.
    double cost_per_pathlet = 0.0;
};

// A subroutine for finding all maximal clusters. Here maximal means that the cluster's
// reference subtrajectory cannot be extended while covering the same number of subtrajecries.
//
// The implementation respects trajectory boundaries and deleted vertices.
// This requires that the points in trajectory are sorted by trajectory id.
template<metric_space m_space>
class subtrajectory_clustering_rightstep {

public:
    using space = m_space;
    using free_space_graph_incremental_t = sparse_free_space_graph_incremental<space>;
    using free_space_graph_t = sparse_free_space_graph<space>;
    using range_search_t = kd_tree_range_search<space>;

    using point_t = space::point_t;
    using distance_function_t = space::distance_function_t;
    using distance_t = distance_function_t::distance_t;

    using trajectory_t = trajectory_collection<space>;
    using index_t = trajectory_t::index_t;
    using id_t = trajectory_t::id_t;
    using subtrajectory_t = trajectory_t::subtrajectory_t;

    using subtrajectory_cluster_t = subtrajectory_cluster<space>;
    using cluster_summary_t  = free_space_graph_incremental_t::cluster_quality;

    using weights_t = std::vector<index_t>;

public:
    subtrajectory_clustering_rightstep(const trajectory_t &trajectory,
                                       const weights_t &point_weights,
                                       range_search_t &range_search,
                                       const rightstep_config &config) :
            trajectory(trajectory), point_weights(reindex_with_trajectory_id(point_weights, trajectory)), search(range_search), config(config) {
        assert(trajectory.is_sorted_by_trajectory_id()); // needed to make free_space_graph_incremental_t respect trajectory ids
    }

    // ignores point_weights
    // subtrajectory_cluster_t find_cluster_covering_most_points(const distance_t &distance_max){
    //     const auto best_cluster = find_best_cluster(distance_max, [&](const cluster_summary_t &cluster){
    //         return std::make_pair(cluster.covered_points_count, cluster.right_column - cluster.left_column);
    //     });
    //     return cluster_from_candidate(distance_max, best_cluster);
    // }
    //
    // std::optional<cluster_summary_t> find_highest_weight_cluster_candidate(const distance_t &distance_max){
    //     const auto best_cluster = find_best_cluster(distance_max, [&](cluster_summary_t const& cluster){
    //         return std::make_pair(cluster.covered_points_count, cluster.right_column - cluster.left_column);
    //     });
    //     return best_cluster;
    // }

    subtrajectory_cluster_t find_best_cluster(const distance_t &distance_max){
        return cluster_from_candidate(distance_max, find_best_cluster_candidate(distance_max));
    }

    std::optional<cluster_summary_t> find_best_cluster_candidate(const distance_t &distance_max){
        if (config.cost_per_pathlet > 0.0){
            return find_best_cluster(distance_max, [&](cluster_summary_t const& cluster){
                return std::make_pair(cluster.coverage_per_cost, cluster.right_column - cluster.left_column);
            });
        } else {
            // cost_per_coverage will not be computed, so use covered_points_count.
            return find_best_cluster(distance_max, [&](cluster_summary_t const& cluster){
                return std::make_pair(cluster.covered_points_count, cluster.right_column - cluster.left_column);
            });
        }
    }

    // Cluster from given left_column, right_column
    subtrajectory_cluster_t cluster_from_candidate(const distance_t &distance_max, const std::optional<cluster_summary_t> &cluster_candidate){
        if (!cluster_candidate) {
            return subtrajectory_cluster_t{}; // return an empty cluster
        }
        return to_subtrajectory_cluster(*cluster_candidate, distance_max);
    }

private:
    const trajectory_t &trajectory;
    internal::prefix_sum<index_t> point_weights;
    range_search_t &search;
    rightstep_config config;

    // Transform: `old_weights[i] = new_weights[i + trajectory_id[i]]`.
    // The later is needed by the incremental_free_space_graph.
    static weights_t reindex_with_trajectory_id(const weights_t &old_weights, const trajectory_t &trajectory) {
        weights_t new_weights(trajectory.total_size() + trajectory.num_trajectories());
        for (index_t i = 0; i < trajectory.total_size(); ++i) {
            if (trajectory.is_point_deleted(i)) continue;
            new_weights[i + trajectory.get_id_at(i)] = old_weights[i];
        }
        return new_weights;
    }

    // Utility function to do the backwards sweep when considering tree clusters
    // Does nothing if `reverse` is false.
    // Otherwise, the important properties are
    //  - adjust_index(i + 1) = adjust_index(i) - 1
    //  - adjust_index(adjust_index(i)) = i
    static index_t adjust_index(index_t index, bool reverse){
        if (!reverse) return index;
        // we subtract 1 to avoid issues with numericic_limits<index_t>::max()
        return ~index - 1;
    }
    // spacing out row indices ensures that subtrajectories stay within a trajectory.
    index_t spaced_index(index_t index){
        return index + trajectory.get_id_at(index);
    }

    // Transform: Subtrajectory (l + trajectory_id[l], r + trajectory_id[r]) to (l, r).
    // Assumes trajectories are non-overlapping and sorted in decreasing order
    void undo_index_spacing(subtrajectory_cluster_t &cluster){
        index_t orig_index = trajectory.total_size() - 1;
        // Only works if index is non-increasing in subsequent calls.
        auto unspace_index = [&](index_t& index){
            while (trajectory.is_point_deleted(orig_index) || spaced_index(orig_index) != index) {
                assert(orig_index >= 0);
                --orig_index;
            }
            index = orig_index;
        };
        for (index_t i = 0; i < cluster.size(); ++i){
            unspace_index(cluster[i].second);
            unspace_index(cluster[i].first);
        }
    }


    // Among all possible clusters, find the one maximizing a given score function.
    // score_function should take a cluster_summary_t and return a comparable type.
    // 
    // Setting tree_intervals_only to true makes this much faster, but
    // the returned interval is only guaranteed to be a 2-approximation.
    template<typename Fun>
    std::optional<cluster_summary_t> find_best_cluster(const distance_t &distance_max, const Fun &score_function){
        std::cout << "    Finding best cluster at distance " << std::sqrt(distance_max) << "\n";
        std::optional<cluster_summary_t> best_cluster;

        auto process_candidate_cluster = [&score_function, &best_cluster](const cluster_summary_t &cluster){
            // std::cout << "Candidate cluster " << "[" << cluster.left_column << "," << cluster.right_column << "] " << cluster.covered_points_count << " | ";
            if(!best_cluster || score_function(*best_cluster) < score_function(cluster)){
                best_cluster = cluster;
            }
        };

        if (config.tree_intervals_only) {
            foreach_tree_cluster(distance_max, process_candidate_cluster);
        } else {
            foreach_possible_cluster(distance_max, process_candidate_cluster);
        }

        std::cout << "    Found ";
        if(best_cluster) std::cout << "[" << best_cluster->left_column << "," << best_cluster->right_column << "] " << best_cluster->covered_points_count;
        std::cout << "\n";

        return best_cluster;
    }

    template<typename Callback>
    void foreach_possible_cluster(const distance_t &distance_max, const Callback &callback){
        free_space_graph_incremental_t free_space(0, config.prefer_small_subtrajectories, config.cost_per_pathlet);
        for (index_t left_column = trajectory.get_first_non_deleted_point(); left_column < trajectory.total_size(); ++left_column) {
            do_column_sweep(distance_max, callback, free_space, left_column, trajectory.total_size(), false);
        }
    }
    template<typename Callback>
    void foreach_tree_cluster(const distance_t &distance_max, const Callback &callback){
        free_space_graph_incremental_t free_space(0, config.prefer_small_subtrajectories, config.cost_per_pathlet);
        // bit trick for largest power of 2 that divides column
        auto sweep_distance = [](const index_t &column) { return column & -column; };
        for (index_t column = trajectory.get_first_non_deleted_point(); column < trajectory.total_size(); ++column){
            if (trajectory.get_id_at(column) == trajectory_t::deleted_id) {
                continue;
            }
            // The choice of column_end might seem arbitrary, but it guarantees a
            // 2 approximation in O(n log n) steps (instead of O(n^2)).
            do_column_sweep(distance_max, callback, free_space, column, std::min(column + sweep_distance(column), trajectory.total_size()), false);
            do_column_sweep(distance_max, callback, free_space, column, column - sweep_distance(column+1), true);
        }
        // also try each whole trajectory. This helps in cases where a trajectory covers only itself.
        index_t left_column = 0;
        for(index_t right_column = 0; right_column <= trajectory.total_size(); ++right_column){
            if (right_column == trajectory.total_size() || trajectory.get_id_at(left_column) != trajectory.get_id_at(right_column)){
                if (trajectory.get_id_at(left_column) != trajectory_t::deleted_id) {
                    do_column_sweep(distance_max, callback, free_space, left_column, right_column - 1, false);
                }
                left_column = right_column;
            }
        }
    }
    template<typename Callback>
    void do_column_sweep(const distance_t &distance_max, const Callback &callback, free_space_graph_incremental_t &free_space,
                                       const index_t column_begin, const index_t column_end, bool reverse) {
        const index_t column_step = (reverse ? (index_t)-1 : (index_t)1);

        const auto reference_trajectory_id = trajectory.get_id_at(column_begin);
        if (reference_trajectory_id == trajectory_t::deleted_id) return;

        free_space.reset(column_begin);
        for(index_t column = column_begin; column != column_end && trajectory.get_id_at(column) == reference_trajectory_id; column += column_step) {
            populate_column(free_space, distance_max, column, reverse);

            auto cluster_summary = free_space.query_cluster_candidate([&](index_t l, index_t r){
                if (reverse) {
                    // turn [l, r) into (l-1, r-1] 
                    --l;
                    --r;
                    std::swap(l, r);
                }
                return point_weights.sum(adjust_index(l, reverse), adjust_index(r, reverse));
            }, adjust_index(spaced_index(column_begin), reverse), adjust_index(spaced_index(column), reverse));

            assert(cluster_summary.left_column == column_begin);
            assert((cluster_summary.right_column - cluster_summary.left_column) * column_step == column - column_begin);
            if (cluster_summary.subtrajectories_count > 0) {
                if (reverse) {
                    // free_space always increments columns by 1, hence the columns in the returned cluster summary are wrong.
                    cluster_summary.left_column = column;
                    cluster_summary.right_column = column_begin;
                }
                callback(cluster_summary);
            }

            free_space.new_column();
        }
    }

    void populate_column(free_space_graph_incremental_t &free_space, const distance_t &distance_max, const index_t &column, bool reverse_column = false){
        TIME_BEGIN();
        auto indices = search.search(column, distance_max);
        if (reverse_column) {
            reverse(indices.begin(), indices.end());
        }
        for(const auto idx : indices){
            // In rare cases, points fail to be deleted from the KD-tree.
            if (trajectory.is_point_deleted(idx)) continue;

            const auto idy = adjust_index(spaced_index(idx), reverse_column);
            free_space.add_zero(idy);
        }
        TIME_END(populate_column);
    }

    subtrajectory_cluster_t to_subtrajectory_cluster(const cluster_summary_t &cluster_summary, const distance_t &distance_max){
        free_space_graph_incremental_t free_space(cluster_summary.left_column, config.prefer_small_subtrajectories, config.cost_per_pathlet);
        for(index_t column = cluster_summary.left_column; column <= cluster_summary.right_column; ++column){
            if(column != cluster_summary.left_column) free_space.new_column();
            for(const auto idx : search.search(column, distance_max)){
                // In rare cases, points fail to be deleted from the KD-tree.
                if (trajectory.is_point_deleted(idx)) continue;
                free_space.add_zero(spaced_index(idx));
            }
        }
        subtrajectory_cluster_t cluster;
        free_space.query_subtrajectories(cluster, spaced_index(cluster_summary.left_column), spaced_index(cluster_summary.right_column));

        cluster.set_reference_trajectory({cluster_summary.left_column, cluster_summary.right_column});
        undo_index_spacing(cluster);

        std::cout << "    Actual cluster has " << cluster.number_of_vertices() << " points on " << cluster.size() << " trajectories\n";
        return cluster;
    }
};

} // namespace frechet

