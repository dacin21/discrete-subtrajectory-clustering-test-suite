#pragma once

#include <unordered_map>

#include <boost/pending/disjoint_sets.hpp>
#include <boost/property_map/property_map.hpp>

namespace lost {

template<typename K, typename V>
using default_map_template = std::unordered_map<K, V>;

template<typename T, template<typename K, typename V> typename map_type = default_map_template>
class union_find {

private:
    using rank_map_t = map_type<T, size_t>;
    using parent_map_t = map_type<T, T>;

    using rank_pmap_t = boost::associative_property_map<rank_map_t>;
    using parent_pmap_t = boost::associative_property_map<parent_map_t>;

    using disjoint_sets_t = boost::disjoint_sets<rank_pmap_t, parent_pmap_t>;

    using set_map_t = default_map_template<T, std::vector<T>>;

    struct set_iterator_pair {

        set_iterator_pair(const set_map_t& map) : set_map(map) {}
        
        set_map_t::const_iterator begin() const {
            return set_map.begin();
        }

        set_map_t::const_iterator end() const {
            return set_map.end();
        }

    private:
        const set_map_t& set_map;

    };

public:

    union_find() : rank_pmap(rank_map),
                   parent_pmap(parent_map),
                   disjoint_sets(rank_pmap, parent_pmap) {}

    void make_set(T t) {
        disjoint_sets.make_set(t);
        elements.push_back(t);
    }

    void union_set(T a, T b) {
        disjoint_sets.union_set(a, b);
    }

    std::size_t number_of_sets() {
        return disjoint_sets.count_sets(elements.begin(), elements.end());
    }

    set_iterator_pair extract_sets() {
        sets.clear();
        for (const auto &elem: elements) {
            auto repr = disjoint_sets.find_set(elem);
            if (sets.contains(repr)) {
                sets[repr].push_back(elem);
            } else {
                sets[repr] = {elem};
            }
        }
        return {sets};
    }

private:
    std::vector<T> elements;

    rank_map_t rank_map;
    rank_pmap_t rank_pmap;

    parent_map_t parent_map;
    parent_pmap_t parent_pmap;

    disjoint_sets_t disjoint_sets;

    set_map_t sets;

};

} // End of namespace `lost`
