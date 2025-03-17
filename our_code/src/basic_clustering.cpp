#include <CGAL/Dimension.h>
#include <CGAL/Simple_cartesian.h>

#include <omp.h>

#include "CLI11.hpp"

#include "free_space_graph.h"
#include "io.h"
#include "basic_clustering_algo.h"
#include "metric_space.h"
#include "trajectory.h"
#include "utility"
#include "validation.h"

using namespace frechet;

using space = CGAL_metric_space<CGAL::Simple_cartesian<double>, CGAL::Dimension_tag<2>>;
using free_space_graph_t = sparse_free_space_graph<space>;
using bbgll_algo = subtrajectory_clustering_bbgll<space>;
using basic_cluster_algo = basic_clustering_algo<space>;
using trajectory_t = trajectory_collection<space>;
using validation_t = validation<space>;
using index_t = basic_cluster_algo::index_t;
using distance_t = basic_cluster_algo::distance_t;

enum class cluster_mode {
    means = 0,
    center = 1
};


int main(int argc, char** argv) {
    std::array<distance_t, 2> distance_limits{-1, -1}; // negative number -> compute the global minimum / maximum distance and use that.
    std::array<distance_t, 3> efficacy_factors{1, 0.005, 1};
    int target_length = -1, target_size = -1;
    std::string infilename, outfilename;
    cluster_mode mode;

    CLI::App app{"Basic subtrajectory clustering via the bbgll subroutine."};
    app.add_option("-d,--distance_limit",
                   distance_limits,
                   "The minimum and maximum distance for which to compute clusters. Set to -1 or omit to use the global minimum or maximum distance instead.");
    app.add_option("-c,--c",
                   efficacy_factors,
                   "Factors for computing the efficacy");
    const auto length_option = app.add_option("-l,--length",
                 target_length,
                 "The target cluster length in the bbgll subroutine.");
    app.add_option("-s,--size",
                 target_size,
                 "The target cluster size in the bbgll subrountine. -l <length> and -s <size> are mutually exclusive.")
        ->excludes(length_option);
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

    omp_set_num_threads(1);


    internal::Timer timer;

    trajectory_t the_trajectory = read_trajectory_with_timestamps_from_file<space>(infilename);

    std::cout << "Loaded trajectory file " << strip_directories(infilename) << ", got "
        << the_trajectory.total_size() << " vertices and "
        << the_trajectory.get_id_at(the_trajectory.total_size()-1) << " trajectories\n";

    assert(the_trajectory.is_sorted_by_trajectory_id());

    std::optional<index_t> target_length_opt, target_size_opt;
    if(target_size > 0) {
        target_size_opt = target_size;
    }
    if(target_length > 0) {
        target_length_opt = target_length;
    }

    basic_cluster_algo clustering_algo{the_trajectory,
                                   distance_limits[0],
                                   distance_limits[1],
                                   target_length_opt,
                                   target_size_opt,
                                   {efficacy_factors[0],
                                    efficacy_factors[1],
                                    efficacy_factors[2],
                                    false,
                                   }
                                   };

    std::cout << "Initialized the basic clustering algorithm\n";

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
    out << "basic_" << (target_length ? "length" : "size") << " "
        << efficacy_factors[0] << " " << efficacy_factors[1] << " " << efficacy_factors[2] << " "
        << distance_limits[0] << " " << distance_limits[1] << " "
        << target_length << " " << target_size << "\n";
    auto seconds = timer.seconds();
    out << seconds << "\n";
    std::cout << "Total time: " << seconds << "\n";

    clustering_algo.print_clustering_spaced(the_trajectory, out);

    validation_t::validate_no_overlap(clustering_algo.get_clusters());

    return 0;
}

