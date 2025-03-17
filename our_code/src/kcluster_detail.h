#pragma once

#include <cmath>
#include <limits>
#include <list>
#include <memory>

#include "curve_simplification.h"
#include "frechet_distance.h"
#include "free_space_graph.h"
#include "kdtree_range_search.h"
#include "metric_space.h"
#include "profiling.h"
#include "subtrajectory_cluster.h"
#include "subtrajectory_routine_bbgll.h"
#include "subtrajectory_routine_rightstep.h"
#include "trajectory.h"

namespace frechet {

namespace internal {

template<typename T>
struct efficacy_factor_vec {
    T c_1;
    T c_2;
    T c_3;
    bool ignore_point_clusters;
};

template<metric_space m_space>
class k_cluster_detail {

public:
    using space = m_space;
    using range_search_t = kd_tree_range_search<space>;

    using distance_function_t = space::distance_function_t;
    using distance_t = distance_function_t::distance_t;

    using trajectory_t = trajectory_collection<space>;
    using index_t = trajectory_t::index_t;
    using subtrajectory_t = trajectory_t::subtrajectory_t;

    using subtrajectory_cluster_t = subtrajectory_cluster<space>;

    using rightstep_algo_t = subtrajectory_clustering_rightstep<space>;
    using cluster_summary_t  = rightstep_algo_t::cluster_summary_t;

    using efficacy_factor_t = efficacy_factor_vec<distance_t>;
    
    // TODO: These find_best_cluster... methods are rather specific. Consider moving the out of this detail class.
    static bool find_best_cluster_at_fixed_distance_rightstep(const trajectory_t &trajectory,
                                                    range_search_t &range_search,
                                                    distance_t sq_distance,
                                                    const rightstep_config &config,
                                                    subtrajectory_cluster_t &best_cluster) {
        if (trajectory.get_actual_size() == 0) {
            return false;
        }
        // no curve simplification
        if (config.curve_simplification_factor == 0) {
            subtrajectory_clustering_rightstep<space> clustering_algo(trajectory, std::vector<index_t>(trajectory.total_size(), index_t{1}), range_search, config);
            best_cluster = clustering_algo.find_best_cluster(sq_distance);
            return true;
        }
        // with curve simplification
        curve_simplification simplification(trajectory, sq_distance, config.curve_simplification_factor);
        subtrajectory_clustering_rightstep<space> clustering_algo_simplified(simplification.trajectory(), simplification.point_weights(), simplification.range_search(), config);

        const auto cluster_candidate = simplification.unsimplify(clustering_algo_simplified.find_best_cluster_candidate(sq_distance));

        subtrajectory_clustering_rightstep<space> clustering_algo(trajectory, std::vector<index_t>(trajectory.total_size(), index_t{1}), range_search, config);
        best_cluster = clustering_algo.cluster_from_candidate(sq_distance, cluster_candidate);
        return true;
    }

    static bool find_best_cluster_at_fixed_distance_bbgll(const trajectory_t &trajectory,
                                                    range_search_t &range_search,
                                                    distance_t sq_distance,
                                                    subtrajectory_cluster_t &best_cluster,
                                                    index_t min_pathlet_length,
                                                    index_t scan_step) {
        subtrajectory_clustering_bbgll<space> clustering_algo{trajectory, range_search};
        size_t best_cover = 0;
        const auto max_size_cluster = clustering_algo.find_max_cardinality_cluster_of_fixed_length(min_pathlet_length, sq_distance);
        if (max_size_cluster.size() == 0) {
            assert(min_pathlet_length > 0);
            return false;
        }
        const auto max_size = max_size_cluster.size();
        const auto size_diff = std::max(index_t{1}, max_size / scan_step);
        for (auto m = max_size; m > 0; m = m <= size_diff ? 0 : m - size_diff) {
            std::cout << "        Finding longest cluster at size " << m << std::endl;
            auto temp_cluster = clustering_algo.find_longest_cluster_of_target_size_by_cardinality(m, sq_distance);
            auto temp_cover = temp_cluster.number_of_vertices();
            if (temp_cover > best_cover) {
                best_cluster = temp_cluster;
                best_cover = temp_cover;
            }
        }
        return true;
    }

    static bool find_best_cluster_at_fixed_distance_and_length(const trajectory_t &trajectory,
                                                    range_search_t &range_search,
                                                    distance_t sq_distance,
                                                    subtrajectory_cluster_t &best_cluster,
                                                    const index_t pathlet_length) {
        subtrajectory_clustering_bbgll<space> clustering_algo{trajectory, range_search};
        const auto max_size_cluster = clustering_algo.find_max_cardinality_cluster_of_fixed_length(pathlet_length, sq_distance);
        if (max_size_cluster.size() == 0) {
            return false;
        }
        best_cluster = max_size_cluster;
        return true;
    }

    static bool find_best_cluster_at_fixed_distance_and_size(const trajectory_t &trajectory,
                                                    range_search_t &range_search,
                                                    distance_t sq_distance,
                                                    subtrajectory_cluster_t &best_cluster,
                                                    const index_t target_size) {
        subtrajectory_clustering_bbgll<space> clustering_algo{trajectory, range_search};
        const auto max_size_cluster = clustering_algo.find_longest_cluster_of_target_size_by_cardinality(target_size, sq_distance);
        if (max_size_cluster.size() == 0) {
            return false;
        }
        best_cluster = max_size_cluster;
        return true;
    }

    static std::pair<distance_t, distance_t> compute_min_max_sq_distance(const trajectory_t &trajectory,
                                                                         range_search_t &range_search) {
        distance_t min_distance = std::numeric_limits<distance_t>::infinity(), max_distance = 0;
        for (index_t idx = 0; idx < trajectory.total_size(); ++idx) {
            auto [d_nn, d_fn] = range_search.nearest_and_farthest_neighbor(idx);
            if (d_nn > 0) { // TODO: this is a stupid hack
                min_distance = std::min(min_distance, d_nn);
            }
            max_distance = std::max(max_distance, d_fn);
        }
        return {min_distance, max_distance};
    }

    static void initialize_sq_distances(distance_t min_distance, distance_t max_distance, std::vector<distance_t> &distances) {
        auto sq_distance = min_distance;
        while(sq_distance < max_distance) {
            distances.push_back(sq_distance);
	    // Since we use squared distances we multiply by 4 instead of 2
            sq_distance *= 4; // TODO: possibly make this parametrizable: if max/min is close to 2, we don't use many distances
        }
    }

    // Function object for use in `eval` and `compute_efficacy`.
    // Computes `max(a,b)` for distances `a` and `b`.
    struct max_accumulator {
        distance_t operator()(const distance_t &a, const distance_t &b) {
            using namespace std;
            return max(a, b);
        }
    };

    // Function object for use in `eval` and `compute_efficacy`.
    // Computes `a+b` for distances `a` and `b`.
    struct plus_accumulator {
        distance_t operator()(const distance_t &a, const distance_t &b) {
            return a + b;
        }
    };

    // Evaluation of a single cluster; max of distances to the reference trajectory, or sum of distances to the reference trajectory.
    // Use `max_accumulator` and `plus_accumulator` to switch between `max` and `plus`,
    // or supply any other accumulator that your heart desires.
    template<typename Accum = max_accumulator>
    static distance_t eval(const trajectory_t &trajectory, const subtrajectory_cluster_t &cluster, Accum accum = {}) {
        distance_t result = 0;
        const auto& reference = cluster.get_reference_subtrajectory();
        for (const auto &subt: cluster.get_subtrajectories()) {
            result = accum(result, std::sqrt(frechet_distance<space>::compute_light(trajectory, reference, subt)));
        }
        return result;
    }

    // Compute efficacy of a subtrajectory clustering of `trajectory` given by `clusters`.
    // If `ignore_point_clusters` is `true`, then clusters with empty reference trajectories are not counted
    // and their vertices are treated as "unclustered".
    // This uses `eval<Accum>` to compute each cluster's contribution to the evaluation score,
    // and also uses `Accum` to accumulate those individual values.
    // `Accum = max_accumulator` yields the k-center score;
    // `Accum = plus_accumulator` yields the k-means score.
    template<typename Accum = max_accumulator>
    static distance_t compute_efficacy(const trajectory_t &trajectory,
                                       const std::vector<subtrajectory_cluster_t> &clusters,
                                       const efficacy_factor_t &factors,
                                       Accum accum = {}) {
        // TODO: add parameters c_1, c_2, c_3 for weighting terms
        const distance_t c_1 = factors.c_1, c_2 = factors.c_2, c_3 = factors.c_3;
        if (factors.ignore_point_clusters) {
//            size_t number_of_uncovered_vertices = 0;
            size_t number_of_clusters = 0;
            distance_t eval_result = 0;
            for (const auto& c: clusters) {
                const auto ref_length = c.get_reference_subtrajectory().second - c.get_reference_subtrajectory().first;
//                if (ref_length == 0) {
//                    number_of_uncovered_vertices += c.number_of_vertices();
//                } else {
                if (ref_length > 0) {
                    ++number_of_clusters;
                    eval_result = accum(eval_result, eval(trajectory, c, accum)); // TODO: potentially allow to use a different accumulator inside eval
                }
            }
//            auto uncovered_ratio = number_of_uncovered_vertices/(double)trajectory.total_size();
            auto uncovered_ratio = trajectory.compute_uncovered_fraction_sum_without_point_clusters(clusters);
            std::cout << "  Computing efficacy."
                      << " # clusters: " << number_of_clusters
                      << ", eval:" << eval_result
                      << ", uncovered per trajectory: " << uncovered_ratio
                      << ", result: " << (c_1 * number_of_clusters) << " + " << (c_2*eval_result) << " + " << (c_3*uncovered_ratio)
                      << "\n";
            return c_1 * number_of_clusters +
                   c_2 * eval_result +
                   c_3 * uncovered_ratio;
        } else {
            distance_t eval_result = 0;
            for (const auto &c: clusters) {
                eval_result = accum(eval_result, eval(trajectory, c, accum));
            }
            auto uncovered_ratio = trajectory.compute_uncovered_fraction_sum_with_point_clusters(clusters);
            std::cout << "  Computing efficacy."
                      << " # clusters: " << clusters.size() 
                      << ", eval:" << eval_result  
                      << ", uncovered per trajectory: " << uncovered_ratio
                      << ", result: " << (c_1 * clusters.size()) << " + " << (c_2*eval_result) << " + " << (c_3*uncovered_ratio)
                      << "\n";
            return c_1 * clusters.size() +
                   c_2 * eval_result +
                   c_3 * uncovered_ratio;
        }
    }

    static distance_t compute_efficacy_center(const trajectory_t &trajectory,
                                              const std::vector<subtrajectory_cluster_t> &clusters,
                                              const efficacy_factor_t &factors) {
        return compute_efficacy<max_accumulator>(trajectory, clusters, factors);
    }

    static distance_t compute_efficacy_means(const trajectory_t &trajectory,
                                             const std::vector<subtrajectory_cluster_t> &clusters,
                                             const efficacy_factor_t &factors) {
        return compute_efficacy<plus_accumulator>(trajectory, clusters, factors);
    }

    static distance_t compute_subtrajectory_score_delta(const trajectory_t &trajectory,
                                                        const subtrajectory_t &covered_subtrajectory,
                                                        const distance_t frechet_distance,
                                                        const distance_t& gamma,
                                                        const efficacy_factor_t &factors) {
        const auto coverage = (distance_t)(covered_subtrajectory.second - covered_subtrajectory.first + 1) / (distance_t)trajectory.total_size();
        return coverage - gamma * factors.c_2 * frechet_distance;
    }
    static distance_t compute_one_frechet_distance(const trajectory_t &trajectory, const subtrajectory_t &reference_trajectory, const subtrajectory_t &covered_trajectory){
        if (reference_trajectory == covered_trajectory) return 0.0; // avoid floating point errors;
        return std::sqrt(frechet_distance<space>::compute_light(trajectory, reference_trajectory, covered_trajectory));
    }
    static std::vector<distance_t> compute_frechet_distances(const trajectory_t &trajectory, const subtrajectory_cluster_t &cluster) {
        std::vector<distance_t> distances(cluster.size());
        for (size_t i = 0; i < cluster.size(); ++i) {
            distances[i] = compute_one_frechet_distance(trajectory, cluster.get_reference_subtrajectory(), cluster.get_subtrajectories()[i]);
        }
        return distances;
    }
    static distance_t compute_gamma(const trajectory_t &trajectory, const subtrajectory_cluster_t &cluster, const efficacy_factor_t &factors) {
        // This can be the bottleneck on very large sets, so let's precompute it.
        auto distances = compute_frechet_distances(trajectory, cluster);

        auto coverage_distance_score = [&](const distance_t &gamma) {
            distance_t score = 0;
            for (size_t i = 0; i < cluster.size(); ++i){
                score += std::max<distance_t>(0, compute_subtrajectory_score_delta(trajectory, cluster.get_subtrajectories()[i], distances[i], gamma, factors));
            }
            return score;
        };
        distance_t l = 0, r = 1.0;
        for (int it = 0; it < 50 && coverage_distance_score(r) >= r * factors.c_1; ++it) {
            r *= 2;
        };
        for (int it = 0; it < 100; ++it) {
            const auto step = r - l;
            const auto m = l + step / 2;
            if (coverage_distance_score(m) >= m * factors.c_1) {
                l += step * 0.4;
            } else {
                r -= step * 0.4;
            }
        }
        return r;
    }

    static void prune_inefficient_subtrajectories(const trajectory_t &trajectory,
                                                  subtrajectory_cluster_t &cluster,
                                                  const distance_t &gamma,
                                                  const efficacy_factor_t &factors) {
        cluster.erase_subtrajectories_if([&](const subtrajectory_t &subtrajectory) {
            const auto distance = compute_one_frechet_distance(trajectory, cluster.get_reference_subtrajectory(), subtrajectory);
            return compute_subtrajectory_score_delta(trajectory, subtrajectory, distance, gamma, factors) < 0;
        });
    }

    static void erase_points_in_cluster(trajectory_t &trajectory, range_search_t &range_search, const subtrajectory_cluster_t &cluster) {
        for (const auto& subt: cluster.get_subtrajectories()) {
            trajectory.delete_subtrajectory(subt);
            for (auto idx = subt.first; idx <= subt.second; ++idx) {
                range_search.delete_point(idx);
            }
        }
    }

    static void print_pathlets(const std::vector<subtrajectory_cluster_t> &pathlets) {
        for (const auto & cluster: pathlets) {
            const auto& ref = cluster.get_reference_subtrajectory();
            std::cout << "[" << ref.first << ", " << ref.second << "]:\n";
            for (const auto & subt: cluster.get_subtrajectories()) {
                std::cout << "  " << subt.first << ", " << subt.second << "\n";
            }
        }
    }

    // Two lines per cluster. The first line described the pathlet, the second line the covered subtrajectories.
    //
    // <traj> <l> <r>
    // <traj_1> <l_1> <r_1> <traj_2> <l_2> <r_2> ... <traj_n> <l_n> <r_n>
    //
    //     <traj>          integer, id of the reference trajectory of the pathlet.
    //     <l>, <r>        integer, the pathlet consists of the points [l, r] (zero based) of this trajectory.
    //
    //     <traj_i>        integer, id of the trajectory this pathlet covers
    //     <l_i>, <r_i>    integer, on this trajectory, the points [l_i, r_i] (zero based) are covered by the pathlet.
    static void print_clustering_spaced(const trajectory_t &trajectory, const std::vector<subtrajectory_cluster_t> &pathlets, std::ostream &stream) {
        for (const auto & cluster: pathlets) {
            const auto [ref_l, ref_r] = cluster.get_reference_subtrajectory();
            const auto ref_id = trajectory.get_id_at(ref_l);
            stream << ref_id << " " << trajectory.get_index_in_trajectory(ref_l) << " " << trajectory.get_index_in_trajectory(ref_r) << "\n";

            bool first = true;
            for (const auto & [traj_l, traj_r]: cluster.get_subtrajectories()) {
                if(!first) stream << " ";
                first = false;
                const auto traj_id = trajectory.get_id_at(traj_l);
                stream << traj_id << " " << trajectory.get_index_in_trajectory(traj_l) << " " << trajectory.get_index_in_trajectory(traj_r);
            }
            stream << "\n";
        }
        stream << std::flush;
    }

    static void print_clustering_csv(const std::vector<subtrajectory_cluster_t> &pathlets, std::ostream &stream) {
        for (size_t idx = 0 ; idx < pathlets.size(); ++idx) {
            const auto& cluster = pathlets[idx];
            for (const auto & subt: cluster.get_subtrajectories()) {
                stream << ".,.,.,.," << idx << "," << subt.first << "," << subt.second << "\n";
            }
        }
    }

};


template<metric_space m_space>
class fixed_distance_clustering {

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
    using cluster_collection_t = std::vector<subtrajectory_cluster_t>;

    using efficacy_factor_t = internal::efficacy_factor_vec<distance_t>;

private:
    using k_cluster_detail = internal::k_cluster_detail<space>;

public:
    fixed_distance_clustering(const trajectory_t &trajectory,
                              distance_t sq_distance) : trajectory(trajectory),
                                                           range_search(this->trajectory),
                                                           sq_distance(sq_distance) {}

    void establish_cluster(const subtrajectory_cluster_t &cluster){
        clusters.push_back(cluster);
        k_cluster_detail::erase_points_in_cluster(trajectory, range_search, cluster);
    }

    bool find_best_cluster_rightstep(subtrajectory_cluster_t &output_cluster, rightstep_config config){
        // It is a bit hacky to modify the config here,
        // but this is the earliest point where we fixed the distance.
        config.cost_per_pathlet *= sqrt(sq_distance);
        return k_cluster_detail::find_best_cluster_at_fixed_distance_rightstep(trajectory,
                                                                                   range_search,
                                                                                   sq_distance,
                                                                                   config,
                                                                                   output_cluster);
    }

    void perform_clustering_rightstep(const rightstep_config &config) {
        std::cout << "Rightstep clustering with distance " << std::sqrt(sq_distance) << "\n";
        while (trajectory.get_actual_size() > 0) {
            std::cout << "    Remaining points: " << trajectory.get_actual_size() << "\n";
            subtrajectory_cluster_t temp_cluster;
            [[maybe_unused]]
            const auto success = find_best_cluster_rightstep(temp_cluster, config);
            assert(success);
            if (!temp_cluster.size()) break; // avoid infinite loop
            establish_cluster(temp_cluster);
        }
    }

    bool find_best_cluster_given_distance_and_length(subtrajectory_cluster_t &output_cluster, index_t pathlet_length){
        return k_cluster_detail::find_best_cluster_at_fixed_distance_and_length(trajectory,
                                                                                   range_search,
                                                                                   sq_distance,
                                                                                   output_cluster,
                                                                                   pathlet_length);
    }
    void perform_clustering_given_distance_and_length(index_t pathlet_length) {
        std::cout << "Basic clustering with distance " << std::sqrt(sq_distance) << " and length " << pathlet_length << "\n";
        while (trajectory.get_actual_size() > 0) {
            std::cout << "    Remaining points: " << trajectory.get_actual_size() << "\n";
            subtrajectory_cluster_t temp_cluster;
            const auto success = find_best_cluster_given_distance_and_length(temp_cluster, pathlet_length);
            // success is false if there all reference subtrajectories of this length contain deleted points.
            if (!success || !temp_cluster.size()) break;
            establish_cluster(temp_cluster);
        }
    }

    bool find_best_cluster_given_distance_and_size(subtrajectory_cluster_t &output_cluster, index_t target_size){
        return k_cluster_detail::find_best_cluster_at_fixed_distance_and_size(trajectory,
                                                                                   range_search,
                                                                                   sq_distance,
                                                                                   output_cluster,
                                                                                   target_size);
    }
    void perform_clustering_given_distance_and_size(index_t target_size) {
        std::cout << "Basic clustering with distance " << std::sqrt(sq_distance) << " and size " << target_size << "\n";
        while (trajectory.get_actual_size() > 0) {
            std::cout << "    Remaining points: " << trajectory.get_actual_size() << "\n";
            subtrajectory_cluster_t temp_cluster;
            const auto success = find_best_cluster_given_distance_and_size(temp_cluster, target_size);
            if (!success || !temp_cluster.size()) break;
            establish_cluster(temp_cluster);
        }
    }

    void perform_clustering_bbgll(index_t initial_min_pathlet_length, index_t cluster_scan_step) {
        index_t min_pathlet_length = initial_min_pathlet_length;
        while (trajectory.get_actual_size() > 0) {
            subtrajectory_cluster_t temp_cluster;
            const auto success = k_cluster_detail::find_best_cluster_at_fixed_distance_bbgll(trajectory,
                                                                                       range_search,
                                                                                       sq_distance,
                                                                                       temp_cluster,
                                                                                       min_pathlet_length,
                                                                                       cluster_scan_step);
            if (!success) {
                min_pathlet_length--;
            } else {
                k_cluster_detail::erase_points_in_cluster(trajectory, range_search, temp_cluster);
                clusters.push_back(temp_cluster);
            }
        }
    }

    distance_t compute_gamma(const subtrajectory_cluster_t &cluster, const efficacy_factor_t &factors){
        return k_cluster_detail::compute_gamma(trajectory, cluster, factors);
    }

    // Perform_clustering_...() always clusters all the points. Call this function afterwards
    // to drop clusters that perform worse than leaving the points unclustered.
    void drop_inefficient_clusters_means(const efficacy_factor_t &efficacy_factors){
        return drop_inefficient_clusters(efficacy_factors, k_cluster_detail::compute_efficacy_means);
    }

    void drop_inefficient_clusters_center(efficacy_factor_t efficacy_factors){
        efficacy_factors.c_2 = 0; // We heuristically assume that droping this cluster will not change the maximum Frechet distance significantly.
        return drop_inefficient_clusters(efficacy_factors, k_cluster_detail::compute_efficacy_center);
    }

    const std::vector<subtrajectory_cluster_t>& get_clusters() {
        return clusters;
    }

    std::size_t count_remaining_points() const {
        return trajectory.get_actual_size();
    }

    const distance_t get_sq_distance() const {
        return sq_distance;
    }

private:
    trajectory_t trajectory;
    range_search_t range_search;

    const distance_t sq_distance;


    cluster_collection_t clusters;

    template<typename Fun>
    void drop_inefficient_clusters(const efficacy_factor_t &efficacy_factors, const Fun &compute_efficacy){
        const auto score_empty = compute_efficacy(trajectory, cluster_collection_t{}, efficacy_factors);
        auto is_inefficient = [&, this](const subtrajectory_cluster_t &cluster){
            auto score_used = compute_efficacy(trajectory, cluster_collection_t{cluster}, efficacy_factors);
            return score_used > score_empty;
        };
        clusters.erase(std::remove_if(clusters.begin(), clusters.end(), is_inefficient), clusters.end());
    }
};

} // namespace internal

} // namespace frechet

