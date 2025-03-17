#include <gtest/gtest.h>

#include "frechet_distance.h"
#include "test_spaces.h"
#include "trajectory.h"

using namespace frechet;

using m_space = test_space_2d;
using trajectory_t = trajectory_collection<m_space>;

TEST(FrechetDistance, One) {
    trajectory_t trajectory;
    trajectory.push_back({0,0}, 0);
    trajectory.push_back({1,0}, 0);
    trajectory.push_back({2,0}, 0);
    trajectory.push_back({3,0}, 0);
    trajectory.push_back({0,1}, 0);
    trajectory.push_back({1,1}, 0);
    trajectory.push_back({2,1}, 0);
    trajectory.push_back({3,1}, 0);

    const auto result = frechet_distance<m_space>::compute(trajectory, {0,3}, {4,7});
    const auto result_light = frechet_distance<m_space>::compute_light(trajectory, {0,3}, {4,7});
    EXPECT_DOUBLE_EQ(result, 1);
    EXPECT_DOUBLE_EQ(result, result_light);
}

TEST(FrechetDistance, Two) {
    trajectory_t trajectory;
    trajectory.push_back({0,0}, 0);
    trajectory.push_back({1,0}, 0);
    trajectory.push_back({2,0}, 0);
    trajectory.push_back({3,0}, 0);
    trajectory.push_back({0,2}, 0);
    trajectory.push_back({1,1}, 0);
    trajectory.push_back({2,2}, 0);
    trajectory.push_back({3,1}, 0);

    const auto result = frechet_distance<m_space>::compute(trajectory, {0,3}, {4,7});
    const auto result_light = frechet_distance<m_space>::compute_light(trajectory, {0,3}, {4,7});
    EXPECT_DOUBLE_EQ(result, 2);
    EXPECT_DOUBLE_EQ(result, result_light);
}

TEST(FrechetDistance, LightEqualsHeavy) {
    trajectory_t trajectory;
    trajectory.push_back({1.2, 3.2}, 0);
    trajectory.push_back({4.4, 6.4}, 0);
    trajectory.push_back({7.1, 2.5}, 0);
    trajectory.push_back({2.8, 3.7}, 0);
    trajectory.push_back({5.4, 4.2}, 0);
    trajectory.push_back({4.5, 7.6}, 0);
    trajectory.push_back({7.8, 2.8}, 0);
    trajectory.push_back({2.2, 8.9}, 0);
    trajectory.push_back({4.9, 6.3}, 0);
    trajectory.push_back({8.4, 5.0}, 0);
    trajectory.push_back({4.5, 8.2}, 0);
    trajectory.push_back({6.8, 4.3}, 0);

    const auto result_heavy = frechet_distance<m_space>::compute(trajectory, {2, 6}, {8, 10});
    const auto result_light = frechet_distance<m_space>::compute_light(trajectory, {2, 6}, {8, 10});
    EXPECT_DOUBLE_EQ(result_heavy, result_light);
}
