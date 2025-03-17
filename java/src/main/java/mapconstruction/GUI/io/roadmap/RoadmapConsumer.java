package mapconstruction.GUI.io.roadmap;

import mapconstruction.algorithms.maps.mapping.RoadMap;
import mapconstruction.log.Log;
import mapconstruction.log.LogLevel;

import java.io.File;
import java.util.function.Consumer;

/**
 * @author Jorren
 */
public abstract class RoadmapConsumer implements Consumer<RoadMap> {

    protected File path;

    public void consume(String directory, RoadMap map) {
        this.path = new File(directory);
        if (path.mkdirs()) {
            Log.log(LogLevel.INFO, "SaveState", "Created directory " + path);
        }
        this.accept(map);
    }

}
