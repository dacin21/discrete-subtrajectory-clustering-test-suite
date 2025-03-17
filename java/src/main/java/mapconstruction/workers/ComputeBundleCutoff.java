package mapconstruction.workers;

import com.google.common.collect.BiMap;
import com.google.common.collect.HashBiMap;

import mapconstruction.trajectories.Bundle;

import java.awt.geom.Line2D;
import java.awt.geom.Rectangle2D;
import java.util.*;

import static mapconstruction.GUI.datastorage.DataStorage.STORAGE;

/**
 * Simple utility for simplifying a dataset given a boundingbox. If a bundle
 *
 * @author Jorren
 */
public class ComputeBundleCutoff extends AbortableAlgorithmWorker<Void, Void> {

    private Rectangle2D bounds;

    public ComputeBundleCutoff(Rectangle2D boundingbox) {
        this.bounds = boundingbox;
    }

    @Override
    protected Void doInBackground() throws Exception {
        BiMap<Bundle,Integer> original = STORAGE.getAllBundlesWithClasses();
        BiMap<Bundle,Integer> results = HashBiMap.create();
        int count = 0; double modifier = 99 / (double) original.size();

        System.out.println(bounds);

        for (Map.Entry<Bundle,Integer> entry : original.entrySet()) {
            if (this.clipsBounds(entry.getKey())) {
                results.put(entry.getKey(), entry.getValue());
            }
            count ++;
            STORAGE.setProgressAlgorithm((int) (count * modifier));
        }

        STORAGE.setBundlesWithClasses(results);

        return null;
    }

    private boolean clipsBounds(Bundle bundle) {
        System.out.println("NEW BUNDLE");
        for (Line2D segment : bundle.getOriginalRepresentative().edges()) {
            System.out.println("TRIED: " + segment.getP1() + "," + segment.getP2());
            if (bounds.intersectsLine(segment)) {
                return true;
            }
        }

        return false;
//        return bundle.getOriginalRepresentative().edges().stream().anyMatch(l -> bounds.intersectsLine(l));
    }
}
