package mapconstruction.web;

import com.google.common.collect.BiMap;
import com.google.common.collect.HashBiMap;
import com.google.common.math.DoubleMath;
import mapconstruction.GUI.io.*;
import mapconstruction.GUI.io.roadmap.RoadmapConsumer;
import mapconstruction.GUI.io.roadmap.RoadmapToKML;
import mapconstruction.GUI.io.roadmap.RoadmapToSavedState;
import mapconstruction.GUI.io.roadmap.RoadmapToVertexEdge;
import mapconstruction.GUI.listeners.*;
import mapconstruction.algorithms.diagram.EvolutionDiagram;
import mapconstruction.algorithms.maps.ComputeRoadNetwork;
import mapconstruction.algorithms.maps.mapping.RoadMap;
import mapconstruction.algorithms.maps.network.MapEdge;
import mapconstruction.algorithms.maps.network.MapVertex;
import mapconstruction.algorithms.maps.network.RoadNetwork;
import mapconstruction.algorithms.preprocessing.CompositePreprocessor;
import mapconstruction.algorithms.preprocessing.SegmentationPreprocessor;
import mapconstruction.algorithms.preprocessing.SimplificationPreprocessor;
import mapconstruction.algorithms.preprocessing.StraightenerPreprocessor;
import mapconstruction.algorithms.segmentation.HeadingSegmenter;
import mapconstruction.algorithms.segmentation.SelfSimilaritySementer;
import mapconstruction.algorithms.segmentation.TrajectorySegmenter;
import mapconstruction.algorithms.simplification.SimplificationMethod;
import mapconstruction.algorithms.simplification.TrajectorySimplifier;
import mapconstruction.algorithms.straightener.TrajectoryStraightener;
import mapconstruction.log.Log;
import mapconstruction.log.LogLevel;
import mapconstruction.trajectories.Bundle;
import mapconstruction.trajectories.FullTrajectory;
import mapconstruction.trajectories.Subtrajectory;
import mapconstruction.trajectories.Trajectory;
import mapconstruction.util.Pair;
import mapconstruction.web.config.BundleConfig;
import mapconstruction.web.config.DatasetConfig;
import mapconstruction.web.config.GeneralConfig;
import mapconstruction.web.config.YamlConfigRunner;
import mapconstruction.workers.AbortableAlgorithmWorker;
import mapconstruction.workers.ComputeBundleCutoff;
import mapconstruction.workers.ComputeEvolutionDiagram;
import mapconstruction.workers.ComputeGroundTruthCutoff;
import org.yaml.snakeyaml.DumperOptions;
import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.constructor.Constructor;

import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import java.awt.geom.Rectangle2D;
import java.io.*;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.stream.Collectors;

import static mapconstruction.GUI.datastorage.ConstantsStorage.ALGOCONSTANTS;
import static mapconstruction.GUI.datastorage.DataStorage.STORAGE;

/**
 * Controller handles the logic between the API and the actual implementation of the algorithms.
 * Furthermore it handles quite a lot of data flow.
 *
 * @author Jorrick
 * @since 05/11/2018
 */
public class Controller {

    SaveStateDiagramAndNetworkListener saveStateListener;
    /**
     * Keeps track of the time for the save state controllers.
     */
    private long timingStart = System.currentTimeMillis();
    private TxtTrajectoryReader txtTrajReader;
    private IpeTrajectoryReader ipeTrajReader;
    private FileNameExtensionFilterExt txtFilter;
    private FileNameExtensionFilterExt ipeFilter;
    private GeneralConfig generalConfig;
    private SavedStatesIndexer savedStatesIndexer;
    private BenchmarkManager benchmarkManager;
    private OutputManager outputManager;
    private boolean ignoreDirection;
    private boolean useSimplifier;
    private boolean simplifierGreedy;
    private boolean simplifierRDP;
    private boolean walkingDataset;
    private int simplifierError;
    private boolean useSegmenter;
    private boolean segmenterHeading;
    private boolean segmenterSelfSim;
    private boolean cutOffTrajectoryEndings;
    private int segmenterHeadingAngle;
    private int segmenterDistSelfSim;
    private AbortableAlgorithmWorker currentWorker;
    private ComputeRoadNetwork computeRoadNetwork;

    private List<RoadmapConsumer> roadmapConsumers;

    public Controller(GeneralConfig generalConfig) {
        txtFilter = new FileNameExtensionFilterExt(new FileNameExtensionFilter("Text file", "txt"));
        ipeFilter = new FileNameExtensionFilterExt(new FileNameExtensionFilter("Ipe document", "ipe", "xml"));

        txtTrajReader = new TxtTrajectoryReader();
        ipeTrajReader = new IpeTrajectoryReader();

        roadmapConsumers = Arrays.asList(
            new RoadmapToSavedState(),
            new RoadmapToVertexEdge(),
            new RoadmapToKML()
        );

        this.generalConfig = generalConfig;
        savedStatesIndexer = new SavedStatesIndexer(generalConfig.getSavedStatesDirectory());
        outputManager = new OutputManager(generalConfig.getOutputDirectory());
        benchmarkManager = new BenchmarkManager(generalConfig.getBenchmarkDirectory());

        ignoreDirection = true;
        useSimplifier = false;
        simplifierGreedy = false;
        simplifierRDP = false;
        simplifierError = 3;

        useSegmenter = false;
        segmenterHeading = false;
        segmenterSelfSim = false;
        segmenterHeadingAngle = 90;
        segmenterDistSelfSim = 20;

        cutOffTrajectoryEndings = true;
        walkingDataset = false;
        computeRoadNetwork = null;

        ALGOCONSTANTS.setNumThreads(generalConfig.getNumOfProcesses());

        initLog();
    }

    /**
     * Enable walking dataset
     * @param enableWalkingProperties true if we should enable the properties for a walking dataset, false for car
     *                                or anything else.
     */
    public void enableWalkingProperties(boolean enableWalkingProperties){

        if (enableWalkingProperties){
            walkingDataset = true;
            enableSimplifier(simplifierError);
        } else {
            walkingDataset = false;
            useSimplifier = false;
        }
    }

    public void setCutOffRepresentatives(boolean enableCutOff){
        cutOffTrajectoryEndings = false;
        ALGOCONSTANTS.setEnableCutOff(enableCutOff);
    }

    /**
     * Set the use of a segmenter
     * @param segmenterEpsilon
     */
    public void setUseSegmenter(int segmenterEpsilon){
        useSegmenter = true;
        segmenterSelfSim = true;
        segmenterDistSelfSim = segmenterEpsilon;
    }

    /**
     * Starting the logger
     */
    private void initLog() {
        // init log
        Log.addLogUser(new FileLogger());
        new Thread(Log.instance()).start();
    }

    /**
     * To check whether the datasetConfig is set.
     *
     * @return true if the datasetConfig is set, false otherwise.
     */
    public boolean isDatasetConfigSet() {
        return STORAGE.getDatasetConfig() != null;
    }

    /**
     * Function to load in a Dataset.
     *
     * @param datasetDir
     */
    public void loadDataset(String datasetDir) {
        if (!isDatasetConfigSet()) {
            DatasetExplorer datasetExplorer = new DatasetExplorer(generalConfig.getDatasetDirectory());
            if (datasetExplorer.getAllDatasets().indexOf(datasetDir) < 0) {
                System.out.println("Error! This dataset does not exist.");
            }


            File configFile = datasetExplorer.getConfigFileInDataset(datasetDir);

            File[] txtFiles = datasetExplorer.getAllFilesInDataset(datasetDir);
            loadTrajectories(txtFiles);

            Map<String, Pair<File,File>> roadmaps = datasetExplorer.getRoadmapFiles(datasetDir);
            loadRoadmaps(roadmaps);

            try {
                STORAGE.setDatasetConfig(YamlConfigRunner.getDatasetConfig(configFile));
                STORAGE.getDatasetConfig().setPath(datasetDir);
                STORAGE.getDatasetConfig().setWalkingDataset(walkingDataset);
                Log.log(LogLevel.INFO, "APIService", "Loaded dataset : %s", datasetDir);
            } catch (Exception e) {
                e.printStackTrace();
                Log.log(LogLevel.ERROR, "APIService", "Dataset not existent: %s", datasetDir);
                System.out.println("Unable to load in the dataset. Please make sure the dataset config is correct.");
            }
        }

    }

    /**
     * Given a bunch of files, it loads in all these trajectories
     *
     * @param files all the files that should be loaded in.
     */
    public void loadTrajectories(File[] files) {
        // Add files to the list
        // Load all trajectories
        List<Trajectory> trajs = new ArrayList<>();
        for (File f : files) {
            TrajectoryReader reader;
            if (txtFilter.accept(f)) {
                // text file
                reader = txtTrajReader;
            } else if (ipeFilter.accept(f)) {
                reader = ipeTrajReader;
            } else {
                Log.log(LogLevel.WARNING, "Control", "Skipped over unsupported file: %s", f.getName());
                continue;
            }

            trajs.addAll(reader.parse(f));

            Log.log(LogLevel.INFO, "Control", "Trajectories from %s added", f.getName());
        }
        STORAGE.clearBundles();
        STORAGE.setEvolutionDiagram(null);
        STORAGE.setOriginalTrajectories(trajs);
        STORAGE.setTrajectories(trajs);
        preProcess();
    }

    public void loadRoadmaps(Map<String,Pair<File,File>> roadmaps) {
        Map<String,RoadMap> networks = new LinkedHashMap<>();

        for (String roadmap : roadmaps.keySet()) {
            Pair<File,File> veFiles = roadmaps.get(roadmap);

            RoadNetwork network = new RoadNetwork(false);
            Map<Integer,MapVertex> vertices = new HashMap<>();
            try (BufferedReader reader = new BufferedReader(new FileReader(veFiles.getFirst()))) {
                for (String line; (line = reader.readLine()) != null;) {
                    String[] values = line.split(",");
                    vertices.put(Integer.parseInt(values[0]), new MapVertex(Double.parseDouble(values[1]), Double.parseDouble(values[2]), null, false));
                }
            } catch (IOException e) {
                e.printStackTrace();
            }

            try (BufferedReader reader = new BufferedReader(new FileReader(veFiles.getSecond()))) {
                for (String line; (line = reader.readLine()) != null;) {
                    String[] values = line.split(",");
                    MapVertex v1 = vertices.get(Integer.parseInt(values[1]));
                    MapVertex v2 = vertices.get(Integer.parseInt(values[2]));
                    network.addEdge(new MapEdge(Integer.parseInt(values[0]), v1, v2));
                }
            } catch (IOException e) {
                e.printStackTrace();
            }

            networks.put(roadmap, new RoadMap(network));
        }
        STORAGE.setComputedRoadmaps(networks);
    }

    /**
     * Enable simplifier
     */
    public void enableSimplifier(int simplifyError) {
        if (simplifyError <= 0) {
            return;
        }
        this.simplifierError = simplifyError;
        simplifierRDP = true;
        useSimplifier = true;
    }

    /**
     * Function to get the trajectory simplifier as configured.
     *
     * @return TrajectorySimplificationMethod or null
     */
    private TrajectorySimplifier getSimplifier() {
        if (simplifierGreedy) {
            return SimplificationMethod.Greedy;
        } else if (simplifierRDP) {
            return SimplificationMethod.RDP;
        }
        return null;
    }

    /**
     * Function to get the trajectory segmenter as configured.
     *
     * @return TrajectorySegmenter
     */
    private TrajectorySegmenter getSegmenter() {
        if (segmenterHeading) {
            double angle = (double) segmenterHeadingAngle;
            return HeadingSegmenter.degrees(angle);
        } else if (segmenterSelfSim) {
            double dist = (double) segmenterDistSelfSim;
            return new SelfSimilaritySementer(dist, 2 * dist, ignoreDirection);
        }
        return null;
    }

    /**
     * This function pre-processes the trajectories.
     * It create a pipeline after which the specific preprocessors are added.
     */
    public void preProcess() {
        CompositePreprocessor pipeline = new CompositePreprocessor();
        double error = (double) simplifierError;
        if (useSimplifier && !DoubleMath.fuzzyEquals(error, 0, 1E-6)) {
            pipeline.add(new SimplificationPreprocessor(getSimplifier(), error));
        }
        if (walkingDataset){
            pipeline.add(new StraightenerPreprocessor(new TrajectoryStraightener(50.0)));
        }
        if (useSegmenter) {
            pipeline.add(new SegmentationPreprocessor(getSegmenter()));
        }
        if (pipeline.numOfPreprocessors() > 0) {
            STORAGE.setTrajectories(pipeline.run(STORAGE.getTrajectories()));
        }
    }

    /**
     * Raises an error if the current worker is running.
     */
    public void raiseErrorIfWorkerRunning() throws ArithmeticException {
        if (currentWorker != null) {
            if (currentWorker.getState() != SwingWorker.StateValue.DONE) {
                throw new ArithmeticException("A worker is still running.");
            }
        }
    }

    /**
     * Getting whether the worker is still busy
     */
    public boolean isWorkerBusy() {
        return currentWorker != null && !currentWorker.isDone();
    }

    /**
     * Instantiates the computation of the bundle evolution diagram.
     */
    public void computeBundlesEvolutionDiagram() {
        attachListener();
        raiseErrorIfWorkerRunning();
        currentWorker = new ComputeEvolutionDiagram();
        currentWorker.run();

        STORAGE.addBundleListener(new BundleChangeListener() {
            @Override
            public void displayedBundlesChanged(BundleChangeEvent evt) {
                System.out.println("BUNDLES DISPLAY CHANGED: " + evt.getBundles().size());
            }

            @Override
            public void bundlesChanged(BundleChangeEvent evt) {
                System.out.println("BUNDLES CHANGED: " + evt.getBundles().size());
                STORAGE.setProgressAlgorithm(100);
            }
        });

        while (STORAGE.getAllUnfilteredBundles().size() == 0) {
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        STORAGE.setProgressAlgorithm(100);

        System.out.println("SAVING STATE...");
        String date = new SimpleDateFormat("yyyy-MM-dd_HH-mm").format(new Date());
        String walking = STORAGE.getDatasetConfig().isWalkingDataset()? "walking_" : "car_";
        String fileName = "bundles_" + walking + STORAGE.getDatasetConfig().getPath() + "_" + date;
        saveBundleState(new File(savedStatesIndexer.getNewSavedStateFilePath(fileName)));
//        Unnecessary as DataStorage does take care of this.
//        calculatePropertiesOfAllBundles();
    }

    /**
     * Initiates the computation of the bundle evolution diagram and afterwards the road network.
     */
    public void computeBundlesAndRoadMap() {
        computeBundlesEvolutionDiagram();
        computeTheRoadMap();
    }

    /**
     * Compute the newly created roadNetwork
     */
    public void computeTheRoadMap() {
        attachListener();
        if (computeRoadNetwork == null){
            this.computeRoadNetwork = new ComputeRoadNetwork();
            STORAGE.setRoadMap(computeRoadNetwork.getRoadMap());

            String date = new SimpleDateFormat("yyyy-MM-dd_HH-mm").format(new Date());
            String walking = STORAGE.getDatasetConfig().isWalkingDataset()? "walking_" : "car_";
            String fileName = "roadmap_" + walking + STORAGE.getDatasetConfig().getPath() + "_" + date;
            String state = savedStatesIndexer.getNewSavedStateFilePath(fileName);
            for (RoadmapConsumer consumer : this.roadmapConsumers) {
                consumer.consume(state, computeRoadNetwork.getRoadMap());
            }

//            outputManager.saveRoadMap(STORAGE.getDatasetConfig().getPath(), new Date(), computeRoadNetwork.getRoadMap());
            System.out.println("Done computing road network");
        }
    }

    public void computeBundleCutoff(Rectangle2D cutoff) {
        raiseErrorIfWorkerRunning();
        currentWorker = new ComputeBundleCutoff(cutoff);
        currentWorker.run();
        currentWorker.addPropertyChangeListener((event) -> {
            if ("state".equals(event.getPropertyName()) && SwingWorker.StateValue.DONE == event.getNewValue()) {
                String date = new SimpleDateFormat("yyyy-MM-dd_HH-mm").format(new Date());
                String walking = STORAGE.getDatasetConfig().isWalkingDataset()? "walking_" : "car_";
                String fileName = "bundles_" + walking + STORAGE.getDatasetConfig().getPath() + "_" + date;
                saveBundleState(new File(savedStatesIndexer.getNewSavedStateFilePath(fileName)));

                STORAGE.setProgressAlgorithm(100);
            }
        });
    }

    public void computeGroundTruthCutoff(double epsilon, GeneralConfig config) {
        currentWorker = new ComputeGroundTruthCutoff(epsilon, config);
        currentWorker.run();
    }

    public ComputeRoadNetwork returnTheRoadNetworkComputer() {
        return computeRoadNetwork;
    }

    /**
     * Attach the listener
     */
    private void attachListener() {
        if (saveStateListener == null) {
            saveStateListener = new SaveStateDiagramAndNetworkListener();
            STORAGE.addDiagramListener(saveStateListener);
            STORAGE.addNetworkListeners(saveStateListener);
        }
    }

    /**
     * This function loads all relevant variables into a file.
     * <p>
     * This stores the current state of the program into a file, such that we can set all these variables back later.
     * This function is called when the algorithm for calculating the road network is finished.
     *
     * @param out, the file where we write the state too. (Note, this is not the path, this is defined somewhere else)
     */
    private void saveState(File out) {
        try (ObjectOutputStream writer = new ObjectOutputStream(new FileOutputStream(out))) {

            writer.writeObject(STORAGE.getDatasetConfig());
            writer.writeObject(STORAGE.getOriginalTrajectories());
            writer.writeObject(STORAGE.getTrajectories());
            writer.writeObject(STORAGE.getEvolutionDiagram());
            writer.writeObject(this.computeRoadNetwork);
            ParameterSerializable params = ParameterSerializable.create();
            writer.writeObject(params);

            Log.log(LogLevel.INFO, "Control", "State exported to: %s", out.getAbsolutePath());
        } catch (IOException ex) {
            Logger.getLogger(Controller.class.getName()).log(Level.SEVERE, null, ex);
            Log.log(LogLevel.ERROR, "Control", "Failed to export state: %s", ex.toString());
        }
    }

    private void saveBundleState(File out) {
        BundleConfig config = new BundleConfig();
        config.setDataset(STORAGE.getDatasetConfig().getPath());
        config.setFilemask("bundle_(\\d+)\\.txt");
        config.setIsWalking(STORAGE.getDatasetConfig().isWalkingDataset());

        DumperOptions options = new DumperOptions();
        options.setDefaultFlowStyle(DumperOptions.FlowStyle.BLOCK);

        try {
            if (!out.mkdirs()) return;

            new Yaml(options).dump(config.asMap(), new FileWriter(new File(out, "bundleconfig.yml")));

            EvolutionDiagram ed = STORAGE.getEvolutionDiagram();
//            Set<Bundle> relevantBundles = STORAGE.getDisplayedBundles();
//                                           : STORAGE.getAllBundlesWithClassesUnfiltered()
            int i = 0; // j = 0;
            for (Map.Entry<Bundle,Integer> e : STORAGE.getAllBundlesWithClasses().entrySet()) {
                Bundle b = e.getKey();
                int bundleClass = e.getValue();
//                boolean isRelevant = relevantBundles.contains(b);
//                File bundleFile = new File(out, (isRelevant ? "bundle_" + (i++) : "ibundle_" + (j++)) + ".txt");
                File bundleFile = new File(out, "bundle_" + (i++) + ".txt");

                try (FileWriter writer = new FileWriter(bundleFile)) {
                    writer.write(bundleClass + "," + b.size() + "," + ed.getBestEpsilon(bundleClass) + "," + ed.getBirthMoment(bundleClass) + ","  + ed.getMergeMoment(bundleClass) +"\n");
                    Subtrajectory rep = b.getOriginalRepresentative();
                    writer.write(((FullTrajectory) rep.getParent()).getId() + "," + rep.getFromIndex() + "," + rep.getToIndex() + "\n");
                    for (Subtrajectory t : b.getSubtrajectories()) {
                        FullTrajectory parent = (FullTrajectory) t.getParent();
                        writer.write(parent.getId() + "," + t.getFromIndex() + "," + t.getToIndex() + "\n");
                    }
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        Log.log(LogLevel.INFO, "Control", "Bundles exported to: %s", out.getAbsolutePath());
    }

    public void timer(String s) {
        long timingEnd = System.currentTimeMillis();
        float elapsedTimeSec = (timingEnd - timingStart) / 1000F;
        timingStart = timingEnd;

        String str = Float.toString(elapsedTimeSec) + " - " + s;
        System.out.println(str);
        Log.log(LogLevel.INFO, "Control", str);
    }

    /**
     * This function loads all the relevant variables from the saved state back into the storage.
     *
     * @param in, the File
     */
    public void loadState(File in) {
        try (ObjectInputStream reader = new ObjectInputStream(new FileInputStream(in))) {
            timer("Controller - Starting the loading state");
            DatasetConfig datasetConfig = (DatasetConfig) reader.readObject();
            List<Trajectory> originalTrajectories = (List<Trajectory>) reader.readObject();
            List<Trajectory> trajectories = (List<Trajectory>) reader.readObject();
            EvolutionDiagram diagram = (EvolutionDiagram) reader.readObject();
            ComputeRoadNetwork roadMapComputer = (ComputeRoadNetwork) reader.readObject();
            timer("Controller - Loaded the files into main memory");

            try {
                ((ParameterSerializable) reader.readObject()).restore(this);
            } catch (EOFException ex) {
                Log.log(LogLevel.WARNING, "Control", "Parameters could not be restored");
            }

            STORAGE.setDatasetConfig(datasetConfig);
            STORAGE.setOriginalTrajectories(originalTrajectories);
            STORAGE.setTrajectories(trajectories);
            STORAGE.setEvolutionDiagram(diagram);
            this.computeRoadNetwork = roadMapComputer;
            if (this.computeRoadNetwork != null) {
                STORAGE.setRoadMap(computeRoadNetwork.getRoadMap());
            }

            enableWalkingProperties(STORAGE.getDatasetConfig().isWalkingDataset());

            timer("Controller - Set all trajectories and the evolution diagram");

            AbortableAlgorithmWorker lastWorker = new ComputeEvolutionDiagram();
            ((ComputeEvolutionDiagram) lastWorker).updateDiagramBundles();

            System.out.println("Controller - Computing all bundle properties.");
            Log.log(LogLevel.INFO, "Control", "Computing all bundle properties");

            STORAGE.setFilter(((ComputeEvolutionDiagram) lastWorker).predicate);
            timer("Controller - Applied the trajectory filter");

            timer("Controller - Calculated all bundle properties");
            Log.log(LogLevel.INFO, "Control", "State imported from: %s", in.getAbsolutePath());
        } catch (IOException | ClassNotFoundException ex) {
            Logger.getLogger(Controller.class.getName()).log(Level.SEVERE, null, ex);
            Log.log(LogLevel.ERROR, "Control", "Failed to import state: %s", ex.toString());
        }
    }

    public void loadBundleState(File in) {
        try {
            Yaml yaml = new Yaml(new Constructor(BundleConfig.class));
            BundleConfig config = yaml.load(new FileReader(new File(in, "bundleconfig.yml")));

            loadDataset(config.getDataset());
            BiMap<Integer,Bundle> bundles = HashBiMap.create();
//            BiMap<Integer,Bundle> iBundles = HashBiMap.create();
            Map<Integer,Double> bestEpsMapping = new HashMap<>();
            Map<Integer,Double> birthMoments = new HashMap<>();
            Map<Integer,Double> mergeMoments = new HashMap<>();
            //Arrays.stream( ... ).sorted(Comparator.comparing(File::getName)).collect(Collectors.toList())
            for (File bundle : Objects.requireNonNull(in.listFiles(config.filemask()))) {
                try (BufferedReader reader = new BufferedReader(new FileReader(bundle))) {
                    String[] header = reader.readLine().split(",");
                    if (header.length < 4) continue;
                    int k = Integer.parseInt(header[1]);
                    String[] r = reader.readLine().split(",");
                    Subtrajectory representative = STORAGE.getTrajectory(Long.parseLong(r[0]), Double.parseDouble(r[1]), Double.parseDouble(r[2]));
                    List<Subtrajectory> trajectories = new ArrayList<>(k);
                    for (int i = 0; i < k; i++) {
                        String[] t = reader.readLine().split(",");
                        trajectories.add(STORAGE.getTrajectory(Long.parseLong(t[0]), Double.parseDouble(t[1]), Double.parseDouble(t[2])));
                    }
                    int bundleClass = Integer.parseInt(header[0]);
                    double epsilon = Double.parseDouble(header[2]);
                    double birthMoment = Double.parseDouble(header[3]);
                    double mergeMoment = header.length > 4 ? Double.parseDouble(header[4]) : Double.NaN;
                    Bundle b = new Bundle(trajectories, representative);
                    // We assume bundle representative was cutoff before saving the state
                    b.setBundleRepCutOff(true);
                    bundles.put(bundleClass, b);
//                    iBundles.put(bundleClass, b);
                    bestEpsMapping.put(bundleClass, epsilon);
                    birthMoments.put(bundleClass, birthMoment);
                    if (mergeMoment != Double.NaN)
                        mergeMoments.put(bundleClass, mergeMoment);
                }
            }
//            for (File ibundle : Objects.requireNonNull(in.listFiles(config.filemask("i")))) {
//                try (BufferedReader reader = new BufferedReader(new FileReader(ibundle))) {
//                    String[] header = reader.readLine().split(",");
//                    if (header.length != 5) continue;
//                    int k = Integer.parseInt(header[1]);
//                    String[] r = reader.readLine().split(",");
//                    Subtrajectory representative = STORAGE.getTrajectory(Long.parseLong(r[0]), Double.parseDouble(r[1]), Double.parseDouble(r[2]));
//                    List<Subtrajectory> trajectories = new ArrayList<>();
//                    for (int i = 0; i < k; i++) {
//                        String[] t = reader.readLine().split(",");
//                        trajectories.add(STORAGE.getTrajectory(Long.parseLong(t[0]), Double.parseDouble(t[1]), Double.parseDouble(t[2])));
//                    }
//                    int bundleClass = Integer.parseInt(header[0]);
//                    double epsilon = Double.parseDouble(header[2]);
//                    double birthMoment = Double.parseDouble(header[3]);
//                    double mergeMoment = Double.parseDouble(header[4]);
//                    Bundle b = new Bundle(trajectories, representative);
//                    b.setBundleRepCutOff(true);
//                    iBundles.put(bundleClass, b);
//                    bestEpsMapping.put(bundleClass, epsilon);
//                    birthMoments.put(bundleClass, birthMoment);
//                    mergeMoments.put(bundleClass, mergeMoment);
//                }
//            }

            // Order DOES matter, HashBiMap seems to preserve order of insertion so we reinsert all bundles in order.
            BiMap<Bundle,Integer> sorted = HashBiMap.create();
            for (Integer bundleClass : bundles.keySet().stream().sorted().collect(Collectors.toList())) {
                sorted.put(bundles.get(bundleClass), bundleClass);
            }

            STORAGE.setEvolutionDiagram(new EvolutionDiagram(bestEpsMapping, birthMoments, mergeMoments));
//            STORAGE.setBundlesWithClasses(iBundles.inverse(), sorted);
            STORAGE.setBundlesWithClasses(sorted, false);
            STORAGE.getDatasetConfig().setWalkingDataset(config.getIsWalking());
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * Function to delete all objects and to restart as with a fresh install
     */
    public void resetToTheStart() {
        STORAGE.setDatasetConfig(null);
        STORAGE.setOriginalTrajectories(new ArrayList<>());
        STORAGE.setTrajectories(new ArrayList<>());
        STORAGE.clearBundles();
        STORAGE.setEvolutionDiagram(null);
        if (computeRoadNetwork != null){
            computeRoadNetwork = null;
            STORAGE.setRoadMap(null);
        }

        currentWorker = new ComputeEvolutionDiagram();
    }

    /**
     * Get an instance of the benchmark manager pointing at the configured benchmark directory
     */
    public BenchmarkManager getBenchmarkManager() {
        return benchmarkManager;
    }

    /**
     * Helper class containing all configured parameters in serializable form.
     * <p>
     * Can be used to easily restore parameters.
     */
    private static class ParameterSerializable implements Serializable {

        private static final long serialVersionUID = 1L;

        private double sweep_size;

        private ParameterSerializable() {

        }

        public static ParameterSerializable create() {
            ParameterSerializable o = new ParameterSerializable();
            o.sweep_size = 0.0;
            return o;
        }

        public void restore(Controller controller) {
//            Skipp
        }
    }

////////////////////////////////////////////////////////////////////////////////
//////////////////////// Utility inner classes /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

    /**
     * Class to save the state of the program when the RoadNetwork was changed.
     */
    private class SaveStateDiagramAndNetworkListener implements NetworkChangeListener, DiagramChangeListener {
        @Override
        public void diagramChanged(DiagramChangeEvent evt) {
            String date = new SimpleDateFormat("yyyy-MM-dd_HH-mm").format(new Date());
            String walking = STORAGE.getDatasetConfig().isWalkingDataset()? "walking_" : "car_";
            String fileName = "diagram_" + walking + STORAGE.getDatasetConfig().getPath() + "_" + date + ".savst";
            saveState(new File(savedStatesIndexer.getNewSavedStateFilePath(fileName)));
        }

        @Override
        public void networkChanged(NetworkChangeEvent evt) {
//            DISABLED! There is a very weird error when running from this saved state.
//            Probably has to do with the fact that not everything from the intersection is stored, but who knows..
//            String date = new SimpleDateFormat("yyyy-MM-dd_HH:mm").format(new Date());
//            String walking = STORAGE.getDatasetConfig().isWalkingDataset()? "walking_" : "car_";
//            String fileName = "network_" + walking +  STORAGE.getDatasetConfig().getPath() + "_" + date + ".savst";
//            saveState(new File(savedStatesIndexer.getNewSavedStateFilePath(fileName)));
        }
    }
}
