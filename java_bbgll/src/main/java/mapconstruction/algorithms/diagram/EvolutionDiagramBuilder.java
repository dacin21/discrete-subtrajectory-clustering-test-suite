package mapconstruction.algorithms.diagram;

import com.google.common.collect.BiMap;
import com.google.common.collect.HashBiMap;
import mapconstruction.algorithms.AbstractTrajectoryAlgorithm;
import mapconstruction.algorithms.bundles.BundleGenerationAlgorithm;
import mapconstruction.algorithms.bundles.KLSubbundleAlgorithm;
import mapconstruction.algorithms.bundles.MaximalSubbundleAlgorithm;
import mapconstruction.algorithms.bundles.graph.GeneratingSemiWeakFDLabelledGraph;
import mapconstruction.algorithms.bundles.sweep.FurthestEndpointSweep;
import mapconstruction.algorithms.distance.KdTree;
import mapconstruction.algorithms.distance.RTree;
import mapconstruction.benchmark.Benchmark;
import mapconstruction.exceptions.AlgorithmAbortedException;
import mapconstruction.log.Log;
import mapconstruction.log.LogLevel;
import mapconstruction.trajectories.Bundle;
import mapconstruction.trajectories.Subtrajectory;
import mapconstruction.trajectories.Trajectory;

import java.awt.*;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.io.*;
import java.util.*;
import java.util.List;
import java.util.Map.Entry;
import java.util.concurrent.*;
import java.util.function.DoubleUnaryOperator;
import java.util.function.IntUnaryOperator;

import static mapconstruction.GUI.datastorage.ConstantsStorage.ALGOCONSTANTS;

/**
 * Builder for the diagram that tracks evolution of bundles.
 * <p>
 * This builder does not actually draw the diagram, but keeps track of the
 * intermediate states.
 *
 * @author Roel (original author)
 * @author Jorrick Sleijster (modified by)
 */
public class EvolutionDiagramBuilder extends AbstractTrajectoryAlgorithm<EvolutionDiagram> {

    static final String LOGTAG = "Evolution";
    /**
     * The value of lambda to use will be labdaFactor * epsilon
     */
    private final double lambdaFactor;
    /**
     * Maximum epsilon size for termination.
     */
    private final double maxEps;
    /**
     * Minimum epsilon size to start
     */
    private final double minEps;
    /**
     * Whether direction should be ignored when generating bundles.
     */
    private final boolean ignoreDirection;
    /**
     * Unary operator in the current value of epsilon that computes the next
     * value.
     */
    private final DoubleUnaryOperator nextEpsilon;
    /**
     * Type of increments
     */
    private final IncrType incrType;
    /**
     * value of incementer.
     */
    private final double incrementer;
    /**
     * Initial diagram to use. Used to extend a diagram with additional levels.
     */
    private final EvolutionDiagram initialDiagram;
    private final IntUnaryOperator kStep;
    /**
     * Whether we want to output message in the console whether the merges failed
     */
    private final boolean debugMerges = false;
    /**
     * ID of the next bundle class.
     */
    private int nextClassNumber = 0;
    /**
     * Algorithm used to generate the bundles in each step.
     */
    private BundleGenerationAlgorithm algo;
    private Set<Bundle> encounteredBundles;

    /**
     * Constructs a diagram builder with the given parameters
     *
     * @param lambdaFactor      factor for multiplying the current distance value to
     *                          get lambda.
     * @param maxEps            maximum epsilon
     * @param ignoreDirection
     */
    private EvolutionDiagramBuilder(double lambdaFactor, double minEps, double maxEps, boolean ignoreDirection, IncrType incrType, double incrementer, EvolutionDiagram initialDiagram, IntUnaryOperator kStep) {
        super();
        this.lambdaFactor = lambdaFactor;
        this.maxEps = maxEps;
        this.ignoreDirection = ignoreDirection;
        this.incrType = incrType;
        this.incrementer = incrementer;
        this.minEps = minEps;
        this.initialDiagram = initialDiagram;
        this.kStep = kStep;


        nextEpsilon = d -> {
            switch (incrType) {
                case Additive:
                    return d + incrementer;
                case Multiplicative:
                    return d == 0 ? 1 : d * incrementer;
                case Hybrid:
                    return d + (int) Math.ceil(d / 100) * incrementer;
                default:
                    throw new IllegalStateException();
            }
        };
    }

    public static EvolutionDiagramBuilder additive(double incrementer, double lambdaFactor, double minEps, double maxEps, boolean ignoreDirection, IntUnaryOperator kStep) {

        return new EvolutionDiagramBuilder(lambdaFactor, minEps, maxEps, ignoreDirection, IncrType.Additive, incrementer, new EvolutionDiagram(), kStep);
    }

    public static EvolutionDiagramBuilder multiplicative(double incrementer, double lambdaFactor, double minEps, double maxEps, boolean ignoreDirection, IntUnaryOperator kStep) {
        if (incrementer <= 1) {
            throw new IllegalArgumentException("Cannot build the diagram in Multiplicative mode if the  incrementer is not greater than 1.");
        }
        return new EvolutionDiagramBuilder(lambdaFactor, minEps, maxEps, ignoreDirection, IncrType.Multiplicative, incrementer, new EvolutionDiagram(), kStep);
    }

    public static EvolutionDiagramBuilder additive(double incrementer, double lambdaFactor, double minEps, double maxEps, boolean ignoreDirection, IntUnaryOperator kStep, EvolutionDiagram initialDiagram) {

        return new EvolutionDiagramBuilder(lambdaFactor, minEps, maxEps, ignoreDirection, IncrType.Additive, incrementer, initialDiagram, kStep);
    }

    public static EvolutionDiagramBuilder multiplicative(double incrementer, double lambdaFactor, double minEps, double maxEps, boolean ignoreDirection, IntUnaryOperator kStep, EvolutionDiagram initialDiagram) {
        if (incrementer <= 1) {
            throw new IllegalArgumentException("Cannot build the diagram in Multiplicative mode if the  incrementer is not greater than 1.");
        }
        return new EvolutionDiagramBuilder(lambdaFactor, minEps, maxEps, ignoreDirection, IncrType.Multiplicative, incrementer, initialDiagram, kStep);
    }

    /**
     * Build the diagram for the given list of trajectories in parallel
     *
     * @param trajectories
     * @return
     */
    public EvolutionDiagram runAlgorithmParallel(List<Trajectory> trajectories) {
        EvolutionDiagram diagram = initialDiagram;

        double epsilon;
        this.encounteredBundles = new HashSet<>();
        if (initialDiagram.isEmpty()) {
            Log.log(LogLevel.STATUS, LOGTAG, "Starting to build diagram from scratch");
            epsilon = minEps;
        } else {
            Log.log(LogLevel.STATUS, LOGTAG, "Starting to build diagram, extending an existing one");
            epsilon = nextEpsilon.applyAsDouble(diagram.getEpsilons().last());
            for (double e : diagram.getEpsilons()) {
                encounteredBundles.addAll(diagram.getBundleClasses(epsilon).keySet());
            }
            nextClassNumber = diagram.numClasses();
        }

        Log.log(LogLevel.INFO, LOGTAG, "Parameters for evolution diagram: lambdaFactor=%.2f, incr=%.2f, incrType=%s, minEps=%.2f, maxEps=%.2f, ignoreDir=%b", lambdaFactor, incrementer, incrType.name(), minEps, maxEps, ignoreDirection);

        // Assign all remaining threads to subtasks, keep one free for the current thread.
        ExecutorService executor = Executors.newFixedThreadPool(ALGOCONSTANTS.getNumThreads() - 1);
//        CompletionService<String> manager = new ExecutorCompletionService<>(executor);

//        Map<Double, Future<String>> results = new HashMap();
        Map<Double, Future<Pair<Set<Bundle>, Map<Bundle, Bundle>>>> results = new LinkedHashMap<>();

        Log.log(LogLevel.STATUS, LOGTAG, "Starting Threads to find all bundles.");

        trajectories = Collections.synchronizedList(trajectories);

        boolean use_rtree = Integer.parseInt(System.getenv("USE_RTREE")) > 0;

        // start workers to compute bundles
        while (epsilon <= maxEps && !aborted) {
            try {
                Callable<Pair<Set<Bundle>, Map<Bundle, Bundle>>> worker = new FindAllBundles(trajectories,
                        !use_rtree ? new MaximalSubbundleAlgorithm(epsilon, epsilon * lambdaFactor, this.ignoreDirection, kStep)
                         : new KLSubbundleAlgorithm(epsilon, epsilon * lambdaFactor, this.ignoreDirection)
                );
                results.put(epsilon, executor.submit(worker));

                // proper incrementing
                if (epsilon < maxEps && nextEpsilon.applyAsDouble(epsilon) > maxEps) {
                    epsilon = maxEps;
                } else {
                    epsilon = nextEpsilon.applyAsDouble(epsilon);
                }

                setProgress((int) (100 * epsilon / (maxEps - minEps + 1) / 2));
            } catch (AlgorithmAbortedException ex) {
                // algorithm aborted return partial diagram
                Log.log(LogLevel.WARNING, LOGTAG, "Algorithm aborted. Showing partial diagram");
                break;
            }
        }

        for (Iterator<Double> it = results.keySet().iterator(); it.hasNext();) {
            try {
                epsilon = it.next();
                Pair<Set<Bundle>, Map<Bundle, Bundle>> p = results.get(epsilon).get();

                Log.log(LogLevel.INFO, LOGTAG, "Starting processBundles bundl=%d eps=%f", p.k.size(), epsilon);

                DiagramState state = processBundles(p.k, p.v, epsilon, diagram);

                // Add the state
                Log.log(LogLevel.INFO, LOGTAG, "Got processBundles births=%d merges=%d", state.getBirths().size(), state.getMerges().size());

                diagram.addState(epsilon, state);

                for (int c : state.getBirths()) {
                    // Process birth moments
                    diagram.addBirthMoment(c, epsilon);
                }

                for (int c : state.getMerges().keySet()) {
                    // Process merges
                    diagram.addMergeMoment(c, epsilon);
                }

                // DISABLED: encounteredBundles has no useful functionality
//                encounteredBundles.addAll(state.getBundleClasses().keySet());
                // remove bundles so they can be cleared from memory
                it.remove();

                setProgress((int) (100 * epsilon / (maxEps - minEps + 1) / 2) + 50);
            } catch (AlgorithmAbortedException | InterruptedException e) {
                // algorithm aborted return partial diagram
                Log.log(LogLevel.WARNING, LOGTAG, "Algorithm aborted. Showing partial diagram");
                break;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        setProgress(100);
        Log.log(LogLevel.STATUS, LOGTAG, "Diagram finished");
        return diagram;
    }

    @Override
    public EvolutionDiagram runAlgorithm(List<Trajectory> trajectories) {
//        return runAlgorithmSequential(trajectories);
        return runAlgorithmParallel(trajectories);
    }

    /**
     * Build the diagram for the given list of trajectories. If aborted early,
     * returns a partial diagram.
     *
     * @param trajectories
     * @return
     */
    public EvolutionDiagram runAlgorithmSequential(List<Trajectory> trajectories) {
        EvolutionDiagram diagram = initialDiagram;

        double epsilon;
        this.encounteredBundles = new HashSet<>();
        if (initialDiagram.isEmpty()) {
            Log.log(LogLevel.STATUS, LOGTAG, "Starting to build diagram from scratch");
            epsilon = minEps;
        } else {
            Log.log(LogLevel.STATUS, LOGTAG, "Starting to build diagram, extending an existing one");
            epsilon = nextEpsilon.applyAsDouble(diagram.getEpsilons().last());
            for (double e : diagram.getEpsilons()) {
                encounteredBundles.addAll(diagram.getBundleClasses(epsilon).keySet());
            }
            nextClassNumber = diagram.numClasses();
        }

        Set<Bundle> result;

        Log.log(LogLevel.INFO, LOGTAG, "Parameters for evolution diagram: lambdaFactor=%.2f, incr=%.2f, incrType=%s, minEps=%.2f, maxEps=%.2f, ignoreDir=%b", lambdaFactor, incrementer, incrType.name(), minEps, maxEps, ignoreDirection);

        while (epsilon <= maxEps /*&& result.size() > 1*/ && !aborted) {
            try {
                BundleGenerationAlgorithm lambdaAlgo =
                       new MaximalSubbundleAlgorithm(epsilon, epsilon * lambdaFactor, this.ignoreDirection, kStep);
                        // new KLSubbundleAlgorithm(epsilon, epsilon * lambdaFactor, this.ignoreDirection);

                algo = lambdaAlgo;

                Log.log(LogLevel.STATUS, LOGTAG, "Finding all bundles.");

                result = algo.run(trajectories);
                Map<Bundle, Bundle> merges = lambdaAlgo.getMerges();

                Log.log(LogLevel.STATUS, LOGTAG, "Building state");

                DiagramState state = processBundles(result, merges, epsilon, diagram);

                // Add the state
                diagram.addState(epsilon, state);

                for (int c : state.getBirths()) {
                    // Process birth moments
                    diagram.addBirthMoment(c, epsilon);
                }

                for (int c : state.getMerges().keySet()) {
                    // Process merges
                    diagram.addMergeMoment(c, epsilon);
                }

                encounteredBundles.addAll(state.getBundleClasses().keySet());

//                Benchmark.addResult("Epsilon " + epsilon, state.getBundleClasses().keySet());

                setProgress((int) (100 * epsilon / (maxEps - minEps + 1)));
                // proper incrementing
                if (epsilon < maxEps && nextEpsilon.applyAsDouble(epsilon) > maxEps) {
                    epsilon = maxEps;
                } else {
                    epsilon = nextEpsilon.applyAsDouble(epsilon);
                }

                if (epsilon < maxEps && state.getBundleClasses().size() <= 1) {
                    epsilon = maxEps;
                }
            } catch (AlgorithmAbortedException ex) {
                // algorithm aborted return partial diagram
                Log.log(LogLevel.WARNING, LOGTAG, "Algorithm aborted. Showing partial diagram");
                break;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        setProgress(100);
        Log.log(LogLevel.STATUS, LOGTAG, "Diagram finished");
        return diagram;
    }

    /**
     * Processes the given bundles to generate a new state.
     *
     * @param result
     * @param epsilon
     * @param diagram
     * @return
     */
    private DiagramState processBundles(Set<Bundle> result, Map<Bundle, Bundle> bundleMerges, double epsilon, EvolutionDiagram diagram) {
        Log.log(LogLevel.STATUS, "processBundles", "Adding %d bundles", result.size());

        BiMap<Bundle, Integer> bundleClasses = HashBiMap.create();
        Set<Integer> births = new HashSet<>();
        Map<Integer, Integer> merges = new HashMap<>();

        if (diagram.isEmpty()) {
            handleFirstState(result, bundleClasses, births);
        } else {
            // Get the previous state
            DiagramState previousState = diagram.getPrevious(epsilon);

            for (Bundle bunNew : result) {

                boolean isContinuation = tryFindContinuation(previousState, bunNew, bundleClasses, epsilon);

                if (!isContinuation && !encounteredBundles.contains(bunNew)) {
                    // No old bundle found that the new one continues.
                    // Additionally, it is not a reappearing bundle
                    // add as new class

                    Log.log(LogLevel.STATUS, "processBundles", "Added a bundle");

                    bundleClasses.put(bunNew, nextClassNumber);

                    // Current birth moment
                    births.add(nextClassNumber);
                    nextClassNumber++;
                }

            }

            // Determine where the old classes have merged into
            // Find class numbers of classes that are no longer present.
            Set<Integer> mergedClasses = new HashSet<>(previousState.getBundleClasses().values());
            mergedClasses.removeAll(bundleClasses.values());

            Log.log(LogLevel.STATUS, "processBundles", "mergedClasses size %d", mergedClasses.size());

            for (int c : mergedClasses) {
                tryFindMerge(previousState, c, bundleClasses, epsilon, merges, bundleMerges);
            }

        }
        Log.log(LogLevel.STATUS, "processBundles", "Reported %d bundle", bundleClasses.size());
        return new DiagramState(bundleClasses, births, merges);
    }

    private void tryFindMerge(DiagramState previousState, int bundleClass, BiMap<Bundle, Integer> bundleClasses, double epsilon, Map<Integer, Integer> merges, Map<Bundle, Bundle> bundleMerges) {
        // Find into which classes the bundles have merged.

        Bundle mergedBundle = previousState.getBundleClasses().inverse().get(bundleClass);
        // find other class
        for (Entry<Bundle, Integer> bunClassPair : bundleClasses.entrySet()) {
            // Candidate
            Bundle otherBundle = bunClassPair.getKey();
            int otherClass = bunClassPair.getValue();
            if (otherBundle.hasAsLambdaSubBundle(mergedBundle, epsilon * lambdaFactor)) {
                // We allow a decrease of size
                merges.put(bundleClass, otherClass);
                return; // Found the merge for this class
            }
        }
        // Another attempt to find merges
        // find find correct bundle in bundle merges
        for (Entry<Bundle, Bundle> entry : bundleMerges.entrySet()) {
            Bundle from = entry.getKey();
            Bundle to = entry.getValue();
            if (from.hasAsLambdaSubBundle(mergedBundle, epsilon * lambdaFactor)) {
                // Find proper class
                while (!bundleClasses.containsKey(to) && bundleMerges.containsKey(to)) {
                    to = bundleMerges.get(to);
                    if (to == null) {
                        System.err.println("to == null");
                        Log.log(LogLevel.WARNING, LOGTAG, "to == null");
                        return;
                    }
                }
                if (debugMerges) {
                    System.err.println("WARNING: Second merge attempt used!");
                }
                Log.log(LogLevel.WARNING, LOGTAG, "Second merge attempt used!");
                merges.put(bundleClass, bundleClasses.get(to));
                return;
            }
        }
        if (debugMerges) {
            System.err.println("WARNING: No merge found!");
        }
        Log.log(LogLevel.WARNING, LOGTAG, "No merge found!");
    }

    private boolean tryFindContinuation(DiagramState previousState, Bundle bunNew, BiMap<Bundle, Integer> bundleClasses, double epsilon) {
        Set<Bundle> candidates = previousState.getBundleClasses().keySet();

        // For every new bundle, we have to check to every previous bundle
        // whether it is a continuation of a previous bundle.
        // If so, we can copy the class.
        // Otherwise we add a new class
        // We prefer proper subbundles over lambda subbundles. First attempt,
        // proper subbundles
        for (Bundle bunOld : candidates) {
            int classNumber = previousState.getBundleClasses().get(bunOld);
            if (bunNew.size() == bunOld.size() && !bundleClasses.containsValue(classNumber)) {
                // finally check for subbundle, as this is the most expensive check
                if (bunNew.hasAsSubBundle(bunOld)) {
                    // Continuation of old bundle
                    bundleClasses.put(bunNew, classNumber);
                    return true;
                }
            }
        }
        // Second attempt, with lambda subbundle.
        for (Bundle bunOld : candidates) {
            int classNumber = previousState.getBundleClasses().get(bunOld);
            if (bunNew.size() == bunOld.size() && !bundleClasses.containsValue(classNumber)) {
                // finally check for lambdasubbundle, as this is the most expensive check
                if (bunNew.hasAsLambdaSubBundle(bunOld, epsilon * lambdaFactor)) {
                    // Continuation of old bundle
                    bundleClasses.put(bunNew, classNumber);
                    return true;
                }
            }
        }

        return false;
    }

    private void handleFirstState(Set<Bundle> result, BiMap<Bundle, Integer> bundleClasses, Set<Integer> births) {
        // No previous states yet to compare to
        Log.log(LogLevel.STATUS, "FirstState", "Adding %d bundles.", result.size());
        for (Bundle b : result) {
            // Each bundle its own class
            bundleClasses.put(b, nextClassNumber);

            // Current birth moment
            births.add(nextClassNumber);

            // Note, we do not add any merges, as no previous clss is present.
            nextClassNumber++;
        }
    }

    @Override
    public void abort() {
        super.abort(); //To change body of generated methods, choose Tools | Templates.
        algo.abort();
    }

    /**
     * Enum indicating ways to increment epsilon.
     */
    private enum IncrType {
        Additive,
        Multiplicative,
        Hybrid
    }

}

class Pair<K, V> implements Serializable {
    private static final long serialVersionUID = -7115961138233562435L;
    K k;
    V v;

    public Pair(K k, V v) {
        this.k = k;
        this.v = v;
    }
}

class FindAllBundles implements Callable<Pair<Set<Bundle>, Map<Bundle, Bundle>>> {

    List<Trajectory> trajectories;
    BundleGenerationAlgorithm lambdaAlgo;

    FindAllBundles(List<Trajectory> trajectories, BundleGenerationAlgorithm lambdaAlgo) {
        this.trajectories = trajectories;
        this.lambdaAlgo = lambdaAlgo;
    }


    @Override
    public Pair<Set<Bundle>, Map<Bundle, Bundle>> call() throws Exception {
        Set<Bundle> result;
        Log.log(LogLevel.STATUS, EvolutionDiagramBuilder.LOGTAG, "Finding all bundles.");
        result = lambdaAlgo.run(trajectories);
        Log.log(LogLevel.STATUS, EvolutionDiagramBuilder.LOGTAG, "Found %d bundles.", result.size());
        Pair<Set<Bundle>, Map<Bundle, Bundle>> p = new Pair<Set<Bundle>, Map<Bundle, Bundle>>(result, lambdaAlgo.getMerges());

        return p;
    }


}
	
	
	
