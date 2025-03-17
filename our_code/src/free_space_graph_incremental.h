#pragma once

#include <array>
#include <cassert>
#include <limits>
#include <list>
#include <ostream>
#include <utility>
#include <vector>

#include "ankerl/unordered_dense.h"

#include "profiling.h"
#include "recycling_object_pool.h"
#include "subtrajectory_cluster.h"
#include "utility.h"
#include "trajectory.h"

namespace frechet {

// An faster version of the `sparse_free_space_graph` with restricted features.
//  - only supports insertions. Cannot change the left column.
//  - only counts the subtrajectories covered by a pathlet. Cannot reconstruct the actual subtrajectories.
//  - doesn't handle deleted points. (The caller can just skip inserting the corresponding zeros.)
//  - doesn't handle trajectory ids. (The caller can just add a gap between rows of different trajectories.)
// 
// Points are allowed to have weights, to deal with compressed points arriving from curve simplification.
template<metric_space m_space>
class sparse_free_space_graph_incremental {

public:
    using space = m_space;
    using point_t = m_space::point_t;
    using distance_t = space::distance_function_t::distance_t;
    using trajectory_t = trajectory_collection<space>;
    using index_t = trajectory_t::index_t;
    using subtrajectory_length_t = std::size_t;
    using subtrajectory_cluster_t = subtrajectory_cluster<space>;

public:
    using row_index_t = index_t;

    struct cluster_quality{
        cluster_quality(const index_t &subtrajectories_count, const subtrajectory_length_t &covered_points_count, const distance_t& coverage_per_cost,
                        const index_t &left_column, const index_t &right_column) :
            subtrajectories_count(subtrajectories_count), covered_points_count(covered_points_count), coverage_per_cost(coverage_per_cost),
            left_column(left_column), right_column(right_column) {}

        index_t subtrajectories_count;
        subtrajectory_length_t covered_points_count;
        distance_t coverage_per_cost;

        index_t left_column, right_column;
    };

private:
    struct vertex {
        vertex(const row_index_t &row_index, const row_index_t &highest_left_row, const row_index_t &highest_left_row_bottom)
            : row_index(row_index), highest_left_row(highest_left_row), highest_left_row_bottom(highest_left_row_bottom) {}

        row_index_t row_index;

        row_index_t highest_left_row; // always exists. A vertex that cannot reach the leftmost column will not be stored.
        row_index_t highest_left_row_bottom; // from highest_left_row, we could take steps down until we reach this row.

        std::pair<row_index_t, row_index_t> highest_left_key() const {
            // we want to maximize highest_left_row, and break ties by minimizing highest_left_row_bottom
            return {highest_left_row, ~highest_left_row_bottom};
        }
    };

public:
    sparse_free_space_graph_incremental(const index_t &init_column, bool prefer_small_subtrajectories, distance_t cost_per_pathlet = 0) :
        prefer_small_subtrajectories(prefer_small_subtrajectories), cost_per_pathlet(cost_per_pathlet) {
        reset(init_column);
    }

    void reset(index_t new_left_column) {
        left_column = new_left_column;
        right_column = left_column;

        right_vertices.clear();
        previous_vertices.clear();
        left_candidate = previous_vertices.end();
    }
    void new_column(index_t new_right_column) {
        if (new_right_column != right_column + 1){
            reset(new_right_column);
        } else {
            new_column();
        }
    }
    void add_zero(row_index_t row_index) {
        using index_pair_t = std::pair<row_index_t, row_index_t>;
        INCREMENT_COUNT(zeroes);

        assert(right_vertices.empty() || (row_index > right_vertices.back().row_index));

        // the very first column
        if (left_column == right_column) {
            const row_index_t row_bottom = (!right_vertices.empty() && right_vertices.back().row_index == row_index-1) ?
                                                 right_vertices.back().highest_left_row_bottom : row_index;
            right_vertices.emplace_back(row_index, row_index, row_bottom);
            return;
        }
        auto mize = prefer_small_subtrajectories ? internal::maximize<index_pair_t> : internal::minimize<index_pair_t>;

        std::optional<index_pair_t> best_left_row;
        if (!right_vertices.empty()) {
            auto &down = right_vertices.back();
            if (down.row_index == row_index-1) {
                mize(best_left_row, down.highest_left_key());
            }
        }
        // TODO: potential speed up: add a sentinel with infinite row_index and skip the bounds checks.
        while (left_candidate != previous_vertices.end() && left_candidate->row_index+1 < row_index) {
            ++left_candidate;
        }
        if (left_candidate != previous_vertices.end() && left_candidate->row_index+1 == row_index) {
            mize(best_left_row, left_candidate->highest_left_key());
            ++left_candidate;
        }
        if (left_candidate != previous_vertices.end() && left_candidate->row_index == row_index) {
            mize(best_left_row, left_candidate->highest_left_key());
            // don't advance left_candidate, we might need it for the next row.
        }

        // only store zeros that admit a subtrajectory.
        if(!best_left_row) return; 
        right_vertices.emplace_back(row_index, best_left_row->first, ~best_left_row->second);
    }
    void new_column() {
        ++right_column;

        previous_vertices.clear();
        previous_vertices.swap(right_vertices);
        left_candidate = previous_vertices.begin();
    }

    // total_weight(l, r) should return the total weight of all points `i` with
    // `i + trajectory_id[i]` in [l, r). Note the non-trivial indexing scheme!
    template<typename get_weights_t>
    cluster_quality query_cluster_candidate(const get_weights_t& total_weight, const index_t &first_reference_row, const index_t &last_reference_row) const {
        index_t subtrajectory_count = 0;
        subtrajectory_length_t covered_points_weight = 0;
        pathlet_weights.clear();

        do_query([&](row_index_t first_row, row_index_t last_row) {
            ++subtrajectory_count;
            // first_row >= last_row
            const auto delta_weight = total_weight(last_row, first_row + 1);
            covered_points_weight += delta_weight;
            pathlet_weights.push_back(delta_weight);
        }, [&](row_index_t old_last_row, row_index_t new_last_row) {
            // old_last_row < new_last_row
            const auto delta_weight = total_weight(old_last_row, new_last_row);
            covered_points_weight -= delta_weight;
            pathlet_weights.back() -= delta_weight;
        }, first_reference_row, last_reference_row);

        return cluster_quality(subtrajectory_count, covered_points_weight, compute_coverage_per_cost(), left_column, right_column);
    }
    void query_subtrajectories(subtrajectory_cluster_t &output_trajectories, const index_t &first_reference_row, const index_t &last_reference_row) const {
        do_query([&](row_index_t first_row, row_index_t last_row) {
            // first_row >= last_row
            output_trajectories.push_back({last_row, first_row});
        }, [&]([[maybe_unused]] row_index_t old_last_row, row_index_t new_last_row) {
            assert(output_trajectories.back().first == old_last_row);
            output_trajectories.back().first = new_last_row;
        }, first_reference_row, last_reference_row);
        output_trajectories.set_reference_trajectory({first_reference_row, last_reference_row});
    }

private:
    using column_t = std::vector<vertex>;
    using column_iterator_t = typename column_t::iterator;

    distance_t compute_coverage_per_cost() const {
        if (cost_per_pathlet == 0) return -1;

        auto feasible = [&](distance_t gamma) {
            distance_t total = 0;
            for (const auto coverage : pathlet_weights) {
                const auto delta = (distance_t) coverage  - gamma * cost_per_pathlet;
                if (delta > 0) total += delta;
            }
            return total > gamma;
        };

        distance_t l = 0.0, r = 1.0;
        while (feasible(r)) r *= 2;
        for(int it = 0; it < 50; ++it) {
            const auto m = l + (r - l) / 2;
            if (feasible(m)) {
                l = m;
            } else {
                r = m;
            }
        }
        return l;
    };

    template<typename callback_1, typename callback_2>
    void do_query(const callback_1 &new_subtrajectory,
                             const callback_2 &shorten_previous_subtrajectory,
                             index_t first_reference_row,
                             index_t last_reference_row) const {
        // happens when the rightstep algorithm sweeps to the left
        if (first_reference_row < last_reference_row) {
            std::swap(first_reference_row, last_reference_row);
        }

        index_t start_vertex = right_vertices.size() - 1;
        row_index_t previous_row_extension = std::numeric_limits<row_index_t>::max();
        while (start_vertex + 1) {
            row_index_t first_used_row = right_vertices[start_vertex].row_index;
            row_index_t last_used_row = right_vertices[start_vertex].highest_left_row;
            row_index_t row_extension = right_vertices[start_vertex].highest_left_row_bottom;

            // overlap with the reference trajectory -> use the reference trajectory instead.
            if (first_used_row >= last_reference_row && last_used_row <= first_reference_row) {
                first_used_row = first_reference_row;
                last_used_row = row_extension = last_reference_row;
            }

            // we were too optimistic in the previous interation, let's subtract the overlap.
            if(previous_row_extension <= first_used_row) {
                shorten_previous_subtrajectory(previous_row_extension, first_used_row + 1);
            }
            previous_row_extension = row_extension;

            // let's be optimistic and assume we can go all the way to `row_extension`.
            new_subtrajectory(first_used_row, row_extension);

            while (start_vertex+1 && right_vertices[start_vertex].row_index >= last_used_row) {
                --start_vertex;
            }
        }
    }

    const bool prefer_small_subtrajectories;
    const distance_t cost_per_pathlet;
    index_t left_column = 0;
    index_t right_column = 0;

    column_t right_vertices;
    column_t previous_vertices;
    column_iterator_t left_candidate; // candidate in previous_vertices (not in left_column).

    // to reduce memory allocations, we reuse this temporary vector object.
    mutable std::vector<subtrajectory_length_t> pathlet_weights;
};

} // namespace frechet


