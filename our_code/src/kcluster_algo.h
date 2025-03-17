#pragma once

#include <cmath>
#include <limits>
#include <list>
#include <memory>

#include "frechet_distance.h"
#include "free_space_graph.h"
#include "kdtree_range_search.h"
#include "kcluster_detail.h"
#include "metric_space.h"
#include "profiling.h"
#include "subtrajectory_cluster.h"
#include "subtrajectory_routine_bbgll.h"
#include "trajectory.h"

namespace frechet {

template<metric_space m_space>
class k_clustering_algo {

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

    using efficacy_factor_t = internal::efficacy_factor_vec<distance_t>;

private:
    using k_cluster_detail = internal::k_cluster_detail<space>;

    using fixed_d_cluster = internal::fixed_distance_clustering<space>;

public:
    k_clustering_algo(const trajectory_t &trajectory,
            index_t initial_min_pathlet_length,
            distance_t min_distance = 0.1,
            distance_t max_distance = std::numeric_limits<distance_t>::infinity(),
            bool kmeans_bias_clustering = false,
            index_t cluster_scan_step = 1,
            const efficacy_factor_t &efficacy_factors = {1, 1, 1, false}) :
                    trajectory(trajectory),
                    range_search(this->trajectory),  // Using `this->trajectory`,
                                                     // otherwise `range_search` has a reference
                                                     // to a different trajectory than we actually use.
                    initial_min_pathlet_length(initial_min_pathlet_length),
                    min_pathlet_length(initial_min_pathlet_length),
                    distance_limit(max_distance),
                    kmeans_bias_clustering_on(kmeans_bias_clustering),
                    cluster_scan_step(cluster_scan_step),
                    efficacy_factors(efficacy_factors)
    {
        k_cluster_detail::initialize_sq_distances(min_distance*min_distance, max_distance*max_distance, sq_distances);
    }

    // Perform clustering for $EVAL = MEANS$.
    void perform_means_clustering() {
        std::cout << "Using " << sq_distances.size() << " distances\n";
        while (trajectory.get_actual_size() > 0) {
            // Compute a cluster for each `d` in `distances` using the `clustering_algo`
            // Keep the one with highest score
            // Delete it from the trajectory
            
            std::cout << "Next iteration...\n";
            std::cout << "  " << trajectory.get_actual_size() << " points left\n";

            auto start = time::time_now();
            
            subtrajectory_cluster_t best_cluster;
            const auto success = find_best_cluster(best_cluster);
            if (success) {
                pathlets.push_back(best_cluster);
                k_cluster_detail::erase_points_in_cluster(trajectory, range_search, best_cluster);
            } else {
                --min_pathlet_length;
            }

            std::cout << "  Pathlet length: "
                      << (best_cluster.get_reference_subtrajectory().second - best_cluster.get_reference_subtrajectory().first)
                      << " segments\n";
            std::cout << "  Cluster size: " << best_cluster.size() << "\n";
            std::cout << "  Number of vertices covered: " << best_cluster.number_of_vertices() << "\n";

            std::cout << "  Time for this iteration: " << time::time_diff(start, time::time_now())/1000000 << "\n";
            std::cout.flush();
        }
    }

    // Perform clustering for $EVAL = CENTER$.
    // If `efficacy_factors.ignore_point_clusters` is true, clusters with empty reference trajectory are ignored when estimating efficacy;
    // instead, the points in these clustered are treated as un-clustered
    void perform_center_clustering() {
        std::vector<std::unique_ptr<fixed_d_cluster>> clustering_algos;
        for (const auto &dist: sq_distances) {
            clustering_algos.emplace_back(std::make_unique<fixed_d_cluster>(trajectory, dist));
        }
        #pragma omp parallel for
        for (size_t i = 0; i < clustering_algos.size(); ++i) {
            clustering_algos[i]->perform_clustering_bbgll(initial_min_pathlet_length, cluster_scan_step);
            std::cout << "Clustered at distance " << clustering_algos[i]->get_sq_distance() << std::endl;
        }
        distance_t best_efficacy = std::numeric_limits<distance_t>::infinity();
        fixed_d_cluster* algo_with_best_result = nullptr;
        std::cout << "Finding the best clustering...\n";
        for (auto &algo: clustering_algos) {
            auto efficacy = k_cluster_detail::compute_efficacy_center(trajectory, algo->get_clusters(), efficacy_factors);
            std::cout << " Best so far: " << best_efficacy << ", current: " << efficacy << ", at distance " << algo->get_sq_distance() << "\n";
            if (efficacy < best_efficacy) {
                best_efficacy = efficacy;
                algo_with_best_result = algo.get();
            }
        }
        std::cout << "Efficacy of best found clustering: " << best_efficacy << ", at distance " << algo_with_best_result->get_sq_distance() << "\n";
        assert(algo_with_best_result != nullptr);
        pathlets = algo_with_best_result->get_clusters();
    }

    distance_t compute_means_efficacy() {
        return k_cluster_detail::compute_efficacy_means(trajectory, pathlets, efficacy_factors);
    }
    distance_t compute_center_efficacy() {
        return k_cluster_detail::compute_efficacy_center(trajectory, pathlets, efficacy_factors);
    }

    void print_pathlets() {
        k_cluster_detail::print_pathlets(pathlets);
    }

    void print_clustering_csv(std::ostream &stream) {
        k_cluster_detail::print_clustering_csv(pathlets, stream);
    }

    const std::vector<subtrajectory_cluster_t>& get_clusters() const {
        return pathlets;
    }

private:
    trajectory_t trajectory;
    range_search_t range_search;
    std::vector<distance_t> sq_distances;
    std::vector<subtrajectory_cluster_t> pathlets;
    // Initial minimum length of a pathlet
    const index_t initial_min_pathlet_length;
    index_t min_pathlet_length;
    // Maximum distance at which to search for clusters
    const distance_t distance_limit;
    // If true, bias the cluster score in kmeans against point clusters and those of size 1
    const bool kmeans_bias_clustering_on;
    // How many sizes to skip when finding the best cluster at a fixed distance
    const index_t cluster_scan_step;
    // Weights for computing the efficacy
    const efficacy_factor_t efficacy_factors;

    // Find the best cluster among all maximum cardinality clusters of length at least `min_pathlet_length`.
    // Returns `true` if a cluster of length `min_pathlet_length` exists; `false` otherwise.
    // Assumes that distances are sorted in increasing order
    bool find_best_cluster(subtrajectory_cluster_t &best_cluster) {
        distance_t best_cover = 0;
        bool found_something = false;
        for (const auto& sq_dist: sq_distances) {
//            subtrajectory_clustering_bbgll<space> clustering_algo{trajectory, range_search};
//            auto temp_cluster = clustering_algo.find_max_cardinality_cluster_maximizing_length(sq_dist, min_pathlet_length);
//            if (temp_cluster.size() == 0) {
//                // If a cluster of length `min_pathlet_length` exists at some distance,
//                // then there exists a subtrajectory of that length, which is a cluster of size 1 at any distance.
//                // Thus, if no cluster of length `min_pathlet_length` exists at onc distance,
//                // then no such cluster exists at any other distance and we don't have to consider further distances.
//                assert(min_pathlet_length > 0);
//                return false;
//            }
            subtrajectory_cluster_t temp_cluster;
            const auto success = k_cluster_detail::find_best_cluster_at_fixed_distance_bbgll(trajectory,
                                                                                       range_search,
                                                                                       sq_dist,
                                                                                       temp_cluster,
                                                                                       min_pathlet_length,
                                                                                       cluster_scan_step);
            if (!success) {
                continue;
            }
            found_something = true;
            const distance_t unsquared_dist = std::sqrt(sq_dist);
            const auto ref_length = temp_cluster.get_reference_subtrajectory().second - temp_cluster.get_reference_subtrajectory().first;
            distance_t temp_cover;
            if (kmeans_bias_clustering_on) {
                temp_cover = temp_cluster.number_of_vertices() / unsquared_dist * ref_length * (temp_cluster.size() - 1) + 1;
            } else {
                temp_cover = temp_cluster.number_of_vertices() / unsquared_dist;
            }
            std::cout << "    Found a cluster of value " << temp_cover
                      << "; current best: " << best_cover
                      << "; distance: " << unsquared_dist
                      << "; # vertices: " << temp_cluster.number_of_vertices()
                      << "; # curves: " << temp_cluster.size() << "\n";
            std::cout.flush();
            if (temp_cover >= best_cover) {
                best_cover = temp_cover;
                // TODO: there's lots of copying going on
                best_cluster = temp_cluster;
            }
        }
        return found_something;
    }

};

} // End of namespace `frechet`
