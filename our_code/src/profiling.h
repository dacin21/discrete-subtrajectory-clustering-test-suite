#pragma once

#include <chrono>
#include <ostream>

namespace frechet {

namespace time {
    using clock_type = std::chrono::high_resolution_clock;
    using duration_type = std::chrono::nanoseconds;
    using duration_rep = duration_type::rep;

    clock_type::time_point time_now();

    duration_rep time_diff(clock_type::time_point begin, clock_type::time_point end);
} // End of namespace `time`

#ifdef PROFILING
    #define TIME_VARIABLE_NAME(name) time_##name##_sum
    #define TIME_VARIABLE(name) time::duration_rep TIME_VARIABLE_NAME(name) = 0
    #define TIME_FUNCTION(name) \
        void time_##name(time::duration_rep t) { \
            TIME_VARIABLE_NAME(name) += t; \
        }
    #define PROF_TIME(name) \
        public: TIME_FUNCTION(name) \
        private: TIME_VARIABLE(name);

    #define COUNT_VARIABLE_NAME(name) count_##name##_value
    #define COUNT_VARIABLE(name) size_t COUNT_VARIABLE_NAME(name) = 0
    #define COUNT_INC_FUNCTION(name) \
        void increment_##name() { \
            ++COUNT_VARIABLE_NAME(name); \
        }
    #define PROF_COUNT(name) \
        public: COUNT_INC_FUNCTION(name) \
        private: COUNT_VARIABLE(name);
#else
    #define TIME_VARIABLE_NAME(name)
    #define TIME_VARIABLE(name)
    #define TIME_FUNCTION(name)
    #define PROF_TIME(name)
    #define COUNT_VARIABLE_NAME(name)
    #define COUNT_VARIABLE(name)
    #define COUNT_INC_FUNCTION(name)
    #define PROF_COUNT(name)
#endif



class subtrajectory_clustering_profile {
    PROF_TIME(total)
    PROF_TIME(total_precluster)
    PROF_TIME(populate_column)
    PROF_TIME(query_cluster)
    PROF_TIME(extract_trajectory)
    PROF_TIME(total_dbscan)
    PROF_TIME(dbscan_reach)
    PROF_TIME(dbscan_new_clusters)
    PROF_TIME(frechet_distance_test)
    PROF_COUNT(zeroes)

    friend std::ostream& operator<<(std::ostream &stream, const subtrajectory_clustering_profile &profile);

};

inline std::ostream& operator<<(std::ostream &stream,
                                [[maybe_unused]] const subtrajectory_clustering_profile &profile) {
    #ifdef PROFILING
        stream << "time_total_precluster: " << profile.TIME_VARIABLE_NAME(total_precluster)/1000000 << "\n";
        stream << "time_populate_column: " << profile.TIME_VARIABLE_NAME(populate_column)/1000000 << "\n";
        stream << "time_query_cluster: " << profile.TIME_VARIABLE_NAME(query_cluster)/1000000 << "\n";
        stream << "time_extract_trajectory: " << profile.TIME_VARIABLE_NAME(extract_trajectory)/1000000 << "\n";
        stream << "count_zeroes: " << profile.COUNT_VARIABLE_NAME(zeroes) << "\n";
        stream << "time_total_dbscan: " << profile.TIME_VARIABLE_NAME(total_dbscan)/1000000 << "\n";
        stream << "time_dbscan_reach: " << profile.TIME_VARIABLE_NAME(dbscan_reach)/1000000 << "\n";
        stream << "time_dbscan_new_clusters: " << profile.TIME_VARIABLE_NAME(dbscan_new_clusters)/1000000 << "\n";
        stream << "time_frechet_distance_test: " << profile.TIME_VARIABLE_NAME(frechet_distance_test)/1000000 << "\n";
        stream << "time_total: " << profile.TIME_VARIABLE_NAME(total)/1000000 << "\n";
    #endif
    return stream;
}

#ifdef PROFILING
    extern subtrajectory_clustering_profile subt_cluster_profile;

    #define TIME_BEGIN() const auto time_begin_v = time::time_now()
    #define TIME_END(name) const auto time_end_v = time::time_now(); \
        subt_cluster_profile.time_##name(time::time_diff(time_begin_v, time_end_v))
    #define INCREMENT_COUNT(name) subt_cluster_profile.increment_##name()

    #define DUMP_SUBT_CLUSTER_PROFILE(stream) stream << subt_cluster_profile
#else
    #define TIME_BEGIN() do {} while(false)
    #define TIME_END(name) do {} while(false)
    #define INCREMENT_COUNT(name) do {} while(false)

    #define DUMP_SUBT_CLUSTER_PROFILE(stream) do {} while(false)
#endif

}
