#pragma once

#include "metric_space.h"
#include "subtrajectory_cluster.h"
#include "trajectory.h"

namespace frechet {

template<metric_space m_space>
class validation {

private:
    using space = m_space;
    using trajectory_t = trajectory_collection<space>;
    using index_t = trajectory_t::index_t;
    using subtrajectory_t = trajectory_t::subtrajectory_t;

    using subtrajectory_cluster_t = subtrajectory_cluster<space>;

public:

    static bool validate_no_overlap(const std::vector<subtrajectory_cluster_t> &clusters) {
        std::vector<bool> is_covered;
        bool found_overlap = false;
        for (const auto &cluster: clusters) {
            for (const auto &subt: cluster.get_subtrajectories()) {
                for (index_t i = subt.first; i <= subt.second; ++i) {
                    if (i >= is_covered.size()) {
                        is_covered.resize(i+1, false);
                    }
                    if (is_covered[i]) {
                        std::cout << "Found an overlap at vertex " << i << "\n";
                        found_overlap = true;
                    }
                    is_covered[i] = true;
                }
            }
        }
        return found_overlap;
    }


};

} // End of namespace `frechet`
