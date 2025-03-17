#include <CGAL/Dimension.h>
#include <CGAL/Simple_cartesian.h>

#include <omp.h>

#include "CLI11.hpp"

#include "free_space_graph.h"
#include "io.h"
#include "kcluster_detail.h"
#include "subtrajectory_clustering_algo.h"
#include "metric_space.h"
#include "trajectory.h"
#include "utility"
#include "validation.h"

using namespace frechet;

using space = CGAL_metric_space<CGAL::Simple_cartesian<double>, CGAL::Dimension_tag<2>>;
using free_space_graph_t = sparse_free_space_graph<space>;
using bbgll_algo = subtrajectory_clustering_bbgll<space>;
using trajectory_t = bbgll_algo::trajectory_t;
using range_search_t = bbgll_algo::range_search_t;
using index_t = bbgll_algo::index_t;
using distance_t = bbgll_algo::distance_t;
using k_cluster_detail = internal::k_cluster_detail<space>;


int main(int argc, char** argv) {
    distance_t distance;
    size_t min_cluster_size;
    std::string infilename, outfilename;

    CLI::App app{"Subroutine to compute the longest trajectory cluster, due to Buchin et al."};
    app.add_option("distance",
                   distance,
                   "Maximum allowed Frechet distance.")
        ->required();
    app.add_option("size",
                   min_cluster_size,
                   "Minimum required number of subtrajectories in the cluster")
        ->required();
    app.add_option("input",
                   infilename,
                   "The trajectory file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("output",
                   outfilename,
                   "The output file. Leave blank to use stdout.");

    CLI11_PARSE(app, argc, argv);


    internal::Timer timer;

    trajectory_t the_trajectory = read_trajectory_with_timestamps_from_file<space>(infilename);
    the_trajectory.strip_ids();

    std::cout << "Loaded trajectory file " << strip_directories(infilename) << ", got "
        << the_trajectory.total_size() << " vertices and "
        << the_trajectory.get_id_at(the_trajectory.total_size()-1) << " trajectories\n";


    range_search_t range_search(the_trajectory);
    bbgll_algo algo(the_trajectory, range_search);

    std::cout << "Initialized the bbgll algorithm\n";

    const auto cluster = algo.find_longest_cluster_of_target_size_by_cardinality(min_cluster_size, distance * distance);
    const auto reference = cluster.get_reference_subtrajectory();

    std::cout << "Found cluster of length " << (reference.second - reference.first + 1) << " covering " << cluster.size() << " subtrajectories.\n";


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
    out << "bbgll "
        << distance << " " << min_cluster_size << "\n";
    auto seconds = timer.seconds();
    out << seconds << "\n";
    std::cout << "Total time: " << seconds << "\n";

    k_cluster_detail::print_clustering_spaced(the_trajectory, {cluster}, out);

    return 0;
}

