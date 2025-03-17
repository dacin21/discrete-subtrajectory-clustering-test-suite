package mapconstruction.GUI.io.roadmap;

import mapconstruction.algorithms.maps.mapping.RoadMap;
import mapconstruction.algorithms.maps.mapping.RoadSection;
import mapconstruction.algorithms.maps.network.MapEdge;
import mapconstruction.algorithms.maps.network.MapVertex;
import mapconstruction.log.Log;
import mapconstruction.log.LogLevel;

import java.awt.geom.Point2D;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.*;

import static mapconstruction.GUI.datastorage.DataStorage.STORAGE;

/**
 * @author Jorren
 */
public class RoadmapToVertexEdge extends RoadmapConsumer {

    private int precision;

    public RoadmapToVertexEdge(int precision) {
        this.precision = precision;
    }

    public RoadmapToVertexEdge() {
        this(5);
    }

    @Override
    public void accept(RoadMap roadMap) {
        saveRoadMap(STORAGE.getDatasetConfig().getPath(), roadMap);
    }

    public void saveRoadMap(String name, RoadMap network) {
        Map<Point2D, MapVertex> vMap = new HashMap<>();
        Set<MapVertex> vertices = new LinkedHashSet<>();
        Set<MapEdge> edges = new LinkedHashSet<>();

        for (RoadSection section : network.getPresentRoadSections()) {
            List<Point2D> points = section.getPointList();
            MapVertex vj = null;
            for (Point2D pi : points) {
                MapVertex vi = vMap.get(pi);
                if (vi == null) {
                    vi = new MapVertex(pi.getX(), pi.getY(), null, false);
                    vMap.put(pi, vi);
                    vertices.add(vi);
                }

                if (vj != null) {
                    edges.add(new MapEdge(vj, vi, null));
                }

                vj = vi;
            }
        }

        try {
            makeVertexFile(String.format("%s_vertices.txt", name), vertices);
            makeEdgeFile(String.format("%s_edges.txt", name), edges);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void makeVertexFile(String filename, Set<MapVertex> vertices) throws IOException {
        File output = new File(this.path, filename).getAbsoluteFile();
        BufferedWriter writer = new BufferedWriter(new FileWriter(output.toString()));
        for (MapVertex v : vertices) {
            Point2D loc = v.getLocation();
            writer.write(String.format(Locale.US, "%d,%." + precision + "f,%." + precision + "f\n", v.getId(), loc.getX(), loc.getY()));
        }
        writer.close();
    }

    private void makeEdgeFile(String filename, Set<MapEdge> edges) throws IOException {
        File output = new File(this.path, filename).getAbsoluteFile();
        BufferedWriter writer = new BufferedWriter(new FileWriter(output.toString()));
        int id = 0;
        for (MapEdge e : edges) {
            writer.write(String.format(Locale.US,"%d,%d,%d\n", id ++, e.getV1().getId(), e.getV2().getId()));
        }
        writer.close();
    }

}
