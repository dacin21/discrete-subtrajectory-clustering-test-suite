package mapconstruction.GUI.io.roadmap;

import mapconstruction.algorithms.maps.mapping.RoadMap;
import mapconstruction.algorithms.maps.mapping.RoadSection;
import mapconstruction.log.Log;
import mapconstruction.log.LogLevel;
import mapconstruction.trajectories.Trajectory;
import mapconstruction.util.Coordinate;
import mapconstruction.util.Pair;
import org.eclipse.jetty.util.IO;

import java.awt.geom.Point2D;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.List;
import java.util.Locale;
import java.util.StringJoiner;

import static mapconstruction.GUI.datastorage.DataStorage.STORAGE;

/**
 * @author Jorren
 */
public class RoadmapToKML extends RoadmapConsumer {

    private int indent = 0;

    @Override
    public void accept(RoadMap roadMap) {
        File outFile = new File(path, STORAGE.getDatasetConfig().getPath() + "_roadmap.kml");
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(outFile))) {
            writeDocument(writer, roadMap);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void writeDocument(BufferedWriter writer, RoadMap roadmap) throws IOException {
        newLine(writer, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        newLine(writer, "<kml xmlns=\"http://www.opengis.net/kml/2.2\">");
        indent ++;
        newLine(writer, "<Document>");
        indent ++;
        newLine(writer, "<name> Roadmap </name>");
        newLine(writer, "<open>1</open>");
        newLine(writer, "<description> A Roadmap constructed by the Buchin et. al. Algorithm </description>");

        writeStyle(writer);
        writeTrajectories(writer);
        writeFolder(writer, roadmap);

        indent --;
        newLine(writer, "</Document>");
        indent --;
        newLine(writer, "</kml>");
    }

    private void writeStyle(BufferedWriter writer) throws IOException {
        writeLineStyle(writer, "trajectory", "ffb40014", 1);
        writeLineStyle(writer, "roadSegment", "ff2b39c0", 4);
    }

    private void writeLineStyle(BufferedWriter writer, String id, String color, int width) throws IOException {
        newLine(writer, String.format("<Style id=\"%s\">", id));
        indent ++;
        newLine(writer, "<LineStyle>");
        indent ++;
        newLine(writer, String.format("<color>%s</color>", color));
        newLine(writer, String.format("<width>%d</width>", width));
        indent --;
        newLine(writer, "</LineStyle>");
        indent --;
        newLine(writer, "</Style>");
    }

    private void writeFolder(BufferedWriter writer, RoadMap roadMap) throws IOException {
        newLine(writer, "<Folder>");
        indent ++;
        newLine(writer, "<name> RoadMap </name>");
        newLine(writer, "<visibility>1</visibility>");
        newLine(writer, "<description> A collection of Road Sections in the RoadMap. </description>");

        int i = 0;
        for (RoadSection section : roadMap.getPresentRoadSections()) {
            i ++;
            writePlacemark(writer, "Road Section " + i, "roadSegment", 20, section.getPointList());
        }

        indent --;
        newLine(writer, "</Folder>");
    }

    private void writeTrajectories(BufferedWriter writer) throws IOException {
        newLine(writer, "<Folder>");
        indent ++;
        newLine(writer, "<name> Trajectories </name>");
        newLine(writer, "<visibility>1</visibility>");
        newLine(writer, "<description> The input Trajectory data. </description>");
        int i = 0;
        for (Trajectory t : STORAGE.getTrajectories()) {
            i ++;
            writePlacemark(writer, "Trajectory " + i, "trajectory", 15, t.points());
        }

        indent --;
        newLine(writer, "</Folder>");
    }

    private void writePlacemark(BufferedWriter writer, String label, String style, int altitude, List<Point2D> section) throws IOException {
        newLine(writer, "<Placemark>");
        indent ++;
        newLine(writer, "<name> " + label + "</name>");
        newLine(writer, "<visibility>1</visibility>");
        newLine(writer, String.format("<styleUrl>#%s</styleUrl>", style));

        writeLineString(writer, section, altitude);

        indent --;
        newLine(writer, "</Placemark>");
    }

    private void writeLineString(BufferedWriter writer, List<Point2D> section, int altitude) throws IOException {
        newLine(writer, "<LineString>");
        indent ++;
        newLine(writer, "<altitudeMode>relativeToGround</altitudeMode>");
        writeCoordinates(writer, section, altitude);
        indent --;
        newLine(writer, "</LineString>");
    }

    private void writeCoordinates(BufferedWriter writer, List<Point2D> section, int altitude) throws IOException {
        for (int i = 0; i < section.size(); i++) {
            Point2D p = Coordinate.UTMtoLatLon(section.get(i));
            if (i == 0) {
                newLine(writer, String.format(Locale.US, "<coordinates> %f,%f,%d", p.getY(), p.getX(), altitude));
                indent ++;
            } else if (i == section.size()-1) {
                newLine(writer, String.format(Locale.US,"%f,%f,%d </coordinates>", p.getY(), p.getX(), altitude));
                indent --;
            } else {
                newLine(writer, String.format(Locale.US,"%f,%f,%d", p.getY(), p.getX(), altitude));
            }
        }
    }

    private void newLine(BufferedWriter writer, String contents) throws IOException {
        String indent = new String(new char[this.indent]).replace('\0', '\t');
        writer.write(indent + contents + "\n");
    }

}
