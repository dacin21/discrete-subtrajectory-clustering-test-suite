package mapconstruction.algorithms.bundles.sweep;

import mapconstruction.algorithms.bundles.graph.GeneratingSemiWeakFDLabelledGraph;
import mapconstruction.algorithms.bundles.graph.representation.Event;
import mapconstruction.algorithms.bundles.graph.representation.Vertex;
import mapconstruction.trajectories.*;

import java.util.*;
import java.util.stream.Collectors;

public class FurthestEndpointSweep extends KLSweepline {

    private static int kMin = -1;

    private SortedSet<Event> eventQueue;
    private double lambda;

    public FurthestEndpointSweep(GeneratingSemiWeakFDLabelledGraph freeSpace, double lambda) {
        super(freeSpace);
        this.lambda = lambda;
        int old_kMin = kMin;
        this.kMin = Integer.parseInt(System.getenv("BBGLL_SIZE")) - 1; // -1 since we don't count the trajectory itself.
        if (old_kMin == -1) { // we call this many times, so only print the detection once.
            System.out.println("Detected minK=" + this.kMin + " from BBGLL_SIZE.");
        }
    }

    @Override
    public void initialize() {
        // generate free space and extract events
        freeSpace.compute();
        eventQueue = freeSpace.collectEvents();
    }

    @Override
    public Set<Bundle> sweep() {
        Map<Trajectory, Edge> status = new HashMap<>();
        TreeSet<Edge> edges = new TreeSet<>();
        Set<Bundle> results = new HashSet<>();

        int high = -1;
        for (Event e : eventQueue) {
            Trajectory parent = freeSpace.getTrajectory(e);
            if (parent == null) continue;

            if (e.isStart()) {
                Edge current = status.get(parent);
                if (current == null || current.t.x() < e.t.x()) {
                    Edge entry = new Edge(e.s, e.t);
                    status.put(parent, entry);
                    // remove current if present
                    if (current != null) {
                        edges.remove(current);
                    }
                    edges.add(entry);
                }
            } else if (status.containsKey(parent)) {
                Edge ce = status.get(parent);
                // if current parent Edge is from a different event pair, continue
                if (ce.sx() != e.s.x() || ce.tx() != e.t.x()) continue;

                Trajectory representative = freeSpace.getRepresentative();
                if (e.t.x() > high && !parent.reverse().equals(representative)) {
                    List<Edge> partialBundle = new ArrayList<>();
                    int low = e.s.x();
                    for (Edge current : edges) {
                        if (current.sx() > low) {
                            // create bundle
                            Bundle b = makeBundle(representative, partialBundle, low, e.t.x());
                            // add bundle and update results
                            updateBundles(results, b);

                            low = current.sx();
                        }
                        partialBundle.add(current);
                    }

                    // create bundle
                    Bundle b = makeBundle(representative, partialBundle, low, e.t.x());
                    // update
                    updateBundles(results, b);

                    high = e.t.x();
                }
                status.remove(parent);
                edges.remove(new Edge(e.s, e.t));
            }
        }
        return results;
    }

    /**
     * Create a bundle given a representative, a list of {@code Edge}s and the integer bounds on the representative.
     * It guarantees each parent trajectory can only occur once in a bundle, either normal or reversed. Furthermore,
     * sub-trajectories are trimmed to be maximal taking epsilon into account.
     */
    private Bundle makeBundle(Trajectory representative, List<Edge> edges, int s, int t) {
        Map<Trajectory, Subtrajectory> candidates = new HashMap<>();
        for (Edge edge : edges) {
            Subtrajectory candidate = freeSpace.getTrimmedSubTrajectory(s, t, edge.t);

            if (candidate != null && candidate.getFromIndex() < candidate.getToIndex()) {
                Trajectory parent = candidate.getParent();
                if (parent.isReverse()) parent = parent.reverse();
                // keep the longest subtrajectory from the same parent.
                if (!candidates.containsKey(parent) || candidates.get(parent).euclideanLength() < candidate.euclideanLength()) {
                    candidates.put(parent, candidate);
                }
            }
        }
        return new UndirectionalBundle(candidates.values(), new Subtrajectory(representative, s, t));
    }

    /**
     * Update the set of resulting bundles by adding b if there is no bundle overlapping b already present.
     * When b is added, any bundles overlapped by b are removed from the set.
     * In this instance we take 'overlap' somewhat loosely and also look at lambda-subbundles.
     */
    private void updateBundles(Set<Bundle> results, Bundle b) {
        if (b.size() >= kMin){
            if (b.getOriginalRepresentative().intervalLength() >= 20) {
                // System.out.println("Got bundle of length: " + b.getOriginalRepresentative().intervalLength());
            }
            results.add(b);
        }
        return;
        // // lambda/2 is 'hack' to obtain epsilon, should rewrite class to have epsilon as parameter
        // if (b.size() >= kMin && b.continuousLength() >= lambda / 2d) {
        //     Optional<Bundle> b2 = results.stream().filter(c -> c.hasAsLambdaSubBundle(b, lambda)).findFirst();
        //     if (!b2.isPresent()) {
        //         // remove all bundles covered by 'b'
        //         results.stream().filter(r -> b.hasAsLambdaSubBundle(r, lambda)).collect(Collectors.toList()).forEach(results::remove);
        //         // add b to the result set
        //         results.add(b);
        //     }
        // }
    }

    static class Edge implements Comparable<Edge> {
        Vertex s;
        Vertex t;

        Edge(Vertex s, Vertex t) {
            this.s = s;
            this.t = t;
        }

        int sx(){ return s.x(); }
        int tx(){ return t.x(); }
        int sy(){ return s.y(); }
        int ty(){ return t.y(); }

        @Override
        public String toString() {
            return "Edge{s=(" + s.x() + "," + s.y() + "), t=(" + t.x() + "," + t.y() + ")}";
        }


        @Override
        public int compareTo(Edge o) {
            return Comparator.comparingInt(Edge::sx)
                    .thenComparingInt(Edge::sy)
                    .thenComparingInt(Edge::ty)
                    .compare(this, o);
        }
    }
}
