#include <CGAL/Dimension.h>
#include <CGAL/Simple_cartesian.h>

#include <omp.h>

#include "CLI11.hpp"

#include "free_space_graph.h"
#include "io.h"
#include "subtrajectory_clustering_algo.h"
#include "metric_space.h"
#include "trajectory.h"
#include "utility"
#include "validation.h"

using namespace frechet;

using space = CGAL_metric_space<CGAL::Simple_cartesian<double>, CGAL::Dimension_tag<2>>;
using free_space_graph_t = sparse_free_space_graph<space>;
using bbgll_algo = subtrajectory_clustering_bbgll<space>;
using subtrajectory_cluster_algo = subtrajectory_clustering_algo<space>;
using trajectory_t = trajectory_collection<space>;
using validation_t = validation<space>;
using index_t = subtrajectory_cluster_algo::index_t;
using distance_t = subtrajectory_cluster_algo::distance_t;

enum class cluster_mode {
    means = 0,
    center = 1
};


int main(int argc, char** argv) {
    std::array<distance_t, 2> distance_limits{-1, -1}; // negative number -> compute the global minimum / maximum distance and use that.
    std::array<distance_t, 3> efficacy_factors{1, 0.005, 1};     // These default weights correspond to values used by Agarwal et al. (PODS'18)
    bool ignore_point_clusters_in_efficacy = false;
    rightstep_config config;
    int max_threads = 1;
    config.tree_intervals_only = true;
    std::string infilename, outfilename;
    cluster_mode mode;

    CLI::App app{"Subtrajectory clustering via rightstep only."};
    app.add_option("-d,--distance_limit",
                   distance_limits,
                   "The minimum and maximum distance for which to compute clusters. Set to -1 or omit to use the global minimum or maximum distance instead.");
    app.add_option("-c,--c",
                   efficacy_factors,
                   "Factors for computing the efficacy");
    app.add_flag("-p,--ignore_point_clusters",
                 ignore_point_clusters_in_efficacy,
                 "Treat points in point clusters as unclustered when computing efficacy");
    app.add_flag("!-a,!--all_intervals",
                 config.tree_intervals_only,
                 "Consider all intervals. Better efficacy at the cost of being much slower.");
    app.add_flag<int>("-t,--threads",
                 max_threads,
                 "Maximum number of threads to use. Single-threaded by default.")
        ->default_val(1)
        ->expected(1, 999);
    app.add_option<double>("-s,--simplify",
                   config.curve_simplification_factor,
                   "Relative threshold to use curve simplification (0 = never). Has to be < 1. Default value is 0.2 .")
        ->default_val(0.2)
        ->expected(0.0, 1.0);
    app.add_option("-m,--mode",
                   mode,
                   "Which clustering to use (0 = means, 1 = center)")
        ->default_val(1);
    app.add_option("input",
                   infilename,
                   "The trajectory file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("output",
                   outfilename,
                   "The output file. Leave blank to use stdout.");

    CLI11_PARSE(app, argc, argv);

    omp_set_num_threads(max_threads);
    std::cout << "Using up to " << max_threads << " threads.\n";

    config.prefer_small_subtrajectories = (mode == cluster_mode::center);

    internal::Timer timer;

    trajectory_t the_trajectory = read_trajectory_with_timestamps_from_file<space>(infilename);

    std::cout << "Loaded trajectory file " << strip_directories(infilename) << ", got "
        << the_trajectory.total_size() << " vertices and "
        << the_trajectory.get_id_at(the_trajectory.total_size()-1) << " trajectories\n";

    assert(the_trajectory.is_sorted_by_trajectory_id());

    subtrajectory_cluster_algo clustering_algo{the_trajectory,
                                   distance_limits[0],
                                   distance_limits[1],
                                   {efficacy_factors[0],
                                    efficacy_factors[1],
                                    efficacy_factors[2],
                                    ignore_point_clusters_in_efficacy},
                                   config};

    std::cout << "Initialized the k-cluster algorithm\n";

    if (mode == cluster_mode::means) {
        clustering_algo.perform_means_clustering();
        auto eff = clustering_algo.compute_means_efficacy();
        std::cout << " Efficacy: " << eff << "\n";
    } else if (mode == cluster_mode::center) {
        clustering_algo.perform_center_clustering();
        auto eff = clustering_algo.compute_center_efficacy();
        std::cout << " Efficacy: " << eff << "\n";
    }

    std::ofstream outfile;
    auto &out = [&]() -> std::ostream& {
        if(outfilename.empty()) {
            return std::cout;
        }
        outfile.open(outfilename);
        assert(outfile);
        return outfile;
    }();
    // Header
    //   <data set name>
    //   <algorithm name> <param_1> ... <param_k>
    //   <running time>
    out << strip_directories(infilename) << "\n";
    out << "rightstep "
        << efficacy_factors[0] << " " << efficacy_factors[1] << " " << efficacy_factors[2] << " "
        << distance_limits[0] << " " << distance_limits[1] << "\n";
    auto seconds = timer.seconds();
    out << seconds << "\n";
    std::cout << "Total time: " << seconds << "\n";

    clustering_algo.print_clustering_spaced(the_trajectory, out);

    validation_t::validate_no_overlap(clustering_algo.get_clusters());

    return 0;
}

