#include <gtest/gtest.h>
#include <utility>

#include "kdtree_range_search.h"
#include "free_space_graph.h"
#include "kcluster_algo.h"
#include "subtrajectory_routine_bbgll.h"
#include "subtrajectory_cluster.h"
#include "test_spaces.h"
#include "trajectory.h"

using namespace frechet;

using m_space = cgal_test_space_2d;
using trajectory_t = trajectory_collection<m_space>;
using range_search_t = kd_tree_range_search<m_space>;
using subtrajectory_cluster_t = subtrajectory_cluster<m_space>;
using kcluster_algo = k_clustering_algo<m_space>;

TEST(SubtrajectoryBBGLL, KMeansThreeLoops) {
    trajectory_t trajectory;
    trajectory.push_back({0.00, 0.00}, 0); // 0
    trajectory.push_back({1.00, 0.00}, 0); // 1 // First loop
    trajectory.push_back({2.00, 0.00}, 0); // 2
    trajectory.push_back({1.50, 1.00}, 0); // 3
    trajectory.push_back({1.01, 0.01}, 1); // 4
    trajectory.push_back({2.01, 0.01}, 1); // 5
    trajectory.push_back({3.00, 0.00}, 1); // 6 // Second loop
    trajectory.push_back({4.50, 0.00}, 1); // 7
    trajectory.push_back({3.50, 1.00}, 2); // 8
    trajectory.push_back({2.90, 0.01}, 3); // 9
    trajectory.push_back({4.50, 0.01}, 3); // 10
    trajectory.push_back({4.50, 2.00}, 3); // 11 // Big loop
    trajectory.push_back({1.00, 2.00}, 3); // 12
    trajectory.push_back({0.90, 0.01}, 4); // 13
    trajectory.push_back({2.00, 0.01}, 4); // 14
    trajectory.push_back({3.00, 0.02}, 4); // 15
    trajectory.push_back({4.40, 0.01}, 4); // 16
    trajectory.push_back({6.00, 0.00}, 4); // 17 // Tail
    trajectory.push_back({7.00, 0.10}, 4); // 18
    trajectory.push_back({8.00, 0.20}, 4); // 19
    
    kcluster_algo algo{trajectory, 0};

    algo.perform_means_clustering();

    algo.print_pathlets();

}

TEST(SubtrajectoryBBGLL, KMeansThreeLoopsMinPathletLength) {
    trajectory_t trajectory;
    trajectory.push_back({0.00, 0.00}, 0); // 0
    trajectory.push_back({1.00, 0.00}, 0); // 1 // First loop
    trajectory.push_back({2.00, 0.00}, 0); // 2
    trajectory.push_back({1.50, 1.00}, 0); // 3
    trajectory.push_back({1.01, 0.01}, 1); // 4
    trajectory.push_back({2.01, 0.01}, 1); // 5
    trajectory.push_back({3.00, 0.00}, 1); // 6 // Second loop
    trajectory.push_back({4.50, 0.00}, 1); // 7
    trajectory.push_back({3.50, 1.00}, 2); // 8
    trajectory.push_back({2.90, 0.01}, 3); // 9
    trajectory.push_back({4.50, 0.01}, 3); // 10
    trajectory.push_back({4.50, 2.00}, 3); // 11 // Big loop
    trajectory.push_back({1.00, 2.00}, 3); // 12
    trajectory.push_back({0.90, 0.01}, 4); // 13
    trajectory.push_back({2.00, 0.01}, 4); // 14
    trajectory.push_back({3.00, 0.02}, 4); // 15
    trajectory.push_back({4.40, 0.01}, 4); // 16
    trajectory.push_back({6.00, 0.00}, 4); // 17 // Tail
    trajectory.push_back({7.00, 0.10}, 4); // 18
    trajectory.push_back({8.00, 0.20}, 4); // 19
    
    kcluster_algo algo{trajectory, 1};

    algo.perform_means_clustering();

    algo.print_pathlets();

}

TEST(SubtrajectoryBBGLL, KCenterThreeLoops) {
    trajectory_t trajectory;
    trajectory.push_back({0.00, 0.00}, 0); // 0
    trajectory.push_back({1.00, 0.00}, 0); // 1 // First loop
    trajectory.push_back({2.00, 0.00}, 0); // 2
    trajectory.push_back({1.50, 1.00}, 0); // 3
    trajectory.push_back({1.01, 0.01}, 1); // 4
    trajectory.push_back({2.01, 0.01}, 1); // 5
    trajectory.push_back({3.00, 0.00}, 1); // 6 // Second loop
    trajectory.push_back({4.50, 0.00}, 1); // 7
    trajectory.push_back({3.50, 1.00}, 2); // 8
    trajectory.push_back({2.90, 0.01}, 3); // 9
    trajectory.push_back({4.50, 0.01}, 3); // 10
    trajectory.push_back({4.50, 2.00}, 3); // 11 // Big loop
    trajectory.push_back({1.00, 2.00}, 3); // 12
    trajectory.push_back({0.90, 0.01}, 4); // 13
    trajectory.push_back({2.00, 0.01}, 4); // 14
    trajectory.push_back({3.00, 0.02}, 4); // 15
    trajectory.push_back({4.40, 0.01}, 4); // 16
    trajectory.push_back({6.00, 0.00}, 4); // 17 // Tail
    trajectory.push_back({7.00, 0.10}, 4); // 18
    trajectory.push_back({8.00, 0.20}, 4); // 19
    
    kcluster_algo algo{trajectory, 0};

    algo.perform_center_clustering();

    algo.print_pathlets();

}

TEST(SubtrajectoryBBGLL, KCenterThreeLoopsMinPathletLength) {
    trajectory_t trajectory;
    trajectory.push_back({0.00, 0.00}, 0); // 0
    trajectory.push_back({1.00, 0.00}, 0); // 1 // First loop
    trajectory.push_back({2.00, 0.00}, 0); // 2
    trajectory.push_back({1.50, 1.00}, 0); // 3
    trajectory.push_back({1.01, 0.01}, 1); // 4
    trajectory.push_back({2.01, 0.01}, 1); // 5
    trajectory.push_back({3.00, 0.00}, 1); // 6 // Second loop
    trajectory.push_back({4.50, 0.00}, 1); // 7
    trajectory.push_back({3.50, 1.00}, 2); // 8
    trajectory.push_back({2.90, 0.01}, 3); // 9
    trajectory.push_back({4.50, 0.01}, 3); // 10
    trajectory.push_back({4.50, 2.00}, 3); // 11 // Big loop
    trajectory.push_back({1.00, 2.00}, 3); // 12
    trajectory.push_back({0.90, 0.01}, 4); // 13
    trajectory.push_back({2.00, 0.01}, 4); // 14
    trajectory.push_back({3.00, 0.02}, 4); // 15
    trajectory.push_back({4.40, 0.01}, 4); // 16
    trajectory.push_back({6.00, 0.00}, 4); // 17 // Tail
    trajectory.push_back({7.00, 0.10}, 4); // 18
    trajectory.push_back({8.00, 0.20}, 4); // 19
    
    kcluster_algo algo{trajectory, 1};

    algo.perform_center_clustering();

    algo.print_pathlets();

}

TEST(SubtrajectoryBBGLL, KCenterThreeLoopsIgnorePointClusters) {
    trajectory_t trajectory;
    trajectory.push_back({0.00, 0.00}, 0); // 0
    trajectory.push_back({1.00, 0.00}, 0); // 1 // First loop
    trajectory.push_back({2.00, 0.00}, 0); // 2
    trajectory.push_back({1.50, 1.00}, 0); // 3
    trajectory.push_back({1.01, 0.01}, 1); // 4
    trajectory.push_back({2.01, 0.01}, 1); // 5
    trajectory.push_back({3.00, 0.00}, 1); // 6 // Second loop
    trajectory.push_back({4.50, 0.00}, 1); // 7
    trajectory.push_back({3.50, 1.00}, 2); // 8
    trajectory.push_back({2.90, 0.01}, 3); // 9
    trajectory.push_back({4.50, 0.01}, 3); // 10
    trajectory.push_back({4.50, 2.00}, 3); // 11 // Big loop
    trajectory.push_back({1.00, 2.00}, 3); // 12
    trajectory.push_back({0.90, 0.01}, 4); // 13
    trajectory.push_back({2.00, 0.01}, 4); // 14
    trajectory.push_back({3.00, 0.02}, 4); // 15
    trajectory.push_back({4.40, 0.01}, 4); // 16
    trajectory.push_back({6.00, 0.00}, 4); // 17 // Tail
    trajectory.push_back({7.00, 0.10}, 4); // 18
    trajectory.push_back({8.00, 0.20}, 4); // 19
    
    kcluster_algo algo{trajectory, 0, 0.001, 9000000, false, 1, {1, 1, 1, true}};

    algo.perform_center_clustering();

    algo.print_pathlets();

}

TEST(SubtrajectoryBBGLL, KCenterThreeLoopsMinPathletLengthIgnorePointClusters) {
    trajectory_t trajectory;
    trajectory.push_back({0.00, 0.00}, 0); // 0
    trajectory.push_back({1.00, 0.00}, 0); // 1 // First loop
    trajectory.push_back({2.00, 0.00}, 0); // 2
    trajectory.push_back({1.50, 1.00}, 0); // 3
    trajectory.push_back({1.01, 0.01}, 1); // 4
    trajectory.push_back({2.01, 0.01}, 1); // 5
    trajectory.push_back({3.00, 0.00}, 1); // 6 // Second loop
    trajectory.push_back({4.50, 0.00}, 1); // 7
    trajectory.push_back({3.50, 1.00}, 2); // 8
    trajectory.push_back({2.90, 0.01}, 3); // 9
    trajectory.push_back({4.50, 0.01}, 3); // 10
    trajectory.push_back({4.50, 2.00}, 3); // 11 // Big loop
    trajectory.push_back({1.00, 2.00}, 3); // 12
    trajectory.push_back({0.90, 0.01}, 4); // 13
    trajectory.push_back({2.00, 0.01}, 4); // 14
    trajectory.push_back({3.00, 0.02}, 4); // 15
    trajectory.push_back({4.40, 0.01}, 4); // 16
    trajectory.push_back({6.00, 0.00}, 4); // 17 // Tail
    trajectory.push_back({7.00, 0.10}, 4); // 18
    trajectory.push_back({8.00, 0.20}, 4); // 19
    
    kcluster_algo algo{trajectory, 1, 0.001, 9000000, false, 1, {1, 1, 1, true}};

    algo.perform_center_clustering();

    algo.print_pathlets();

}
