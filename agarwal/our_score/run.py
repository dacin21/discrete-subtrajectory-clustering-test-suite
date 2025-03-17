# Python script to run all modules in one go, without generating intermediate files.
import cPickle as pickle
import time
from math import sqrt

from base import *
from distance import *
from greedy import *
from ioUtils import *


def sq(x):
    return x * x


def distance(p1, p2):
    return sqrt(sq(p1.lat - p2.lat) + sq(p1.lon - p2.lon))


"""
Approximately computes the smallest distance between any pair of points.
As a simplification, we only consider consecutive points on the same pathlet.
This also avoids issues with (self-)intersecting trajectories.

parameters
    <trajs>     {trajID : traj}
"""
def compute_rmin(trajs):
    rmin = 1e90
    for traj in trajs.values():
        for p1, p2 in zip(traj.pts[:-1], traj.pts[1:]):
            d = distance(p1, p2)
            if d > 0:
                rmin = min(rmin, d)
    return rmin


"""
Approximately computes the maximum distance between any pair of points.
To simplify the implementation, we compute the bounding box, which yields a 1.41-approximation.

parameters
    <trajs>     {trajID : traj}
"""
def compute_rmax(trajs):
    xmin = 1e90
    xmax = -1e90
    ymin = 1e90
    ymax = -1e90
    for traj in trajs.values():
        for p in traj.pts:
            xmin = min(xmin, p.lon)
            xmax = max(xmax, p.lon)
            ymin = min(ymin, p.lat)
            ymax = max(ymax, p.lat)

    return sqrt(sq(xmax - xmin) + sq(ymax - ymin))


def main():
    if not (len(sys.argv) in (6, 7, 8)):
        print "Wrong command. Correct command is python run.py data_file output_file c1 c2 c3 (rmin) (rmax)"
        return

    data_file_name = sys.argv[1]
    output_file_name = sys.argv[2]
    c1, c2, c3 =  map(float, sys.argv[3:6])
    rmin = float(sys.argv[6]) if len(sys.argv) >= 7 else None
    rmax = float(sys.argv[7]) if len(sys.argv) >= 8 else None

    start_time = time.time()

    print "Loading trajectories ..."
    data_sets_folder = '../../data_sets/'
    trajs = readTrajsFromDaruFile(data_file_name)

    # rmin, rmax = 0.5, 2
    rmin = rmin if rmin else compute_rmin(trajs)
    rmax = rmax if rmax else compute_rmax(trajs)
    print "rmin: ", rmin, "rmax: ", rmax

    print "Computing Frechet distances ..."
    distPairs1 = process(trajs, rmin, rmax)

    # distPairs1 is of form {(pth, straj):dist}, change it to distPairs2 of the form {(pth, trajID):[(straj, dist)]}
    distPairs2 = {}
    for k, v in distPairs1.iteritems():
        pth, trID, dist, straj = k[0], k[1].trajID, v, k[1]
        if distPairs2.has_key((pth, trID)):
            distPairs2[(pth, trID)].append((straj, dist))
        else:
            distPairs2[(pth, trID)] = [(straj, dist)]

    info = {}
    for k, v in distPairs2.iteritems():
        pth, trajID = k
        if pth not in info:
            info[pth] = []
        info[pth].extend([(straj[0].trajID, straj[0].bounds, straj[1]) for straj in v])

    # print "candidate pathlets:"
    # for pth, v in sorted(info.iteritems(), key = lambda e : (e[0].trajID, e[0].bounds)):
    #     print pth.trajID, pth.bounds, ' : ', v

    # info = [(pth.trajID, pth.bounds, trajID, ':', [(straj[0].trajID, straj[0].bounds, straj[1]) for straj in v]) for k, v in distPairs2.iteritems() for pth, trajID in [k]]
    # print "pathlets: ", info

    print "Computing prerequisite data structures ..."
    (strajCov, ptStraj, strajPth, trajCov) = preprocessGreedy(trajs, distPairs2)

    print "c1, c2, c3: ", c1, c2, c3

    print "Running greedy algorithm ..."
    pthAssignments, pthStats, unassignedPts = runGreedy(trajs, distPairs2, strajCov, ptStraj, strajPth, trajCov, c1, c2, c3)

    end_time = time.time()
    total_time = (end_time - start_time)

    with open(output_file_name, 'w') as outfile:
        print >> outfile, data_file_name
        print >> outfile, "agarwal", c1, c2, c3, rmin, rmax
        print >> outfile, total_time
        # pthAssignments is {pathlet : [subtraj]}
        # pthStats is [(pathlet, ...)]

        # loop over list to get pathlets in order. (The order of a dict is random.)
        for stat in pthStats:
        # for pathlet, subtrajectories in pthAssignments.items():
            pathlet = stat[0]
            subtrajectories = pthAssignments[pathlet]
            print >> outfile, pathlet.trajID, pathlet.bounds[0], pathlet.bounds[1]
            for subtrajectory in subtrajectories:
                print >> outfile, subtrajectory.trajID, subtrajectory.bounds[0], subtrajectory.bounds[1],
            print >> outfile, '\n',

    print "found", len(pthAssignments), "pathlets."
    # print pthAssignments
    # print pthStats
    # print unassignedPts
    # print retVal


if __name__ == "__main__":
    main()
