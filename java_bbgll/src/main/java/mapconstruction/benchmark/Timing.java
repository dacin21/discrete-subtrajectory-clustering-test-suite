package mapconstruction.benchmark;

import java.util.*;
import java.util.concurrent.CopyOnWriteArrayList;

public class Timing implements AutoCloseable {

    private String title;
    private boolean isRunning;
    // duration in ms
    private long duration;

    private Timing parent;
    private Collection<Timing> subtimings;

    public Timing(String title) {
        this.title = title;
        this.duration = 0;
        this.subtimings = new CopyOnWriteArrayList<>();
    }

    public String getTitle() {
        return title;
    }

    public long getDuration() {
        return isRunning ? System.currentTimeMillis() - duration : duration;
    }

    public Timing getParent() {
        return parent;
    }

    public void setParent(Timing parent) {
        this.parent = parent;
    }

    public boolean hasParent() {
        return this.parent != null;
    }

    /**
     * Switches between 'running' and 'paused'. On first occurrence the duration is set to the current time.
     * With the second occurrence we can then compute the duration. On a third occurrence we set it back to the
     * current time with the duration subtracted, such that on the next occurrence it will contain the sum of durations.
     */
    public void time() {
        this.isRunning = !this.isRunning;
        this.duration = System.currentTimeMillis() - this.duration;

        if (!this.isRunning) {
            subtimings.forEach(Timing::close);
        }
    }

    /**
     * Add a new subtiming and update its references to the current timing
     */
    public Timing addSubtiming(String title) {
        Timing subtiming = getSubtiming(title);
        if (subtiming == null) {
            subtiming = new Timing(title);
            subtimings.add(subtiming);
            subtiming.setParent(this);
        }
        return subtiming;
    }

    private Timing getSubtiming(String title) {
        for (Timing subtiming : subtimings) {
            if (title.equals(subtiming.title)) {
                return subtiming;
            }
        }
        return null;
    }

    public Collection<Timing> getSubtimings() {
        return subtimings;
    }


    /**
     * Force stop the current timer, will call {@link Benchmark#stop()} when still running.
     */
    @Override
    public void close() {
        if (this.isRunning) {
            time();
        }
    }
}
