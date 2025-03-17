// Code to benchmark MoveTK's trajectory clustering algorithm.
// This file is not part of MoveTK.

#include <cassert>
#include <sstream>

#include "movetk/utils/GeometryBackendTraits.h"
#include "movetk/Clustering.h"
#include "movetk/ds/FreeSpaceDiagram.h"
#include "movetk/geom/GeometryInterface.h"
#include "movetk/metric/Norm.h"
#include "movetk/utils/Iterators.h"
#include "movetk/utils/TrajectoryUtils.h"

using MovetkGeometryKernel = typename GeometryKernel::MovetkGeometryKernel;
using Norm = movetk::metric::FiniteNorm<MovetkGeometryKernel, 2>;
using NT = typename MovetkGeometryKernel::NT;
using MovetkPoint = typename MovetkGeometryKernel::MovetkPoint;
using IntersectionTraits =
	movetk::geom::IntersectionTraits<MovetkGeometryKernel, Norm, movetk::geom::sphere_segment_intersection_tag>;
using FreeSpaceCellTraits = movetk::ds::FreeSpaceCellTraits<IntersectionTraits>;
using FreeSpaceCell = movetk::ds::FreeSpaceCell<FreeSpaceCellTraits>;
using FreeSpaceDiagramTraits = movetk::ds::FreeSpaceDiagramTraits<FreeSpaceCell>;
using FreeSpaceDiagram = movetk::ds::FreeSpaceDiagram<FreeSpaceDiagramTraits>;
using ClusteringTraits = movetk::clustering::ClusteringTraits<FreeSpaceDiagram>;
using SubTrajectoryClustering = movetk::clustering::SubTrajectoryClustering<ClusteringTraits>;
movetk::geom::MakePoint<MovetkGeometryKernel> make_point;

template<typename... Args>
void error(Args... args){
	(std::cerr << ... << args);
	std::cerr << "\n";
	exit(1);
}

template<typename T>
T parse(char* arg){
	std::stringstream ss(arg);
	T ret;
	ss >> ret;
	assert(ss);
	return ret;
}

std::vector<MovetkPoint> parse_input(char* input_path){
	std::ifstream infile(input_path);
	if (!infile) error("failed to open input file: ", input_path);

	std::vector<MovetkPoint> trajectory;
	std::string line;
	while (std::getline(infile, line)) {
		if (line.empty()) continue;
		std::stringstream ss(line);
		int id, time;
		double lat, lon;
		ss >> id >> time >> lat >> lon;

		if (!ss) error("failed to parse line: ", line);
		trajectory.push_back(make_point({lat, lon}));
	}
	return trajectory;
}


int main(int argc, char** argv){
	if (argc != 5) {
		std::cout << "Usage: " << argv[0] << " input output distance minlength\n";
		return 1;
	}
	const auto input_path = argv[1];
	const auto output_path = argv[2];
	const auto distance = parse<double>(argv[3]);
	const auto minlength = parse<size_t>(argv[4]);

	const auto trajectory = parse_input(argv[1]);

	SubTrajectoryClustering clustering(std::begin(trajectory), std::end(trajectory), minlength, distance);

	std::ofstream outfile(output_path);
	if (!outfile) error("failed to open output file: ", output_path);
	outfile << input_path << "\n";
	outfile << "movetk " << argv[3] << " " << argv[4] << "\n";
	outfile << time << "\n";

	const auto reference_pathlet = clustering.get_subtrajectory_indices();

	outfile << reference_pathlet.first << " " << reference_pathlet.second << "\n";
	outfile << clustering.get_cluster_size() << "\n";
}
