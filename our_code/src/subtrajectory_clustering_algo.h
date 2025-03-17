#pragma once

#include <cmath>
#include <iostream>
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
class subtrajectory_clustering_algo {

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
    subtrajectory_clustering_algo(const trajectory_t &trajectory,
            distance_t min_distance = -1,
            distance_t max_distance = -1,
            const efficacy_factor_t &efficacy_factors = {1, 1, 1, false},
            const rightstep_config &config = {}) :
                    trajectory(trajectory),
                    range_search(this->trajectory),  // Using `this->trajectory`,
                                                     // otherwise `range_search` has a reference
                                                     // to a different trajectory than we actually use.
                    distance_limit(max_distance),
                    efficacy_factors(efficacy_factors),
                    config(config)
    {
        initialize_distance_limits(min_distance, max_distance);
    }

    // Perform clustering for $EVAL = CENTER$.
    // If `efficacy_factors.ignore_point_clusters` is true, clusters with empty reference trajectory are ignored when estimating efficacy;
    // instead, the points in these clustered are treated as un-clustered
    void perform_center_clustering() {
        // For k-centers we can directly maximize the coverage, skipping the (slow) cost computation.
        config.cost_per_pathlet = 0;
        std::vector<std::unique_ptr<fixed_d_cluster>> clustering_algos;
        for (const auto &dist: sq_distances) {
            clustering_algos.emplace_back(std::make_unique<fixed_d_cluster>(trajectory, dist));
        }
        #pragma omp parallel for
        for (size_t i = 0; i < clustering_algos.size(); ++i) {
            clustering_algos[i]->perform_clustering_rightstep(config);
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
    // Perform clustering for $EVAL = CENTER$.
    // If `efficacy_factors.ignore_point_clusters` is true, clusters with empty reference trajectory are ignored when estimating efficacy;
    // instead, the points in these clustered are treated as un-clustered
    void perform_means_clustering() {
        // The distance isn't fixed yet, so we will multiply with it later.
        config.cost_per_pathlet = efficacy_factors.c_2 / efficacy_factors.c_1; 
        std::vector<std::unique_ptr<fixed_d_cluster>> clustering_algos;
        std::vector<std::pair<bool, subtrajectory_cluster_t> > candidate_clusters;
        std::vector<distance_t> gamma;
        for (const auto &dist: sq_distances) {
            clustering_algos.emplace_back(std::make_unique<fixed_d_cluster>(trajectory, dist));
            candidate_clusters.emplace_back();
            gamma.emplace_back();
        }
        while (clustering_algos.front()->count_remaining_points() > 0) {
            #pragma omp parallel for
            for (size_t i = 0; i < clustering_algos.size(); ++i) {
                auto &[success, output_cluster] = candidate_clusters[i];
                success = clustering_algos[i]->find_best_cluster_rightstep(output_cluster, config);
                assert(success);
                gamma[i] = clustering_algos[i]->compute_gamma(output_cluster, efficacy_factors);
            }
            // Pick best cluster as in Section 4.3 of Agarwal et. al, 2018.
            size_t best_i = std::max_element(gamma.begin(), gamma.end()) - gamma.begin();
            auto &best_cluster = candidate_clusters[best_i].second;
            k_cluster_detail::prune_inefficient_subtrajectories(trajectory, best_cluster, gamma[best_i], efficacy_factors);
            // On very dense data sets, this can happen when very few points remain, due to floating point inaccuracies.
            // The remaining points are better left unclustered, assuming c1 > 0.
            if (best_cluster.get_subtrajectories().empty()) {
                break;
            }
            std::cout << "Best cluster has "
                << "distance: " << std::sqrt(sq_distances[best_i])
                << ", gamma: " << gamma[best_i]
                << ", vertices: " << best_cluster.number_of_vertices() << "\n";

            // Each algo has it's own trajectory and range search object.
            // I think this is required for the parallel for loop.
            for (auto &algo : clustering_algos) {
                algo->establish_cluster(best_cluster);
            }
        }
        // all algorithms store the same clustering.
        clustering_algos.front()->drop_inefficient_clusters_means(efficacy_factors);
        pathlets = clustering_algos.front()->get_clusters();
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
    // Weights for computing the efficacy
    const efficacy_factor_t efficacy_factors;
    rightstep_config config;

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

