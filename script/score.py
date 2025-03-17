"""
Scoring script that compute the c1/c2/c3 score of a clustering.
"""
from collections import defaultdict
from math import sqrt
from sys import stderr

from script.common import find_file, frechet_distance_squared, parse_clusters, parse_data_set

"""
Try to recover c1/c2/c3 from the header.
This is very hacky, as it depends on the commands that are used to run stuff.
I would've been better to include c1/c2/c3 in the output format...

c1      cost per cluster
c2      cost per distance
c3      cost if everything is uncovered
"""
def parse_c_from_header(header):
    line = header[1] # the command that was run.
    tokens = line.strip().split()
    if tokens[0] == 'agarwal':
        # agarwal swaps c2 and c3
        c1, c3, c2 = map(float, tokens[1:4])
    else:
        # our code uses -c <c1> <c2> <c3>
        c1, c2, c3 = map(float, tokens[1:4])

    return c1, c2, c3


def score_clusters(trajectories, header, clusters, c=None, old_score=False, verbose=True, centers=False):
    if c is None:
        c = parse_c_from_header(header)
    assert len(c) == 3

    n = sum(len(trajectory) for trajectory in trajectories.values())
    print(f"{len(trajectories)} trajectories spanning {n} points. Score coefficients: {c}")

    uncovered = [[1] * len(trajectory) for trajectory in trajectories.values()]
    cover_coefficient = [1.0/len(trajectory) if old_score else 1.0 / n for trajectory in trajectories.values()]
    overlap = 0

    def do_cover(subtrajectory):
        nonlocal uncovered, overlap
        id, l, r = subtrajectory
        ret = sum(uncovered[id][l:r+1])
        overlap += (r-l+1) - ret
        uncovered[id][l:r+1] = [0] * (r-l+1)
        return ret

    total_cost = [0] * 3
    total_cost[2] = c[2] * sum(cover_coefficient[i] * len(trajectory) for i, trajectory in enumerate(trajectories.values()))
    for pathlet_cover in clusters.items():
        pathlet, cover = pathlet_cover
        current_cost = [0] * 3
        current_cost[0] = c[0]
        for subtrajectory in cover:
            if len(trajectories) == 1:
                print(sqrt(frechet_distance_squared(trajectories, pathlet, subtrajectory)))

            if centers:
                current_cost[1] = max(current_cost[1], c[1] * sqrt(frechet_distance_squared(trajectories, pathlet, subtrajectory)))
            else:
                current_cost[1] += c[1] * sqrt(frechet_distance_squared(trajectories, pathlet, subtrajectory))
            current_cost[2] -= c[2] * cover_coefficient[subtrajectory[0]] * do_cover(subtrajectory)
        total_cost[0] += current_cost[0]
        if centers:
            total_cost[1] = max(total_cost[1], current_cost[1])
        else:
            total_cost[1] += current_cost[1]
        total_cost[2] += current_cost[2]

        if verbose:
            print(f"Pathlet: {current_cost[0]:>9.5f} {current_cost[1]:>9.5f} {current_cost[2]:>10.5f} -> {sum(current_cost):>10.5f}, ratio {current_cost[2] / (current_cost[0] + current_cost[1]):>10.5f}")

    # print(f"Total cost: {total_cost[0]:>9.5f} {total_cost[1]:>9.5f} {c[2] / n * sum(sum(e) for e in uncovered):>9.5f} -> {sum(total_cost):>9.5f}")
    print(f"Total:   {total_cost[0]:>9.5f} {total_cost[1]:>9.5f} {total_cost[2]:>10.5f} -> {sum(total_cost):>10.5f}")
    print(f"Overlap: {overlap}")

    return sum(total_cost)


"""
Used by plot.py
"""
def compute_score(input_file_path, output_file_path):
    with output_file_path.open('r') as infile:
        header, clusters = parse_clusters(infile)

    with input_file_path.open('r') as infile:
        trajectories = parse_data_set(infile)
    return score_clusters(trajectories, header, clusters)


def main(args):
    file_path = find_file(args.file, ensure_unique=True)
    with file_path.open('r') as infile:
        header, clusters = parse_clusters(infile)

    trajectory_file_path = find_file(header[0])
    with trajectory_file_path.open('r') as infile:
        trajectories = parse_data_set(infile)

    if args.concatenate:
        trajectories = {0: [point for id, trajectory in trajectories.items() for point in trajectory]}

    score_clusters(trajectories, header, clusters, c=vars(args).get('c', None), old_score=args.old_score)
