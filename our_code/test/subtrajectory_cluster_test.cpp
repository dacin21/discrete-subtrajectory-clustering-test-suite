#include <gtest/gtest.h>
#include <utility>

#include "kcluster_algo.h"
#include "kdtree_range_search.h"
#include "free_space_graph.h"
#include "subtrajectory_routine_bbgll.h"
#include "subtrajectory_cluster.h"
#include "test_spaces.h"
#include "trajectory.h"

using namespace frechet;

using m_space = cgal_test_space_2d;
using trajectory_t = trajectory_collection<m_space>;
using range_search_t = kd_tree_range_search<m_space>;
using subtrajectory_cluster_t = subtrajectory_cluster<m_space>;
using subtrajectory_cluster_algo = subtrajectory_clustering_bbgll<m_space>;

TEST(SubtrajectoryBBGLL, SingleLoop) {
    trajectory_t trajectory;
    trajectory.push_back({0.00, 0.00}, 0);
    trajectory.push_back({1.00, 0.00}, 0);
    trajectory.push_back({2.00, 0.00}, 0);
    trajectory.push_back({2.00, 1.00}, 0);
    trajectory.push_back({1.00, 1.00}, 0);
    trajectory.push_back({1.00, 0.01}, 0);
    trajectory.push_back({1.98, 0.01}, 0);
    trajectory.push_back({3.00, 0.00}, 0);

    range_search_t range_search{trajectory};

    subtrajectory_cluster_algo algo{trajectory, range_search};
    subtrajectory_cluster_t cluster = algo.find_longest_cluster_of_target_size_by_cardinality(2, 0.5);

    ASSERT_EQ(cluster.size(), 2);
    EXPECT_EQ(cluster[0], (std::pair<size_t, size_t>(5, 6)));
    EXPECT_EQ(cluster[1], (std::pair<size_t, size_t>(1, 2)));
    EXPECT_TRUE(cluster.contains({1,2}));
    EXPECT_TRUE(cluster.contains({5,6}));
}

TEST(SubtrajectoryBBGLL, ThreeLoops) {
    trajectory_t trajectory;
    trajectory.push_back({0.00, 0.00}, 0); // 0
    trajectory.push_back({1.00, 0.00}, 0); // 1 // First loop
    trajectory.push_back({2.00, 0.00}, 0); // 2
    trajectory.push_back({1.50, 1.00}, 0); // 3
    trajectory.push_back({1.01, 0.01}, 0); // 4
    trajectory.push_back({2.01, 0.01}, 0); // 5
    trajectory.push_back({3.00, 0.00}, 0); // 6 // Second loop
    trajectory.push_back({4.50, 0.00}, 0); // 7
    trajectory.push_back({3.50, 1.00}, 0); // 8
    trajectory.push_back({2.90, 0.01}, 0); // 9
    trajectory.push_back({4.50, 0.01}, 0); // 10
    trajectory.push_back({4.50, 2.00}, 0); // 11 // Big loop
    trajectory.push_back({1.00, 2.00}, 0); // 12
    trajectory.push_back({0.90, 0.01}, 0); // 13
    trajectory.push_back({2.00, 0.01}, 0); // 14
    trajectory.push_back({3.00, 0.02}, 0); // 15
    trajectory.push_back({4.40, 0.01}, 0); // 16
    trajectory.push_back({6.00, 0.00}, 0); // 17 // Tail
    trajectory.push_back({7.00, 0.10}, 0); // 18
    trajectory.push_back({8.00, 0.20}, 0); // 19

    // With distance 0.2:
    //      With m = 2 the longest cluster is {[4, 7], [13, 16]} with reference trajectory [4,7]
    //      With m = 3 the longest cluster is {[13,14], [4,5], [1, 2]} with reference trajectory [1,2]
    //          There is a second cluster {[15,16], [9,10], [6,7]}, i.e., in total 6 candidates taking all possible reference trajectories,
    //          but only the first is considered, as all have reference trajectory of length 1 segment.

    range_search_t range_search{trajectory};

    subtrajectory_cluster_algo algo{trajectory, range_search};

    auto cluster2 = algo.find_longest_cluster_of_target_size_by_cardinality(2, 0.2);

    ASSERT_EQ(cluster2.size(), 2);
    EXPECT_EQ(cluster2[0], (std::pair<size_t, size_t>(13, 16)));
    EXPECT_EQ(cluster2[1], (std::pair<size_t, size_t>(4, 7)));
    EXPECT_TRUE(cluster2.contains({4,7}));
    EXPECT_TRUE(cluster2.contains({13,16}));

    auto cluster3 = algo.find_longest_cluster_of_target_size_by_cardinality(3, 0.2);

    ASSERT_EQ(cluster3.size(), 3);
    EXPECT_EQ(cluster3[0], (std::pair<size_t, size_t>(13, 14)));
    EXPECT_EQ(cluster3[1], (std::pair<size_t, size_t>(4, 5)));
    EXPECT_EQ(cluster3[2], (std::pair<size_t, size_t>(1, 2)));

    EXPECT_TRUE(cluster3.contains({1,2}));
    EXPECT_TRUE(cluster3.contains({4,5}));
    EXPECT_TRUE(cluster3.contains({13,14}));

    auto cluster4 = algo.find_longest_cluster_of_target_size_by_cardinality(4, 0.2);
    ASSERT_EQ(cluster4.size(), 0);
}

TEST(SubtrajectoryBBGLL, ThreeLoopsMaxCardinality) {
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

    range_search_t range_search{trajectory};

    subtrajectory_cluster_algo algo{trajectory, range_search};

    auto cluster0 = algo.find_max_cardinality_cluster_maximizing_length(0.5, 0);
    EXPECT_EQ(cluster0.size(), 3);

    auto cluster1 = algo.find_max_cardinality_cluster_maximizing_length(0.5, 1);
    EXPECT_EQ(cluster1.size(), 3);

    auto cluster2 = algo.find_max_cardinality_cluster_maximizing_length(0.5, 2);
    EXPECT_EQ(cluster2.size(), 2);

    auto cluster3 = algo.find_max_cardinality_cluster_maximizing_length(0.5, 5);
    EXPECT_EQ(cluster3.size(), 1);

    auto cluster4 = algo.find_max_cardinality_cluster_of_fixed_length(0, 0.5);
    EXPECT_EQ(cluster4.size(), 3);

    auto cluster5 = algo.find_max_cardinality_cluster_of_fixed_length(1, 0.5);
    EXPECT_EQ(cluster5.size(), 3);

    auto cluster6 = algo.find_max_cardinality_cluster_of_fixed_length(0, 3.276);
    EXPECT_EQ(cluster6.size(), 12);

    auto cluster7 = algo.find_max_cardinality_cluster_maximizing_length(3.276, 0); // TODO: fix the bug in `find_longest_cluster_of_target_size`
    ASSERT_EQ(cluster7.size(), 12);
    EXPECT_EQ(cluster7.get_reference_subtrajectory(), (std::pair<size_t, size_t>(3,3)));
}


TEST(SubtrajectoryBBGLL, zShapedClusterWithIDs) {

    trajectory_t trajectory;
    
    trajectory.push_back({0.0, 0.0}, 0);
    trajectory.push_back({1.0, 0.0}, 0);
    trajectory.push_back({0.0, 5.0}, 0);
    trajectory.push_back({1.0, 5.0}, 0);
    trajectory.push_back({0.1, 0.1}, 1);
    trajectory.push_back({1.1, 0.1}, 1);
    trajectory.push_back({0.1, 5.1}, 2);
    trajectory.push_back({1.1, 5.1}, 2);

    range_search_t range_search{trajectory};

    subtrajectory_cluster_algo clustering{trajectory, range_search};

    auto cluster = clustering.find_longest_cluster_of_target_size_by_cardinality(1, 0.3);

    EXPECT_EQ(cluster.size(), 1);
    EXPECT_EQ(cluster[0], (std::pair<size_t,size_t>(0,3)));
    EXPECT_TRUE(cluster.contains({0,3}));
}

TEST(SubtrajectoryBBGLLWithIDs, zShapedClusterMaxCardinality) {

    trajectory_t trajectory;
    
    trajectory.push_back({0.0, 0.0}, 0);
    trajectory.push_back({1.0, 0.0}, 0);
    trajectory.push_back({0.0, 5.0}, 0);
    trajectory.push_back({1.0, 5.0}, 0);
    trajectory.push_back({0.1, 0.1}, 1);
    trajectory.push_back({1.1, 0.1}, 1);
    trajectory.push_back({0.1, 5.1}, 2);
    trajectory.push_back({1.1, 5.1}, 2);
    trajectory.push_back({0.2, 0.2}, 3);
    trajectory.push_back({1.2, 0.2}, 3);
    trajectory.push_back({0.2, 5.2}, 3);
    trajectory.push_back({1.2, 5.2}, 4);

    range_search_t range_search{trajectory};

    subtrajectory_cluster_algo clustering(trajectory, range_search);

    auto cluster = clustering.find_max_cardinality_cluster_of_fixed_length(1, 0.3);

    EXPECT_EQ(cluster.size(), 3);
    EXPECT_TRUE(cluster.contains({0,1}));
    EXPECT_TRUE(cluster.contains({4,5}));
    EXPECT_TRUE(cluster.contains({8,9}));
}

TEST(SubtrajectoryBBGLL, ThreeLoopsAfterDeletion) {
    trajectory_t trajectory;
    trajectory.push_back({0.00, 0.00}, 0); // 0
    trajectory.push_back({1.00, 0.00}, 0); // 1 // First loop
    trajectory.push_back({2.00, 0.00}, 0); // 2
    trajectory.push_back({1.50, 1.00}, 0); // 3
    trajectory.push_back({1.01, 0.01}, 0); // 4
    trajectory.push_back({2.01, 0.01}, 0); // 5
    trajectory.push_back({3.00, 0.00}, 0); // 6 // Second loop
    trajectory.push_back({4.50, 0.00}, 0); // 7
    trajectory.push_back({3.50, 1.00}, 0); // 8
    trajectory.push_back({2.90, 0.01}, 0); // 9
    trajectory.push_back({4.50, 0.01}, 0); // 10
    trajectory.push_back({4.50, 2.00}, 0); // 11 // Big loop
    trajectory.push_back({1.00, 2.00}, 0); // 12
    trajectory.push_back({0.90, 0.01}, 0); // 13
    trajectory.push_back({2.00, 0.01}, 0); // 14
    trajectory.push_back({3.00, 0.02}, 0); // 15
    trajectory.push_back({4.40, 0.01}, 0); // 16
    trajectory.push_back({6.00, 0.00}, 0); // 17 // Tail
    trajectory.push_back({7.00, 0.10}, 0); // 18
    trajectory.push_back({8.00, 0.20}, 0); // 19

    range_search_t range_search{trajectory};

    subtrajectory_cluster_algo algo{trajectory, range_search};

    // Delete some points
    trajectory.delete_point(2);
    range_search.delete_point(2);
    trajectory.delete_point(5);
    range_search.delete_point(5);
    trajectory.delete_point(14);
    range_search.delete_point(14);

    auto cluster1 = algo.find_max_cardinality_cluster_of_fixed_length(1, 0.3);
    EXPECT_TRUE(cluster1.contains({6,7}));
    EXPECT_TRUE(cluster1.contains({9,10}));
    EXPECT_TRUE(cluster1.contains({15,16}));

    auto cluster2 = algo.find_max_cardinality_cluster_maximizing_length(0.3);
    EXPECT_TRUE(cluster1.contains({6,7}));
    EXPECT_TRUE(cluster1.contains({9,10}));
    EXPECT_TRUE(cluster1.contains({15,16}));
}

TEST(SubtrajectoryBBGLL, VerySpecificThreeLoopsCase) {
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

    range_search_t range_search{trajectory};
    trajectory.delete_subtrajectory({4,7});
    trajectory.delete_subtrajectory({13, 16});
    for (size_t i: {4, 5, 6, 7, 13, 14, 15, 16}) {
        range_search.delete_point(i);
    }

    subtrajectory_cluster_algo algo{trajectory, range_search};
    auto longest_11_cluster = algo.find_longest_cluster_of_target_size_by_cardinality(11, 52.4288);
    for (size_t i = 0; i < longest_11_cluster.size() - 1; ++i) {
        for (size_t j = i+1; j < longest_11_cluster.size(); ++j) {
            EXPECT_NE(longest_11_cluster[i].first, longest_11_cluster[j].first);
            EXPECT_NE(longest_11_cluster[i].first, longest_11_cluster[j].second);
            EXPECT_NE(longest_11_cluster[i].second, longest_11_cluster[j].first);
            EXPECT_NE(longest_11_cluster[i].second, longest_11_cluster[j].second);
        }
    }

    subtrajectory_cluster_t best_cluster;
    internal::k_cluster_detail<m_space>::find_best_cluster_at_fixed_distance_bbgll(trajectory,
                                                                             range_search,
                                                                             52.4288,
                                                                             best_cluster,
                                                                             0,
                                                                             1);
    for (size_t i = 0; i < best_cluster.size() - 1; ++i) {
        for (size_t j = i+1; j < best_cluster.size(); ++j) {
            EXPECT_NE(best_cluster[i].first, best_cluster[j].first);
            EXPECT_NE(best_cluster[i].first, best_cluster[j].second);
            EXPECT_NE(best_cluster[i].second, best_cluster[j].first);
            EXPECT_NE(best_cluster[i].second, best_cluster[j].second);
        }
    }
}
