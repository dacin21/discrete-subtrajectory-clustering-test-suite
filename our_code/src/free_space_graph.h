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
#include "trajectory.h"

namespace frechet {

// A version of the `sparse_free_space_graph` that doesn't use a dictionary.
template<metric_space m_space>
class sparse_free_space_graph {

public:
    using space = m_space;
    using point_t = m_space::point_t;
    using trajectory_t = trajectory_collection<space>;
    using index_t = trajectory_t::index_t;
    using subtrajectory_t = trajectory_t::subtrajectory_t;
    using subtrajectory_cluster_t = subtrajectory_cluster<space>;

public:
    using row_index_t = index_t;

private:
    struct vertex {
        using label_t = index_t;
        static constexpr label_t no_edge_label = std::numeric_limits<label_t>::max();

        vertex(const row_index_t &row_index) : row_index(row_index) {}

        row_index_t row_index;

        vertex* up = nullptr;

        vertex* left = nullptr;
        vertex* below_left = nullptr;
        vertex* below = nullptr;

        label_t label_left = no_edge_label;
        label_t label_below_left = no_edge_label;
        label_t label_below = no_edge_label;
        label_t min_label = no_edge_label;
    };

public:
    sparse_free_space_graph(const index_t &init_right_column) : left_column(init_right_column),
                                                                right_column(init_right_column) {
        lowest_vertex_per_column.push_back(nullptr);
    }

    void new_column() {
        new_column(right_column + 1);
    }

    void new_column(index_t new_right_column) {
        if (!lowest_vertex_per_column.empty() && lowest_vertex_per_column.back() != nullptr) {
            candidate_for_left = lowest_vertex_per_column.back();
            candidate_for_below_left = candidate_for_left->below;
        } else {
            candidate_for_left = candidate_for_below_left = nullptr;
        }
        lowest_vertex_per_column.push_back(nullptr);
        highest_in_last_col = nullptr;
        right_column = new_right_column;
    }

    void add_zero(row_index_t row_idx) {
        INCREMENT_COUNT(zeroes);

        assert((highest_in_last_col == nullptr) || (highest_in_last_col->row_index < row_idx));
        auto *new_vertex = vertex_pool.construct(row_idx);
        if (lowest_vertex_per_column.back() == nullptr) {
            lowest_vertex_per_column.back() = new_vertex;
        }
        if (right_column > 0) {
            advance_candidate_for_left(row_idx);
            if (candidate_for_left != nullptr && candidate_for_left->row_index == row_idx) {
                new_vertex->left = candidate_for_left;
                new_vertex->label_left = candidate_for_left->min_label;
            }
            if (candidate_for_below_left != nullptr && candidate_for_below_left->row_index == row_idx - 1) {
                new_vertex->below_left = candidate_for_below_left;
                new_vertex->label_below_left = candidate_for_below_left->min_label;
            }
        }
        if (highest_in_last_col != nullptr) {
            new_vertex->below = highest_in_last_col;
            new_vertex->label_below = highest_in_last_col->row_index == row_idx - 1 ? highest_in_last_col->min_label
                                                                                    : vertex::no_edge_label;
            highest_in_last_col->up = new_vertex;
        }
        highest_in_last_col = new_vertex;
        new_vertex->min_label = std::min({new_vertex->label_left,
                                          new_vertex->label_below_left,
                                          new_vertex->label_below,
                                          right_column});
    }

    void delete_column() {
        auto *delete_ptr = lowest_vertex_per_column.front();
        lowest_vertex_per_column.pop_front();
        while (delete_ptr != nullptr) {
            auto *next_ptr = delete_ptr->up;
            vertex_pool.free(delete_ptr);
            delete_ptr = next_ptr;
        }
        left_column++;
    }

    void advance_left_column_to_right() {
        while (lowest_vertex_per_column.size() > 1) {
            auto *delete_ptr = lowest_vertex_per_column.front();
            lowest_vertex_per_column.pop_front();
            while (delete_ptr != nullptr) {
                auto *next_ptr = delete_ptr->up;
                vertex_pool.free(delete_ptr);
                delete_ptr = next_ptr;
            }
        }
        left_column = right_column;
    }

    void query_subtrajectories_respecting_ids(const trajectory_t &trajectory,
                                              subtrajectory_cluster_t &output_trajectories,
                                              const index_t target_num_trajectories = std::numeric_limits<index_t>::max()) const {
        assert(output_trajectories.empty());
        TIME_BEGIN();

        auto* start_vertex = highest_in_last_col;
        vertex* end_vertex = nullptr;
        auto next_row = start_vertex->row_index;
        while (true) {
            do {
                if (trajectory.get_id_at(start_vertex->row_index) == trajectory_t::deleted_id) {
                    next_row--;
                }
                start_vertex = find_eligible_row(start_vertex, next_row);
            } while (start_vertex != nullptr && trajectory.get_id_at(start_vertex->row_index) == trajectory_t::deleted_id);
            if (start_vertex == nullptr) {
                break;
            }
            // traverse the graph from (right_column, current_row) to (left_column, j)
            subtrajectory_t candidate;
            const auto [success, next_row_c, next_end_vertex] = extract_trajectory_respecting_ids(trajectory,
                                                                                 start_vertex,
                                                                                 candidate);
            next_row = next_row_c;
            if (success) {
                if (end_vertex) {
                    optimize_in_left_column(trajectory, output_trajectories.back(), end_vertex, start_vertex);
                }
                end_vertex = next_end_vertex;
                next_row = candidate.first - 1;
                output_trajectories.push_back(candidate);
                if (candidate.first == 0 || output_trajectories.size() >= target_num_trajectories) {
                    break;
                }
            }
            if (next_row == start_vertex->row_index) {
                if (next_row == 0) {
                    break;
                }
                next_row--;
            }
        }
        if (end_vertex) {
            optimize_in_left_column(trajectory, output_trajectories.back(), end_vertex, nullptr);
        }
        output_trajectories.push_back({left_column, right_column});
        output_trajectories.set_reference_trajectory({left_column, right_column});

        TIME_END(query_cluster);
    }

private:
    lost::recycling_object_pool<vertex> vertex_pool;
    // List of the lowest vertices in each column
    // `.front()` corresponds to the lowest vertex in the `left_column`
    // `.back()` corresponds to the lowest vertex in the `right_column`
    std::list<vertex*> lowest_vertex_per_column;
    index_t left_column = 0;
    index_t right_column = 0;
    vertex* highest_in_last_col = nullptr;
    vertex* candidate_for_below_left = nullptr;
    vertex* candidate_for_left = nullptr;

    // Advance the vertices `candidate_for_left` such that it lies at least in the given row.
    // Sets `candidate_for_below_left` to be the next free vertex down from `candidate_for_left`.
    void advance_candidate_for_left(const row_index_t &row) {
        while (candidate_for_left != nullptr && candidate_for_left->row_index < row) {
            candidate_for_below_left = candidate_for_left;
            candidate_for_left = candidate_for_left->up;
        }
    }
    
    // Find the next vertex below `current_vertex` whose row index is at most `below_this`,
    // and which lies on a path that ends in a vertex in the column indicated by `left_column`.
    // Returns a pointer to the vertex if such a vertex was found, `nullptr` otherwise.
    vertex* find_eligible_row(vertex* current_vertex, index_t below_this) const {
        auto *v_ptr = current_vertex;
        while (v_ptr != nullptr &&
                (v_ptr->row_index > below_this || v_ptr->min_label > left_column)) {
            v_ptr = v_ptr->below;
        }
        return v_ptr;
    }

    // Extract the subtrajectory to `start_vertex`, write it into `output_trajectory` and return true,
    // unless it overlaps the subtrajectory defined by indices `left_column` and `right_column`.
    // If it overlaps this subtrajectory, return false.
    // The value of `output_trajectory` has no meaning if this function returns false.
    bool extract_trajectory(vertex *start_vertex,
                            subtrajectory_t &output_trajectory) const {
        TIME_BEGIN();

        if (left_column <= start_vertex->row_index && start_vertex->row_index <= right_column) {
            // We are already in "reference trajectory territory"
            TIME_END(extract_trajectory); return false;
        }
        auto current_column_idx = right_column;
        output_trajectory.first = start_vertex->row_index;
        output_trajectory.second = start_vertex->row_index;
        while (current_column_idx > left_column) {
            if (start_vertex->label_left <= left_column) {
                current_column_idx--;
                start_vertex = start_vertex->left;
            } else if (start_vertex->label_below_left <= left_column) {
                current_column_idx--;
                output_trajectory.first--;
                start_vertex = start_vertex->below_left;
            } else if (start_vertex->label_below <= left_column) {
                output_trajectory.first--;
                start_vertex = start_vertex->below;
            }
            if (left_column <= output_trajectory.first && output_trajectory.first <= right_column) {
                // We walked into "reference trajectory territory", so we report failure.
                TIME_END(extract_trajectory); return false;
            }
        }
        TIME_END(extract_trajectory); return true;
    }

    // Extract the subtrajectory to `start_vertex`, write it into `output_trajectory` and return.
    // Writes the resulting vertex in the left row to `output_vertex`.
    // Returns `{true, next_row}` if successful, `{false, next_row}` if no valid subtrajectory was found.
    // Here, `next_row` is the row in which the algorithm should continue.
    // Non-valid subtrajectories are those overlapping the reference subtrajectory defined by `left_column` and `right_column`,
    // and those subtrajectories that cross input trajectory boundaries.
    // The value of `output_trajectory` and `output_vertex` has no meaning if this function returns false.
    std::tuple<bool, index_t, vertex*> extract_trajectory_respecting_ids(const trajectory_t &trajectory,
                                                               vertex *start_vertex,
                                                               subtrajectory_t &output_trajectory) const {
        TIME_BEGIN();

        if (left_column <= start_vertex->row_index && start_vertex->row_index <= right_column) {
            // We are already in "reference trajectory territory"
            TIME_END(extract_trajectory); return {false, left_column, nullptr};
        }
        auto current_column_idx = right_column;
        output_trajectory.first = start_vertex->row_index;
        output_trajectory.second = start_vertex->row_index;
        while (current_column_idx > left_column) {
            if (start_vertex->label_left <= left_column) {
                current_column_idx--;
                start_vertex = start_vertex->left;
            } else if (start_vertex->label_below_left <= left_column) {
                current_column_idx--;
                output_trajectory.first--;
                start_vertex = start_vertex->below_left;
            } else if (start_vertex->label_below <= left_column) {
                output_trajectory.first--;
                start_vertex = start_vertex->below;
            }
            if (left_column <= output_trajectory.first && output_trajectory.first <= right_column) {
                // We walked into "reference trajectory territory", so we report failure.
                TIME_END(extract_trajectory); return {false, left_column, nullptr};
            }
            if (trajectory.get_id_at(output_trajectory.first) != trajectory.get_id_at(output_trajectory.second)) {
                TIME_END(extract_trajectory); return {false, output_trajectory.second, nullptr};
            }
        }
        TIME_END(extract_trajectory); return {true, output_trajectory.second, start_vertex};
    }
    // Optimize the starting point of `subtrajectory` by walking down from end_vertex in the leftmost column,
    // without crossing the row of `next_start_vertex` or violating id constraints.
    void optimize_in_left_column(const trajectory_t &trajectory, subtrajectory_t &subtrajectory, vertex *end_vertex, vertex* const next_start_vertex) const {
        auto& row = subtrajectory.first;
        const auto trajectory_id = trajectory.get_id_at(row);
        assert(row == end_vertex->row_index);
        do {
            --row;
            end_vertex = end_vertex->below;
        } while ((!next_start_vertex || row > next_start_vertex->row_index) && end_vertex && end_vertex->row_index == row
            && trajectory.get_id_at(row) == trajectory_id && (row < left_column || row > right_column));
        ++row;
    }

};

}
