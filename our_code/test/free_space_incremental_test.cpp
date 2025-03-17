#include <gtest/gtest.h>
#include <span>
#include <utility>

#include "free_space_graph_incremental.h"
#include "test_spaces.h"
#include "trajectory.h"

using namespace frechet;

using m_space = test_space_2d;
using free_space_graph_t = sparse_free_space_graph_incremental<m_space>;
using index_t = free_space_graph_t::index_t;

TEST(FreeSpaceIncremental, Example) {
    free_space_graph_t free_space(0, true);

    auto populate_column = [&](auto&&... zeros) {
        ((void)free_space.add_zero(zeros), ...);
    };
    auto query = [&](){
        return free_space.query_cluster_candidate([](index_t l, index_t r){ return r - l; },
                                                         std::numeric_limits<index_t>::max(),
                                                         std::numeric_limits<index_t>::max()
                                                         ).subtrajectories_count;
    };
    /* Construct this free space diagram:
        1 1 0 1 0 1 
        1 0 1 0 1 1
        0 0 0 1 1 1
        1 1 1 0 0 0
        1 1 0 1 1 1
        1 0 1 1 1 1
        0 1 0 0 1 0
        1 0 1 1 1 1
        0 0 0 1 0 0
        1 1 1 1 1 1
        1 0 0 0 1 1
    */

    populate_column(2, 4, 8);
    EXPECT_EQ(query(), 3);

    free_space.new_column();
    populate_column(0, 2, 3, 5, 8, 9);
    EXPECT_EQ(query(), 3);

    free_space.new_column();
    populate_column(0, 2, 4, 6, 8, 10);
    EXPECT_EQ(query(), 3);

    free_space.new_column();
    populate_column(0, 4, 7, 9);
    EXPECT_EQ(query(), 2);

    free_space.new_column();
    populate_column(2, 7, 10);
    EXPECT_EQ(query(), 2);

    free_space.new_column();
    populate_column(2, 4, 7);
    EXPECT_EQ(query(), 1);
}

