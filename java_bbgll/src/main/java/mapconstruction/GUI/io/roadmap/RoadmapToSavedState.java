package mapconstruction.GUI.io.roadmap;

import mapconstruction.algorithms.maps.mapping.RoadMap;
import mapconstruction.algorithms.maps.mapping.RoadSection;
import mapconstruction.log.Log;
import mapconstruction.log.LogLevel;

import java.awt.geom.Point2D;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Locale;
import java.util.StringJoiner;

/**
 * @author Jorren
 */
public class RoadmapToSavedState extends RoadmapConsumer {

    private int precision;

    public RoadmapToSavedState() {
        this(5);
    }

    public RoadmapToSavedState(int precision) {
        this.precision = precision;
    }

    @Override
    public void accept(RoadMap roadMap) {
        File outFile = new File(path, "roadmap.txt");
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(outFile))) {
            for (RoadSection section : roadMap.getPresentRoadSections()) {
                StringJoiner lineString = new StringJoiner(",");
                for (Point2D point : section.getPointList()) {
                    lineString.add(toString(point));
                }
                writer.write(lineString + "\n");
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private String toString(Point2D point) {
        return String.format(Locale.US, "%." + precision + "f %." + precision + "f", point.getX(), point.getY());
    }

}
