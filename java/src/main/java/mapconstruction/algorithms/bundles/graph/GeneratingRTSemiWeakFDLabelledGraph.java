package mapconstruction.algorithms.bundles.graph;

import mapconstruction.algorithms.bundles.graph.representation.LabelledEdge;
import mapconstruction.algorithms.distance.RTree;
import mapconstruction.trajectories.Trajectory;

import java.awt.geom.Line2D;
import java.awt.geom.Point2D;
import java.util.*;

public class GeneratingRTSemiWeakFDLabelledGraph extends GeneratingSemiWeakFDLabelledGraph {

    private RTree<Line2D, Integer> rTree;

    public GeneratingRTSemiWeakFDLabelledGraph(double epsilon, Trajectory representative, List<Trajectory> concatenated, RTree<Line2D, Integer> rTree) {
        super(epsilon, representative, concatenated);
        this.rTree = rTree;
    }

    public void compute() {
        int x, y;

        for (int i = 0; i < representative.numEdges(); i++) {
            Set<Integer> query = query(i);
            Map<Integer, List<Integer>> candidates = new HashMap<>();

            for (int j : query) {
                x = vertexGraphCoord(i + 1);
                y = edgeGraphCoord(j);
                if (isFree(x, y)) {
                    List<LabelledEdge> edges = getEdges(x, y);
                    tryVertAddLeftEdge(x, y, edges);
                }

                x = edgeGraphCoord(i);
                y = vertexGraphCoord(j + 1);
                if (isFree(x, y)) {
                    Map.Entry<Integer, Trajectory> index = concatenated.floorEntry(j);
                    // if j+1 strictly fits the trajectory at j, add it to the candidates
                    if (j + 1 < index.getKey() + index.getValue().numEdges()) {
                        List<LabelledEdge> edges = getEdges(x, y);
                        tryHorAddLeftEdge(x, y, edges);
                        if (edges.size() > 0) {
                            // add (x,y) as candidate
                            List<Integer> candidate = getCandidate(candidates, index.getKey());
                            candidate.add(j + 1);
                        }
                    }
                }
            }

            for (List<Integer> candidate : candidates.values()) {
                candidate.sort(Integer::compareTo);
                ListIterator<Integer> iter = candidate.listIterator();
                while (iter.hasNext()) {
                    // don't check whether it fits the corresponding trajectory, do that on insertion only
                    int j = iter.next();

                    x = vertexGraphCoord(i + 1);
                    y = edgeGraphCoord(j);
                    if (isFree(x, y)) {
                        List<LabelledEdge> edges = getEdges(x, y);
                        tryVertAddBottomEdge(x, y, edges);
                    }
                    x = edgeGraphCoord(i);
                    y = vertexGraphCoord(j + 1);
                    if (isFree(x, y)) {
                        List<LabelledEdge> edges = getEdges(x, y);
                        tryHorAddBottomEdge(x, y, edges);
                        if (edges.size() > 0) {
                            Map.Entry<Integer,Trajectory> index = concatenated.floorEntry(j);
                            if (j + 1 < index.getKey() + index.getValue().numEdges()) {
                                // insert new j (if needed) at the correct location, move pointer back to current index.
                                if (!iter.hasNext() || candidate.get(iter.nextIndex()) > j + 1) {
                                    iter.add(j + 1);
                                    iter.previous();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    private Set<Integer> query(int i) {
        Point2D p = representative.getPoint(i);
        return rTree.windowQuery(p.getX() - epsilon, p.getY() - epsilon, p.getX() + epsilon, p.getY() + epsilon);
    }

    private List<LabelledEdge> getEdges(int x, int y) {
        if (!labelledGraph.contains(x, y)) {
            labelledGraph.put(x, y, new ArrayList<>(2));
        }
        return labelledGraph.get(x, y);
    }

    private List<Integer> getCandidate(Map<Integer, List<Integer>> candidates, int index) {
        if (!candidates.containsKey(index)) {
            candidates.put(index, new ArrayList<>());
        }
        return candidates.get(index);
    }
}
