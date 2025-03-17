#pragma once

#include "metric_space.h"
#include "trajectory.h"
#include <cstddef>

namespace frechet {

namespace internal {

template<typename T>
class array_2d {

public:
    array_2d(size_t num_rows, size_t num_cols,
             size_t row_0, size_t col_0) : num_rows(num_rows),
                                           num_cols(num_cols),
                                           row_0(row_0),
                                           col_0(col_0) {
        data.resize(num_rows*num_cols);
    }

    T& at(size_t row, size_t col) {
        row = row - row_0;
        col = col - col_0;
        assert(row < num_rows);
        assert(col < num_cols);
        return data[row + num_rows * col];
    }

private:
    std::vector<T> data;
    const size_t num_rows;
    const size_t num_cols;
    const size_t row_0;
    const size_t col_0;

};

} // End of namespace `internal`

template<metric_space m_space>
class frechet_distance {

public:
    using space = m_space;

    using trajectory_t = trajectory_collection<space>;
    using index_t = trajectory_t::index_t;
    using subtrajectory_t = trajectory_t::subtrajectory_t;

    using distance_function_t = space::distance_function_t;
    using distance_t = space::distance_function_t::distance_t;

    // Compute the Fréchet distance using the algorithm by Eiter and Mannila (1994)
    // Space and time: O(|P||Q|)
    static distance_t compute(const trajectory_t &trajectory, const subtrajectory_t &P, const subtrajectory_t &Q) {
        if (P == Q) {
            return 0;
        }
        internal::array_2d<distance_t> ca(P.second - P.first + 1, Q.second - Q.first + 1,
                                          P.first, Q.first);
        for (auto jq = Q.first; jq <= Q.second; ++jq) {
            for (auto ip = P.first; ip <= P.second; ++ip) {
                auto d_ij = distance_function_t{}(trajectory[ip], trajectory[jq]);
                if (ip == P.first && jq == Q.first) {
                    ca.at(ip, jq) = d_ij;
                } else if (ip > P.first && jq == Q.first) {
                    ca.at(ip, jq) = std::max(ca.at(ip-1, jq),
                                             d_ij);
                } else if (ip == P.first && jq > Q.first) {
                    ca.at(ip, jq) = std::max(ca.at(ip, jq-1),
                                             d_ij);
                } else { // if (i > P.first && j > Q.first) {
                    ca.at(ip, jq) = std::max(std::min({ca.at(ip-1, jq),
                                                       ca.at(ip-1, jq-1),
                                                       ca.at(ip, jq-1)}),
                                             d_ij);
                }
            }
        }
        return ca.at(P.second, Q.second);
    }

    // Same as `compute(...)`, but linear space.
    static distance_t compute_light(const trajectory_t &trajectory, const subtrajectory_t &P, const subtrajectory_t &Q) {
        using row_t = std::vector<distance_t>;
        std::array<row_t, 2> rows;
        row_t* current_row = &rows[0];
        row_t* next_row = &rows[1];
        current_row->resize(P.second - P.first + 1);
        next_row->resize(P.second - P.first + 1);
        index_t q_idx = Q.first;
        // Initialize first row
        (*current_row)[0] = distance_function_t{}(trajectory[P.first], trajectory[Q.first]);
        for (index_t i = 1; i < current_row->size(); ++i) {
            const auto p_idx = P.first + i;
            const auto d_ij = distance_function_t{}(trajectory[p_idx], trajectory[q_idx]);
            (*current_row)[i] = std::max((*current_row)[i-1], d_ij);
        }
        // Compute remaining rows
        for (q_idx = Q.first + 1; q_idx <= Q.second; ++q_idx) {
            // Compute the next row
            (*next_row)[0] = std::max((*current_row)[0], distance_function_t{}(trajectory[P.first], trajectory[q_idx]));
            for (index_t i = 1; i < current_row->size(); ++i) {
                const auto p_idx = P.first + i;
                const auto d_ij = distance_function_t{}(trajectory[p_idx], trajectory[q_idx]);
                (*next_row)[i] = std::max(std::min({(*next_row)[i-1],
                                                    (*current_row)[i-1],
                                                    (*current_row)[i]}),
                                       d_ij);
            }
            // Make the next row the current row
            std::swap(current_row, next_row);
        }
        return current_row->back();
    }

};

} // End of namespace `frechet`
