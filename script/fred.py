"""
A script to call fred.Curves.simplify(l).

Input/Output format
    Each line is of the form <trajID> <timestamp> <latitude> <longitude>
        <trajID>    trajectory id, integer starting with 0
        <time>      POSIX timestamp, 64 bit integer
        <lat>       latitude, float
        <long>      longitude, float
"""
import argparse
from script.common import find_file, parse_data_set
import Fred


def write_trajectory(id, points, outfile):
    for point in points:
        print(id, *point, file=outfile)


def fredify_points(points):
    return [(x, y) for t, x, y in points]


def reduce_data(infile, factor, outfile):
    trajectories = parse_data_set(infile)
    for id, points in list(trajectories.items()):
        curves = Fred.Curves()
        curves.add(Fred.Curve(fredify_points(points), str(id)))
        simplified_curves = curves.simplify(1 + len(points) // factor, False)
        for curve in simplified_curves:
            for index, point in enumerate(curve.values):
                # yes time stamps are wrong, it would be annoying to keep stack of them.
                print(id, index, *point, file=outfile)


def main(args):
    data_set_file = find_file(args.filename, True)
    output_file = data_set_file.with_stem(data_set_file.stem + f"_f-{args.factor}")
    with data_set_file.open('r') as infile:
        with output_file.open('w') as outfile:
            reduce_data(infile, args.factor, outfile)
