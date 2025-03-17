#include "profiling.h"

namespace frechet {

subtrajectory_clustering_profile subt_cluster_profile{};

namespace time {

    clock_type::time_point time_now() {
        return clock_type::now();
    }

    duration_rep time_diff(clock_type::time_point begin, clock_type::time_point end) {
        return std::chrono::duration_cast<duration_type>(end-begin).count();
    }

} // End of namespace `time`

} // End of namespace `frechet`