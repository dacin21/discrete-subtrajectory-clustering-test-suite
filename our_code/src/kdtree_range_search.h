#pragma once

#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <vector>

#include <CGAL/Dimension.h>
#include <CGAL/Fuzzy_sphere.h>
#include <CGAL/Kd_tree.h>
#include <CGAL/Search_traits_2.h>
#include <CGAL/Search_traits_d.h>
#include <CGAL/Search_traits_adapter.h>

#include "metric_space.h"
#include "trajectory.h"
#include "utility.h"

namespace frechet {


template<CGAL_metric_space_concept m_space>
class kd_tree_range_search {

public:
    using space = m_space;
private:
    using kernel = space::kernel;
    using point_t = space::point_t;
    using property_map_t = trajectory_property_map_adapter<space>; //vector_property_map_adapter<point_t>;

    using search_traits_base = space::search_traits;
    using search_traits = CGAL::Search_traits_adapter<typename property_map_t::key_type,
                                                      property_map_t,
                                                      search_traits_base>;
    using tree_t = CGAL::Kd_tree<search_traits>;
    using splitter_t = tree_t::Splitter;

    using sphere_t = CGAL::Fuzzy_sphere<search_traits>;

    using k_neighbor_search_t = CGAL::Orthogonal_k_neighbor_search<search_traits>;
    using distance_adapter_t = k_neighbor_search_t::Distance;

    using distance_function_t = space::distance_function_t;
    using distance_t = distance_function_t::distance_t;

public:
    using trajectory_t = trajectory_collection<space>;
    using index_t = trajectory_t::index_t;
    using result_t = std::vector<typename property_map_t::key_type>;

    kd_tree_range_search(const trajectory_t &trajectory) : point_map(trajectory),
                                                              search_traits_obj(point_map),
                                                              tree(splitter_t{}, search_traits_obj) {
        for (index_t idx = 0; idx < trajectory.total_size(); ++idx) {
            if (trajectory.is_point_deleted(idx)) continue;
            tree.insert(idx);
        }
    }

    // Do not allow construction from temporary trajectories.
    kd_tree_range_search(const trajectory_t &&) = delete;
    
    result_t search(index_t index, distance_t distance) {
        auto search_distance_unsquared = std::sqrt(distance);
        auto sphere = sphere_t{point_map[index],
                                  1.1 * search_distance_unsquared,
                                  0.1 * search_distance_unsquared, // TODO: make fuzziness a parameter
                                  search_traits_obj};
        result_t result_tmp;
        tree.search(std::back_inserter(result_tmp), sphere);
        erase_points_out_of_range(result_tmp, index, distance);
        std::sort(result_tmp.begin(), result_tmp.end());
        return result_tmp;
    }

    void delete_point(index_t index) {
        // It is important to check for index equality. Otherwise, CGAL compares
        // coordinates, which fails on duplicate points and is prone to rounding errors.
        tree.remove(index, [&](index_t point_index) { return index == point_index; });
    }

    std::pair<distance_t, distance_t> nearest_and_farthest_neighbor(index_t index) {
        auto nearest_search = k_neighbor_search_t{tree, point_map[index], 2, 0, true, distance_adapter_t(point_map)};
        auto farthest_search = k_neighbor_search_t{tree, point_map[index], 1, 0, false, distance_adapter_t(point_map)};
        auto nnd = (++nearest_search.begin())->second;
        auto fnd = farthest_search.begin()->second;
        return {nnd, fnd};
    }

private:
    const property_map_t point_map;
    search_traits search_traits_obj;
    tree_t tree;

    void erase_points_out_of_range(result_t &points, index_t index, distance_t distance) {
        std::erase_if(points, [this, index, distance](const index_t p) {
            return distance_function_t()(point_map[p], point_map[index]) > distance;
        });
    }

};

}
