"""
A script to reduce the size of a data set, by
    - keeping a random subset of trajectories
    - keeping every k-th point on every trajectory

Input/Output format
    Each line is of the form <trajID> <timestamp> <latitude> <longitude>
        <trajID>    trajectory id, integer starting with 0
        <time>      POSIX timestamp, 64 bit integer
        <lat>       latitude, float
        <long>      longitude, float
"""
import argparse
from script.common import find_file


def write_trajectory(id, points, outfile):
    for point in points:
        print(id, *point, file=outfile)


def weighted_midpoint(p1, p2, w1, w2):
    ret = [(x1 * w1 + x2 * w2) / (w1 + w2) for x1, x2 in zip(map(float, p1), map(float, p2))]
    ret[0] = int(ret[0])  # timestamps should be integers
    return tuple(ret)


def reduce_curve(points, p):
    assert p != 0
    if not points:
        return points

    if p > 0:
        return points[::p]
    else:
        return [weighted_midpoint(p1, p2, -p - i, i) for p1, p2 in zip(points, points[1:]) for i in range(-p)] + [points[-1]]


def reduce_data(infile, t, p, outfile):
    num_trajectories = 0
    heat = 0

    last_id = None
    points = []

    def flush():
        nonlocal heat, num_trajectories, points
        points = reduce_curve(points, p)
        if len(points) < 2:
            return
        heat += 1
        if heat >= t:
            heat -= t
            write_trajectory(num_trajectories, points, outfile)
            num_trajectories += 1

    for line in infile.readlines():
        id, time, lat, long = line.strip().split()
        if last_id != id:
            flush()
            last_id = id
            points = []
        points.append((time, lat, long))
    flush()


def add_filename_suffix(filename, suffix):
    tokens = filename.split('.')
    if len(tokens) == 1:
        return filename + suffix
    return '.'.join(tokens[:-1]) + suffix + '.' + tokens[-1]


def main(args):
    data_set_file = find_file(args.filename, True)
    output_file = data_set_file.with_stem(data_set_file.stem + f"_{args.t}_{args.p}")
    with data_set_file.open('r') as infile:
        with output_file.open('w') as outfile:
            reduce_data(infile, args.t, args.p, outfile)
