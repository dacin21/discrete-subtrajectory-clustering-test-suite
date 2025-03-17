#include <gtest/gtest.h>
#include <utility>

#include "free_space_graph.h"
#include "subtrajectory_cluster.h"
#include "test_spaces.h"
#include "trajectory.h"

using namespace frechet;

using m_space = test_space_2d;
using free_space_graph_t = sparse_free_space_graph<m_space>;
using trajectory_t = trajectory_collection<m_space>;
using subtrajectory_cluster_t = subtrajectory_cluster<m_space>;

TEST(FreeSpace, SingleTrajectory) {
    trajectory_t trajectory;
    trajectory.push_back({0,0}, 0);
    trajectory.push_back({0,0}, 0);
    trajectory.push_back({0,0}, 0);
    free_space_graph_t free_space(0);

    free_space.add_zero(0);
    free_space.new_column();
    free_space.add_zero(1);
    free_space.new_column();
    free_space.add_zero(2);

    trajectory_t dummy;
    subtrajectory_cluster_t cluster;
    free_space.query_subtrajectories_respecting_ids(trajectory, cluster);

    EXPECT_EQ(cluster.size(), 1);
    EXPECT_EQ(cluster[0], (std::pair<size_t, size_t>(0, 2)));
    EXPECT_EQ(cluster.get_reference_subtrajectory(), (std::pair<size_t, size_t>(0, 2)));
}

class FreeSpaceExample : public ::testing::Test {

protected:
    FreeSpaceExample() : free_space(0) {
        /* Construct this free space diagram:
            1 0 0
            0 0 0
            1 1 0
            1 0 0
            0 1 1
        */

        trajectory.push_back({0,0}, 0);
        trajectory.push_back({0,0}, 0);
        trajectory.push_back({0,0}, 0);
        trajectory.push_back({0,0}, 0);
        trajectory.push_back({0,0}, 0);

        free_space.add_zero(0);
        free_space.add_zero(3);
        free_space.new_column();
        free_space.add_zero(1);
        free_space.add_zero(3);
        free_space.add_zero(4);
        free_space.new_column();
        free_space.add_zero(1);
        free_space.add_zero(2);
        free_space.add_zero(3);
        free_space.add_zero(4);

        free_space.query_subtrajectories_respecting_ids(trajectory, cluster);
    }

    void add_and_delete() {
        /* Transform this free space diagram
            1 0 0
            0 0 0
            1 1 0
            1 0 0
            0 1 1
           into this diagram
            0 0 0 0
            0 0 0 1
            1 0 0 1
            0 0 1 0
            1 1 0 1
           with left column 1 and right column 4
        */
        free_space.new_column();
        free_space.add_zero(0);
        free_space.add_zero(2);
        free_space.add_zero(4);
        free_space.delete_column();
        free_space.new_column();
        free_space.add_zero(1);
        free_space.add_zero(4);
        cluster.clear();
    }

    trajectory_t trajectory;
    free_space_graph_t free_space;
    subtrajectory_cluster_t cluster;

};

TEST_F(FreeSpaceExample, SimpleFreeSpace) {
    ASSERT_EQ(cluster.size(), 2);
    EXPECT_EQ(cluster[0], (std::pair<size_t, size_t>(3, 4)));
    EXPECT_EQ(cluster[1], (std::pair<size_t, size_t>(0, 2)));
    EXPECT_EQ(cluster.get_reference_subtrajectory(), (std::pair<size_t, size_t>(0, 2)));
}

TEST_F(FreeSpaceExample, DeletingColumns) {
    add_and_delete();
    free_space.query_subtrajectories_respecting_ids(trajectory, cluster);

    ASSERT_EQ(cluster.size(), 1);
    EXPECT_EQ(cluster[0], (std::pair<size_t, size_t>(1, 4)));
    EXPECT_EQ(cluster.get_reference_subtrajectory(), (std::pair<size_t, size_t>(1, 4)));
}



//
// Testing subtrajectories respecting input trajectories
//

TEST(FreeSpaceRespectingIDs, DoesNotIgnoreBoundaries) {
    trajectory_t trajectory;
    
    trajectory.push_back({0.0, 0.0}, 0);
    trajectory.push_back({1.0, 0.0}, 0);
    trajectory.push_back({0.0, 5.0}, 0);
    trajectory.push_back({1.0, 5.0}, 0);
    trajectory.push_back({0.1, 0.1}, 1);
    trajectory.push_back({1.1, 0.1}, 1);
    trajectory.push_back({0.1, 5.1}, 2);
    trajectory.push_back({1.1, 5.1}, 2);

    /*
     * The free space for this at distance 0.2 looks like
     *
     *    0 1 2 3 4 5 6 7       trajectory id
     *
     * 7  1 1 1 0 1 1 1 0       2
     * 6  1 1 0 1 1 1 0 1       2
     * 5  1 0 1 1 1 0 1 1       1
     * 4  0 1 1 1 0 1 1 1       1
     * 3  1 1 1 0 1 1 1 0       0
     * 2  1 1 0 1 1 1 0 1       0
     * 1  1 0 1 1 1 0 1 1       0
     * 0  0 1 1 1 0 1 1 1       0
     *
     * When querying subtrajectories over reference [0,3],
     * we should only be getting the reference back when respecting ids,
     * even though [4,7] would be at distance < 0.2 when ignoring ids.
     */

    free_space_graph_t free_space{0};

    free_space.add_zero(0);
    free_space.add_zero(4);
    free_space.new_column();
    free_space.add_zero(1);
    free_space.add_zero(5);
    free_space.new_column();
    free_space.add_zero(2);
    free_space.add_zero(6);
    free_space.new_column();
    free_space.add_zero(3);
    free_space.add_zero(7);

    subtrajectory_cluster_t result;

    free_space.query_subtrajectories_respecting_ids(trajectory, result);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], (std::pair<size_t, size_t>{0, 3}));
    
}

TEST(FreeSpaceRespectingIDs, DoesNotIgnoreDeletedVertices) {
    trajectory_t trajectory;
    
    trajectory.push_back({0.0, 0.0}, 0);  // 0
    trajectory.push_back({1.0, 0.0}, 0);
    trajectory.push_back({0.0, 5.0}, 0);
    trajectory.push_back({1.0, 5.0}, 0);
    trajectory.push_back({0.1, 0.1}, 9);
    trajectory.push_back({1.1, 0.1}, 9); // 5
    trajectory.push_back({0.1, 5.1}, 9);
    trajectory.push_back({1.1, 5.1}, 9);
    trajectory.push_back({0.2, 0.2}, 1);
    trajectory.push_back({1.2, 0.2}, 1);
    trajectory.push_back({0.2, 5.2}, 1); // 10
    trajectory.push_back({1.2, 5.2}, 1);

    trajectory.delete_subtrajectory({4, 7});

    /*
     * The free space for this at distance 0.3 looks like
     *
     *     0 1 2 3 4 5 6 7 ...   trajectory id
     *
     * 11  1 1 1 0 1 1 1 0       1
     * 10  1 1 0 1 1 1 0 1       1
     *  9  1 0 1 1 1 0 1 1       1
     *  8  0 1 1 1 0 1 1 1       1
     *  7  1 1 1 0 1 1 1 0       -1
     *  6  1 1 0 1 1 1 0 1       -1
     *  5  1 0 1 1 1 0 1 1       -1
     *  4  0 1 1 1 0 1 1 1       -1
     *  3  1 1 1 0 1 1 1 0       0
     *  2  1 1 0 1 1 1 0 1       0
     *  1  1 0 1 1 1 0 1 1       0
     *  0  0 1 1 1 0 1 1 1       0
     *
     * Trajectory id `-1` indicates a deleted vertices and these should be skipped,
     * so with reference trajectory [0,3] we should get back only the reference trajectory
     * and the trajectory [8,11] when querying for subtrajectories.
     */

    free_space_graph_t free_space{0};

    free_space.add_zero(0);
    free_space.add_zero(4);
    free_space.add_zero(8);
    free_space.new_column();
    free_space.add_zero(1);
    free_space.add_zero(5);
    free_space.add_zero(9);
    free_space.new_column();
    free_space.add_zero(2);
    free_space.add_zero(6);
    free_space.add_zero(10);
    free_space.new_column();
    free_space.add_zero(3);
    free_space.add_zero(7);
    free_space.add_zero(11);

    subtrajectory_cluster_t result;

    free_space.query_subtrajectories_respecting_ids(trajectory, result);

    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], (std::pair<size_t, size_t>{8, 11}));
    EXPECT_EQ(result[1], (std::pair<size_t, size_t>{0, 3}));
    
}

TEST(FreeSpace, EmptyReferenceWithIDs) {

    trajectory_t trajectory;
    trajectory.push_back({0.0, 0.0}, 0);    // 0
    trajectory.push_back({0.1, 0.0}, 0);    // 1
    trajectory.push_back({0.2, 0.0}, 0);    // 2
    trajectory.push_back({0.3, 0.0}, 0);    // 3
    trajectory.push_back({4.0, 0.0}, 0);    // 4
    trajectory.push_back({0.5, 0.0}, 0);    // 5
    trajectory.push_back({5.0, 0.0}, 0);    // 6

    free_space_graph_t free_space{2};
    free_space.add_zero(0);
    free_space.add_zero(1);
    free_space.add_zero(2);
    free_space.add_zero(3);
    free_space.add_zero(5);

    subtrajectory_cluster_t result;
    free_space.query_subtrajectories_respecting_ids(trajectory, result);

    EXPECT_EQ(result.size(), 5);
    EXPECT_EQ(result.get_reference_subtrajectory(), (std::pair<size_t, size_t>{2,2}));
    EXPECT_TRUE(result.contains({0,0}));
    EXPECT_TRUE(result.contains({1,1}));
    EXPECT_TRUE(result.contains({2,2}));
    EXPECT_TRUE(result.contains({3,3}));
    EXPECT_TRUE(result.contains({5,5}));
}

TEST(FreeSpace, ThreeLoopsNoOverlap) {
    trajectory_t trajectory;
    trajectory.push_back({0.00, 0.00}, 0); // 0
    trajectory.push_back({1.00, 0.00}, 0); // 1 // First loop
    trajectory.push_back({2.00, 0.00}, 0); // 2
    trajectory.push_back({1.50, 1.00}, 0); // 3
    trajectory.push_back({1.01, 0.01}, 8); // 4
    trajectory.push_back({2.01, 0.01}, 8); // 5
    trajectory.push_back({3.00, 0.00}, 8); // 6 // Second loop
    trajectory.push_back({4.50, 0.00}, 8); // 7
    trajectory.push_back({3.50, 1.00}, 2); // 8
    trajectory.push_back({2.90, 0.01}, 3); // 9
    trajectory.push_back({4.50, 0.01}, 3); // 10
    trajectory.push_back({4.50, 2.00}, 3); // 11 // Big loop
    trajectory.push_back({1.00, 2.00}, 3); // 12
    trajectory.push_back({0.90, 0.01}, 9); // 13
    trajectory.push_back({2.00, 0.01}, 9); // 14
    trajectory.push_back({3.00, 0.02}, 9); // 15
    trajectory.push_back({4.40, 0.01}, 9); // 16
    trajectory.push_back({6.00, 0.00}, 4); // 17 // Tail
    trajectory.push_back({7.00, 0.10}, 4); // 18
    trajectory.push_back({8.00, 0.20}, 4); // 19
    
    trajectory.delete_subtrajectory({4, 7});
    trajectory.delete_subtrajectory({13, 16});

    free_space_graph_t free_space{0};
    free_space.add_zero(0);
    free_space.add_zero(1);
    free_space.add_zero(2);
    free_space.add_zero(3);
    free_space.add_zero(8);
    free_space.add_zero(9);
    free_space.add_zero(10);
    free_space.add_zero(11);
    free_space.add_zero(12);
    free_space.add_zero(17);
    free_space.add_zero(18);
    free_space.new_column();
    free_space.add_zero(0);
    free_space.add_zero(1);
    free_space.add_zero(2);
    free_space.add_zero(3);
    free_space.add_zero(8);
    free_space.add_zero(9);
    free_space.add_zero(10);
    free_space.add_zero(11);
    free_space.add_zero(12);
    free_space.add_zero(17);
    free_space.add_zero(18);
    free_space.add_zero(19);

    subtrajectory_cluster_t cluster;
    free_space.query_subtrajectories_respecting_ids(trajectory, cluster);
    for (size_t i = 0; i < cluster.size() - 1; ++i) {
        for (size_t j = i+1; j < cluster.size(); ++j) {
            EXPECT_NE(cluster[i].first, cluster[j].first);
            EXPECT_NE(cluster[i].first, cluster[j].second);
            EXPECT_NE(cluster[i].second, cluster[j].first);
            EXPECT_NE(cluster[i].second, cluster[j].second);
        }
    }
}
