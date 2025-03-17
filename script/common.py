from collections import defaultdict
from functools import lru_cache
from itertools import zip_longest
from pathlib import Path


def git_root_directory():
    """
    Get the root directory of this git repo.
    Returns a pathlib.Path object.
    """
    # this script resides in git_repo/script/common.py
    return Path(__file__).parents[1]


def find_all_files(dir, filename, skip_java=False):
    # these directories contain too many files.
    for banned in ['data/tmp', 'movetk/build', 'podman/venv_pypy', 'our_code/build', 'movetk/src', 'java/src', 'java/savst', 'java/target']:
        if str(dir).endswith(banned):
            return

    for child in dir.iterdir():
        if child.is_dir():
            # avoid directories starting with '.' such as '.git'.
            if child.is_symlink() or child.name[0] == '.':
                continue
            yield from find_all_files(child, filename)
        else:
            if child.match(filename):
                yield child


@lru_cache(maxsize=100)
def find_file(filename, *, must_exist=True, ensure_unique=True):
    """
    Searches the git repo for a file with the specific name.
    Supports pathlib.Path.glob patterns.

    Arguments
        <filename>          string, name of the file
        <ensure_unique>     boolean, whether to error if two or more files match

    Returns a pathlib.Path object to the first file found.
    Assertion fails if no file is found.
    """
    files = find_all_files(git_root_directory(), filename, skip_java=True)

    first_match = next(files, None)
    second_match = next(files, None)

    if not first_match and not must_exist:
        return None

    assert first_match, f"Failed to find file: {filename}"
    assert not (ensure_unique and second_match), f"Duplicate files at {first_match} and {second_match}"

    return first_match


def parse_data_set(infile):
    """
    Parses a trajectories data set.

    Input format
        Each line is of the form <trajID> <timestamp> <latitude> <longitude>
            <trajID>    trajectory id, integer starting with 0
            <time>      POSIX timestamp, 64 bit integer
            <lat>       latitude, float
            <long>      longitude, float

    Return value
        {trajID: [(time, lat, long)]}
    """
    trajectories = defaultdict(list)
    for line in infile.readlines():
        tokens = line.strip().split()
        assert len(tokens) == 4, f"Failed to parse line {tokens}"
        traj_id = int(tokens[0])
        t = int(tokens[1])
        lat, long = map(float, tokens[2:4])
        trajectories[traj_id].append((t, lat, long))

    for points in trajectories.values():
        points.sort()

    return trajectories


"""
Given a path string, remove all directories and return only the filename.
Note: we cannot use pathlib for this, as the path might stem
from a container, which makes it invalid on the host system.
"""
def strip_directory(path):
    return path.split('/')[-1]


def parse_clusters(infile):
    """
    Parses a clustering output file.

    Input format
        3 lines of header

        <data set>
        <algorithm> <param_1> ... <param_k>
        <time>

            <data set>      string, filename of the data set
            <algorithm>     string, name of the algorithm
            <param_i>       string, parameter of the algorithm
            <time>          float, running time in seconds

        followed by multiple pairs of lines, describing one pathlet, in the following format

        <traj> <l> <r>
        <traj_1> <l_1> <r_1> <traj_2> <l_2> <r_2> ... <traj_n> <l_n> <r_n>

            <traj>          integer, id of the trajectory out of which the pathlet is constructed
            <l>, <r>        integer, the pathlet consists of the points [l, r] (zero based) of this trajectory.

            <traj_i>        integer, id of the trajectory this pathlet covers
            <l_i>, <r_i>    integer, on this trajectory, the points [l_i, r_i] (zero based) are covered by the pathlet.

    Return value
        (header, {(traj, l, r): [(traj_i, l_i, r_i))]})
    """
    data_set_path = infile.readline().strip()
    data_set_name = strip_directory(data_set_path)
    algorithm = infile.readline().strip()
    time = float(infile.readline())
    covering = {}
    for line1, line2 in zip_longest(*[infile]*2):
        assert line2
        pathlet = tuple(map(int, line1.strip().split()))
        assert len(pathlet) == 3
        subtrajectories = list(map(tuple, zip_longest(*[map(int, line2.strip().split())]*3)))
        # print(line1, line2, '--', pathlet, subtrajectories)
        covering[pathlet] = subtrajectories

    return (data_set_name, algorithm, time), covering


def frechet_distance_squared(trajectories, subtrajectory_a, subtrajectory_b):
    id_a, l_a, r_a = subtrajectory_a
    id_b, l_b, r_b = subtrajectory_b
    traj_a = trajectories[id_a]
    traj_b = trajectories[id_b]
    n_b = r_b - l_b + 1

    def distance_squared(p_a, p_b):
        return sum(d * d for k in [1, 2] for d in [p_a[k] - p_b[k]])

    inf = float('inf')
    dp = [inf] * (n_b + 1)
    dp[0] = 0
    for i in range(l_a, r_a + 1):
        dp2 = [inf] * (n_b + 1)
        for j in range(n_b):
            dp2[j+1] = max(min(dp2[j], dp[j+1], dp[j]), distance_squared(traj_a[i], traj_b[l_b+j]))
        dp = dp2
    return dp[-1]


"""
Formats floats with the specified format_str.
Recursively traverses dicts and lists.
"""
def format_floats(data, format_str="{:.2f}"):
    if isinstance(data, list):
        return [format_floats(e, format_str) for e in data]
    if isinstance(data, dict):
        return {key: format_floats(value, format_str) for key, value in data.items()}
    if not isinstance(data, float):
        return data
    return format_str.format(data)
