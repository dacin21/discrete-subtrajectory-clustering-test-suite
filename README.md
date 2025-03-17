# Subtrajectory clustering test suite

## Prerequisites
A modern Linux system with
- Podman
- Python3 + mathplotlib
- Latex

## Setup
Run
```
python3 main.py run compile
```
Check the files `data/results/compile_N.log` for 0 <= N <= 3 to see that everything compiled successfully.
**Warning** Compilation can fail randomly because the build system sometimes just doesn't find a library. It is important to check the log files until everything compiles. This might take like 5 attempts.

## Doing a run

To execute a run, first do
```
python3 main.py run <run_name>
```
This produces a bunch of log and output files in `data/results`.
Then, do
```
python3 main.py plot <plot_name>
```
This parses the log and output files, and creates TeX files (tables) or .png files (bar plots) in `data/plots`.

## Notes

- For the BBGLL comparison, `bbgll` is SC-m, `java-bbgll` is Map-construct, `movetk` is MoveTK.
- For clustering, `simple-length` is SC-$\ell$, `simple-size` is SC-m, `rightstep` is PSC, `agarwal` is Envelope, `java` is Map-construct.
- Some plots combine data from multiple runs. In many cases, you can generate the plot based on a subset of this data. Non-existant runs will be listed as `---` in a table, or DNF in a bar plot.
- MoveTK uses up to 120GB of memory. You can edit `movetk/podman/podman.sh` to set a smaller memory limit. 
- You can edit `data/jobs.py` to set a smaller time limit (default is 1 day), or skip individual commands.
- You can edit `data/plots.py` to remove individual runs from the plots.

# List of runs

The following runs replicate the figures from our paper. The timings are based on fast modern hardware.
The `<name>_small` runs use less time and/or memory, but only generate a single data point of a plot.

## BBGLL
### Athens table (fast)
```
python3 main.py run socg_bbgll_athens
python3 main.py plot socg_table_bbgll_athens
cd data/plots
```
### Chicago-4 table (high memory)
```
python3 main.py run socg_bbgll_chicago_four
python3 main.py plot socg_table_bbgll_chicago_four
cd data/plots
```
### Berlin-10 table (high memory)
```
python3 main.py run socg_bbgll_berlin_ten
python3 main.py plot socg_table_bbgll_berlin_ten
cd data/plots
```

### Bar plot (high memory)
```
python3 main.py run socg_bbgll_athens
python3 main.py run socg_bbgll_chicago_four
python3 main.py run socg_bbgll_berlin_ten
python3 main.py run socg_bbgll_synth
python3 main.py plot socg_logbar_bbgll_memory
python3 main.py plot socg_logbar_bbgll_time
cd data/plots
```

## Clustering

### Athens kMeans table (fast)
```
python3 main.py run socg_table_athens
python3 main.py plot socg_table_athens
cd data/plots
```

### Athens kCenters table (fast)
```
python3 main.py run socg_table_athens_centers
python3 main.py plot socg_table_athens_centers
cd data/plots
```

### Chicago-4 kMeans table
```
python3 main.py run socg_table_chicago_four
python3 main.py plot socg_table_chicago_four
cd data/plots
```

### Chicago-4 kCenters table
```
python3 main.py run socg_table_chicago_four_centers
python3 main.py plot socg_table_chicago_four_centers
cd data/plots
```

### Chicago kMeans table (slow)
```
python3 main.py run socg_table_chicago
python3 main.py plot socg_table_chicago
cd data/plots
```

### Chicago kCenters table (slow)
```
python3 main.py run socg_table_chicago_four
python3 main.py plot socg_table_chicago_four
cd data/plots
```

### Berlin-10 kMeans table
```
python3 main.py run socg_table_berlin_ten
python3 main.py plot socg_table_berlin_ten
cd data/plots
```

### Berlin-10 kCenters table
```
python3 main.py run socg_table_berlin_ten_centers
python3 main.py plot socg_table_berlin_ten_centers
cd data/plots

```
### Berlin kMeans table (slow)
```
python3 main.py run socg_table_berlin
python3 main.py plot socg_table_berlin
cd data/plots
```

### Berlin kCenters table (slow)
```
python3 main.py run socg_table_berlin_centers
python3 main.py plot socg_table_berlin_centers
cd data/plots
```

### Drifter kMeans table (slow)
This only runs `rightstep`. All other implementations time out.
```
python3 main.py run socg_table_drifter
python3 main.py plot socg_table_drifter
cd data/plots
```

### Drifter kCenters table (slow)
This only runs `rightstep`. All other implementations time out.
```
python3 main.py run socg_table_drifter_centers
python3 main.py plot socg_table_drifter_centers
cd data/plots
```

### UniD kMeans table (slow)
```
python3 main.py run socg_table_unid
python3 main.py plot socg_table_unid
cd data/plots
```

### Drifter kCenters table (slow)
```
python3 main.py run socg_table_unid_centers
python3 main.py plot socg_table_unid_centers
cd data/plots
```

### Synthetic kMeans tables
```
python3 main.py run socg_table_snyth_one
python3 main.py plot socg_table_snyth_one_A
python3 main.py plot socg_table_snyth_one_B
python3 main.py plot socg_table_snyth_one_C
cd data/plots
```
```
python3 main.py run socg_table_snyth_two
python3 main.py plot socg_table_snyth_two_A
python3 main.py plot socg_table_snyth_two_B
python3 main.py plot socg_table_snyth_two_C
cd data/plots
```

### kMeans bar plots (very slow)
```
python3 main.py run socg_table_athens
python3 main.py run socg_table_berlin_ten
python3 main.py run socg_table_chicago_four
python3 main.py run socg_table_chicago
python3 main.py run socg_table_synth_one
python3 main.py run socg_table_synth_two
python3 main.py plot socg_bar_time
python3 main.py plot socg_logbar_time
python3 main.py plot socg_bar_memory
python3 main.py plot socg_logbar_memory
python3 main.py plot socg_bar_score
python3 main.py plot socg_logbar_score
cd data/plots
```

## File Formats
### Data Sets
We preprocess the data sets into the following format:
```
No header

Body: Every line decribes one point on a trajectory, in the following format

<trajID> <timestamp> <lat> <lon>

    <trajID>        integer, trajectory id, continguous starting from 0.
    <timestamp>     integer, UNIX timestamp, increasing along the trajectory.
    <lat>           float, latitude (y-coordinate)
    <long>          float, longitude (x-coordinate)
```
### Algorithm output
```
Header

<data set>
<algorithm> <param_1> ... <param_k>
<time>

    <data set>      string, filename of the data set
    <algorithm>     string, name of the algorithm
    <param_i>       string, parameter of the algorithm
    <time>          float, running time in seconds

Body: Two lines together describes one pathlet, in the following format

<traj> <l> <r>
<traj_1> <l_1> <r_1> <traj_2> <l_2> <r_2> ... <traj_n> <l_n> <r_n>

    <traj>          integer, id of the trajectory out of which the pathlet is constructed
    <l>, <r>        integer, the pathlet consists of the points [l, r] (zero based) of this trajectory.

    <traj_i>        integer, id of the trajectory this pathlet covers
    <l_i>, <r_i>    integer, on this trajectory, the points [l_i, r_i] (zero based) are covered by the pathlet.
```

