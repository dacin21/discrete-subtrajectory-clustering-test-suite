#pragma once

#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "trajectory.h"

namespace frechet {

inline void write_key_value(std::ostream &stream,
                     const std::string &name,
                     const auto &value,
                     std::string separator = ": ") {
    stream << name << separator << value << "\n";
}

// Remove the directories from a file-path.
// TODO: this is horrible, since it makes all sorts of assumptions. Should really use the filesystem library, if possible.
inline std::string strip_directories(const std::string &path) {
    auto last_sep_pos = path.find_last_of("/");
    if (last_sep_pos == std::string::npos) {
        return "";
    }
    return path.substr(last_sep_pos + 1);
}

// Read a trajectory from the file with path `filename` into a vector of points of type `P`.
// The input file is expected to be a list of points, each point on a single line,
// with each line being of the form `x y t`.
// Here, `x` and `y` are coordinates of the vertices; `t` is the trajectory id.
template<metric_space m_space>
trajectory_collection<m_space> read_trajectory_from_file(const std::string &filename) {
    std::ifstream input_stream(filename);
    if (!input_stream.is_open()) {
        std::cerr << "Failed to open input file " << filename << "\n";
        exit(1); // TODO: prettier error handling?
    }
    trajectory_collection<m_space> result;
    double x, y;
    int id;
    while (input_stream >> x >> y >> id) {
        result.push_back({x, y}, id);
    }
    return result;
}

// Read a trajectory from the file with path `filename` into a vector of points of type `P`.
// The input file is expected to be a list of points, each point on a single line,
// with each line being of the form `id t x y`.
// Here, `x` and `y` are coordinates of the vertices; `id` is the trajectory id;
// Points are ordered by the `t`, the timestamp.
template<metric_space m_space>
trajectory_collection<m_space> read_trajectory_with_timestamps_from_file(const std::string &filename) {
    std::ifstream input_stream(filename);
    if (!input_stream.is_open()) {
        std::cerr << "Failed to open input file " << filename << "\n";
        exit(1); // TODO: prettier error handling?
    }
    trajectory_collection<m_space> result;

    int prev_id = -1;
    [[maybe_unused]] double prev_t = 0.0;

    double x, y, t;
    int id;
    while (input_stream >> id >> t >> x >> y) {
        if(id == prev_id){
            assert(t > prev_t);
        } else {
            assert(id == prev_id + 1);
        }
        result.push_back({x, y}, id);

        prev_id = id;
        prev_t = t;
    }
    return result;
}
} // End of namespace `frechet`
