import argparse
import math
from concurrent.futures import ThreadPoolExecutor, ProcessPoolExecutor
import random as rng

from concorde.tsp import TSPSolver



def random_displacement(delta):
    x = rng.gauss(0.0, 1.0)
    y = rng.gauss(0.0, 1.0)
    scale = delta / math.sqrt(x*x + y*y)
    return (x * scale, y * scale)


def add_points(p, q):
    return tuple(a + b for a, b in zip(p, q))


def generate_points(point_count, delta, M=3000):
    points = [(0.0, 0.0)]
    for _ in range(1, point_count):
        base = rng.choice(points)
        dir = random_displacement(delta)
        points.append(add_points(base, dir))

    # recale point set so radius is approximatetly M
    radius = max(abs(x) + abs(y) for x, y in points)
    scale = M / radius
    points = [(x*scale, y*scale) for x, y in points]

    return points


def scale_coordinates(x, M=10**3):
    return [int(e * M) for e in x]


def tsp(points):
    x, y = map(list, zip(*points))
    x = scale_coordinates(x)
    y = scale_coordinates(y)
    solver = TSPSolver.from_data(x, y, norm='EUC_2D')

    solution = solver.solve(verbose=False, time_bound=1)
    # assert solution.success
    tour_ids = list(solution.tour)
    print(f"Solved tsp on {len(tour_ids)} points")
    return [points[id] for id in tour_ids]


def generate_trajectories(points, trajectory_count, density, delta):
    points_per_trajectory = max(1, int(len(points) * density))

    sampled_points = [rng.sample(points, points_per_trajectory) for _ in range(trajectory_count)]
    # add random noise to avoid duplicate points
    sampled_points = [[add_points(p, random_displacement(delta / 1000)) for p in row] for row in sampled_points]

    # concorde takes at least 1s per trajectory, so we multi-thread
    # executor = ThreadPoolExecutor()
    executor = ProcessPoolExecutor()
    return executor.map(tsp, sampled_points)
    # futures = []
    # for _ in range(trajectory_count):
    #     sampled_points = rng.sample(points, points_per_trajectory)
    #     futures.append(executor.submit(tsp, sampled_points))
    # return [future.result() for future in futures]


def run(filename, point_count, trajectory_count, density):
    points = generate_points(point_count, 1.0)
    with open(filename, 'w') as outfile:
        for id, trajectory in enumerate(generate_trajectories(points, trajectory_count, density, 1.0)):
            for t, point in enumerate(trajectory):
                print(id, t, *point, file=outfile)


def main():
    parser = argparse.ArgumentParser(
        prog='synth.py',
        description='Synthetic data generation via TSP')
    parser.add_argument('filename')
    parser.add_argument('points', type=int, help='Number of points to sample trajectories from')
    parser.add_argument('trajectories', type=int, help='Number of trajectores sampled')
    parser.add_argument('density', type=float, help='Probability that a point is included in a trajectory')
    parser.add_argument('seed', type=int, help='Seed for rng')

    args = parser.parse_args()
    rng.seed(args.seed)
    run(args.filename, args.points, args.trajectories, args.density)


if __name__ == '__main__':
    main()
