#include <CGAL/Dimension.h>
#include <CGAL/Simple_cartesian.h>

#include "CLI11.hpp"

#include "free_space_graph.h"
#include "io.h"
#include "kcluster_algo.h"
#include "metric_space.h"
#include "subtrajectory_routine_bbgll.h"
#include "trajectory.h"
#include "validation.h"

using namespace frechet;

using space = CGAL_metric_space<CGAL::Simple_cartesian<double>, CGAL::Dimension_tag<2>>;
using free_space_graph_t = sparse_free_space_graph<space>;
using bbgll_algo = subtrajectory_clustering_bbgll<space>;
using k_cluster_algo = k_clustering_algo<space>;
using trajectory_t = trajectory_collection<space>;
using validation_t = validation<space>;
using index_t = k_cluster_algo::index_t;
using distance_t = k_cluster_algo::distance_t;

enum class cluster_mode {
    means = 0,
    center = 1
};

int main(int argc, char** argv) {
    index_t min_pathlet_length = 0;
    std::array<distance_t, 2> distance_limits{0.5, 2};
    bool kmeans_bias_clustering = false;
    index_t cluster_scan_step = 1;
    std::array<distance_t, 3> efficacy_factors{1, 0.005, 1};     // These default weights correspond to values used by Agarwal et al. (PODS'18)
    bool ignore_point_clusters_in_efficacy = false;
    std::string filename;
    cluster_mode mode;

    CLI::App app{"Compute a clustering"};
    app.add_option("-l,--pathlet_length",
                   min_pathlet_length,
                   "Initial minimum length of pathlets")
        ->check(CLI::NonNegativeNumber);
    app.add_option("-d,--distance_limit",
                   distance_limits,
                   "The minimum and maximum distance for which to compute clusters");
    app.add_flag("-b,--kmeans_bias",
                 kmeans_bias_clustering,
                 "Bias the score in k-means clustering against point clusters and those of size 1");
    app.add_option("-s,--step",
                   cluster_scan_step,
                   "At how many sizes to compute the longest cluster when finding the best cluster at a fixed distance.");
    app.add_option("-c,--c",
                   efficacy_factors,
                   "Factors for computing the efficacy");
    app.add_flag("-p,--ignore_point_clusters",
                 ignore_point_clusters_in_efficacy,
                 "Treat points in point clusters as unclustered when computing efficacy");
    app.add_option("-m,--mode",
                   mode,
                   "Which clustering to use (0 = means, 1 = center)")
        ->default_val(0);
    app.add_option("input",
                   filename,
                   "The trajectory file")
        ->required()
        ->check(CLI::ExistingFile);

    CLI11_PARSE(app, argc, argv);

    trajectory_t the_trajectory = read_trajectory_from_file<space>(filename);

    std::cout << "Loaded the trajectory " << strip_directories(filename) << "\n";

    k_cluster_algo clustering_algo{the_trajectory,
                                   min_pathlet_length,
                                   distance_limits[0],
                                   distance_limits[1],
                                   kmeans_bias_clustering,
                                   cluster_scan_step,
                                   {efficacy_factors[0],
                                    efficacy_factors[1],
                                    efficacy_factors[2],
                                    ignore_point_clusters_in_efficacy}};

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


    clustering_algo.print_clustering_csv(std::cout);

    validation_t::validate_no_overlap(clustering_algo.get_clusters());

    return 0;
}
