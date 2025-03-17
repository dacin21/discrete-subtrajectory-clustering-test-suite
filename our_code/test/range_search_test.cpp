#include <gtest/gtest.h>
#include <utility>

#include "kdtree_range_search.h"
#include "test_spaces.h"
#include "trajectory.h"

using namespace frechet;

using m_space = cgal_test_space_2d;
using trajectory_t = trajectory_collection<m_space>;
using range_search_t = kd_tree_range_search<m_space>;

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
    
    range_search_t search{trajectory};

    auto result_0 = search.search(0, 0.002);
    ASSERT_EQ(result_0.size(), 1);
    EXPECT_EQ(result_0[0], 0);

    auto result_1 = search.search(0, 1.5);
    ASSERT_EQ(result_1.size(), 4);
    EXPECT_EQ(result_1[0],  0);
    EXPECT_EQ(result_1[1],  1);
    EXPECT_EQ(result_1[2],  4);
    EXPECT_EQ(result_1[3], 13);

    auto result_2 = search.search(0, 100);
    ASSERT_EQ(result_2.size(), 20);
    for (size_t i = 0; i < trajectory.total_size(); ++i) {
        EXPECT_EQ(result_2[i], i);
    }

    search.delete_point(1);

    auto result_3 = search.search(0, 1.5);
    ASSERT_EQ(result_3.size(), 3);
    EXPECT_EQ(result_3[0],  0);
    EXPECT_EQ(result_3[1],  4);
    EXPECT_EQ(result_3[2], 13);

}


