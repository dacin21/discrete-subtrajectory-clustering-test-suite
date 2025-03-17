#pragma once

#include <cmath>
#include <iostream>
#include <limits>
#include <list>
#include <memory>
#include <optional>

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
class basic_clustering_algo {

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
    // set min_distance or max_distance to -1 to have the algorithm compute it.
    basic_clustering_algo(const trajectory_t &trajectory,
            distance_t min_distance = -1,
            distance_t max_distance = -1,
            std::optional<index_t> target_length = {}, // exactly one of target_length and cluster_size should be set.
            std::optional<index_t> target_size = {},
            const efficacy_factor_t &efficacy_factors = {1, 1, 1, false}) :
                    trajectory(trajectory),
                    range_search(this->trajectory),  // Using `this->trajectory`,
                                                     // otherwise `range_search` has a reference
                                                     // to a different trajectory than we actually use.
                    distance_limit(max_distance),
                    target_length(target_length),
                    target_size(target_size),
                    efficacy_factors(efficacy_factors)
    {
        initialize_distance_limits(min_distance, max_distance);
        assert(!!target_length != !!target_size);
    }

    void perform_means_clustering() {
        // The distance isn't fixed yet, so we will multiply with it later.
        while (trajectory.get_actual_size() > 0) {
            subtrajectory_cluster_t best_cluster;
            // clusters with lower gamma would *increase* our k-means score.
            distance_t best_gamma = 1.0 / efficacy_factors.c_3;
            for (const auto &dist: sq_distances) {
                subtrajectory_cluster_t candidate_cluster;
                if (target_length) {
                    k_cluster_detail::find_best_cluster_at_fixed_distance_and_length(trajectory, range_search, dist, candidate_cluster, *target_length);
                } else {
                    k_cluster_detail::find_best_cluster_at_fixed_distance_and_size(trajectory, range_search, dist, candidate_cluster, *target_size);
                }
                distance_t gamma = k_cluster_detail::compute_gamma(trajectory, candidate_cluster, efficacy_factors);
                if (gamma > best_gamma) {
                    best_cluster = candidate_cluster;
                    best_gamma = gamma;
                }
            }
            k_cluster_detail::prune_inefficient_subtrajectories(trajectory, best_cluster, best_gamma, efficacy_factors);
            if (best_cluster.get_subtrajectories().empty()) return;

            std::cout << "Best cluster has"
                << " gamma: " << best_gamma
                << ", c3 * gamma: " << efficacy_factors.c_3 * best_gamma
                << ", vertices: " << best_cluster.number_of_vertices() << "\n";

            pathlets.push_back(best_cluster);
            k_cluster_detail::erase_points_in_cluster(trajectory, range_search, best_cluster);
        }
    }
    void perform_center_clustering() {
        std::vector<std::unique_ptr<fixed_d_cluster>> clustering_algos;
        for (const auto &dist: sq_distances) {
            clustering_algos.emplace_back(std::make_unique<fixed_d_cluster>(trajectory, dist));
        }
        #pragma omp parallel for
        for (size_t i = 0; i < clustering_algos.size(); ++i) {
            if (target_length) {
                clustering_algos[i]->perform_clustering_given_distance_and_length(*target_length);
            } else {
                clustering_algos[i]->perform_clustering_given_distance_and_length(*target_size);
            }

            clustering_algos[i]->drop_inefficient_clusters_center(efficacy_factors);
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

    // this->trajectory might be modified, e.g. because we deleted the covered points.
    // Thus, we need to pass the original trajectory as an argument.
    void print_clustering_spaced(trajectory_t &trajectory, std::ostream &stream) {
        k_cluster_detail::print_clustering_spaced(trajectory, pathlets, stream);
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
    // Maximum distance at which to search for clusters
    distance_t distance_limit;
    // Exactly one of these is set.
    std::optional<index_t> target_length, target_size;
    // Weights for computing the efficacy
    const efficacy_factor_t efficacy_factors;

    void initialize_distance_limits(distance_t min_distance, distance_t max_distance){
        if(min_distance < 0 || max_distance < 0){
            const auto [trajectory_min_sq, trajectory_max_sq] = k_cluster_detail::compute_min_max_sq_distance(trajectory, range_search);
            if(min_distance < 0) {
                min_distance = std::sqrt(trajectory_min_sq);
                std::cout << "minimum distance: " << min_distance << "\n";
            }
            if(max_distance < 0) {
                max_distance = std::sqrt(trajectory_max_sq);
                std::cout << "maximum distance: " << max_distance << "\n";
            }
            // avoid weirdness if only one of min_distance, max_distance was computed and is now incompatible with the other (specified) one.
            assert(min_distance <= max_distance);
        }
        distance_limit = max_distance;
        k_cluster_detail::initialize_sq_distances(min_distance*min_distance, max_distance*max_distance, sq_distances);
    }
};

} // End of namespace `frechet`

