"""
Scrip that interfaces between our input / output formats
and the ones used by the java code.
"""
from collections import defaultdict
import re


"""
Ensure that this directory exists and does not contain leftover .txt and .yml files"
"""
def prepare_directory(directory_path):
    if directory_path.exists():
        for file in directory_path.glob('*.txt'):
            # print(file)
            file.unlink()  # deletes the .txt file
    else:
        directory_path.mkdir()

    yml_path = directory_path / "dataset-config.yml"
    with yml_path.open('w') as yml_file:
        # config data for athens_small
        print("system: UTM", file=yml_file)
        print("zone: 16", file=yml_file)
        print("hemisphere: N", file=yml_file)


"""
Read the .txt input file in our input format.
Create the necessary .txt and .yml files in data_subfolder for the java code.
"""
def input_to_java(input_file_path, data_subfolder_path, concatenate_trajectories=False):
    trajectories = defaultdict(list)
    with input_file_path.open('r') as infile:
        for line in infile.readlines():
            values = line.strip().split()
            # format: <id> <time> <lat> <long>
            assert len(values) == 4
            id, t = map(int, values[:2])
            lat, long = map(float, values[2:])
            trajectories[id].append((t, lat, long))

    prepare_directory(data_subfolder_path)

    if concatenate_trajectories:
        with (data_subfolder_path / "trip_0.txt").open('w') as trip_file:
            prev_id = -1
            for id, points in trajectories.items():
                assert prev_id <= id
                prev_id = id
                points.sort()
                for t, lat, long in points:
                    print(f"{lat} {long} {t}", file=trip_file)
    else:
        for id, points in trajectories.items():
            points.sort()
            with (data_subfolder_path / f"trip_{id}.txt").open('w') as trip_file:
                for t, lat, long in points:
                    print(f"{lat} {long} {t}", file=trip_file)


"""
Among all java log file, find the latest one that was a run on this data set.
(Hopefully this is the one the caller is looking for.)
"""
def latest_matching_logfile(java_root_path, data_set_name):
    log_files = [(path.stem, path) for path in (java_root_path / 'log').glob("*.log")]
    # check files from newest to oldest
    log_files.sort(reverse=True, key=lambda e: e[0])
    for name, path in log_files:
        with path.open('r') as infile:
            for line in infile.readlines():
                if f"Loaded dataset : {data_set_name}" in line:
                    return path


def save_state_path(java_root_path, log_file_path):
    with log_file_path.open('r') as infile:
        for line in infile.readlines():
            # Bundles exported to: /me/savst/bundles_car_athens_small_2024-06-10_10-23
            match = re.search('Bundles exported to: /me/([^\\s]+)$', line)
            if match:
                print(match)
                return java_root_path / match.group(1)


"""
Extract running time without IO from log file.
Wall time is a bad metric as a lot of time is spent writing the .savst file.
The ComputeEVO time seems to be pretty accurate.
"""
def running_time(log_file_path):
    with log_file_path.open('r') as infile:
        for line in infile.readlines():
            match = re.search("\\[ComputeEVO\\] Total running time:\\s+([0-9]+) ms", line)
            if match:
                return float(match.group(1))
    return None


def reformat_bundle(bundle_file_path, outfile):
    def parse_bundle(line):
        id, l, r = line.split(',')
        """
        Java bundles have floating point endpoints and might be reversed (negative id).
        We round the enpoints to get subtrajectories.
        We run a modified version of the java code that doesn't reverse subtrajectories.
        Also, java ids are 1-based whereas our ids are 0-based.
        """
        assert int(id) >= 1
        return int(id) - 1, round(float(l)), round(float(r))
        # return abs(int(id)) - 1, round(float(l)), round(float(r))

    with bundle_file_path.open("r") as infile:
        """
        first line is
            <class_id> <num_subtrajectories> <epsilon> <birth> <death>
        but we don't need this
        """
        infile.readline()
        # reference subtrajectory
        print(*parse_bundle(infile.readline()), file=outfile)
        # clustered subtrajectories
        for bundle in map(parse_bundle, infile.readlines()):
            print(*bundle, file=outfile, end=' ')
        print(file=outfile)


def reformat_output(save_state_folder, data_set_name, time_ms, output_file_path):
    if time_ms is None:
        return # no output. The run might have timed out.

    with open(output_file_path, 'w') as outfile:
        """
        <data set>
        <program> <args>
        <time ms>
        """
        print(data_set_name, file=outfile)
        print("java javafy.py", file=outfile)
        print(time_ms / 1000, file=outfile)
        # sometimes java fails to write the bundles. Then we only report the time.
        if save_state_folder:
            bundle_files = [(int(re.search("bundle_([0-9]+)", path.stem).group(1)), path) for path in save_state_folder.glob("*.txt")]
            bundle_files.sort(key=lambda e: e[0])
            for id, bundle_file_path in bundle_files:
                reformat_bundle(bundle_file_path, outfile)


"""
Extracts an output in our format from the latest save state of a java run on this data set.
"""
def output_from_java(java_root_path, data_set_path, output_file_path):
    log_file_path = latest_matching_logfile(java_root_path, data_set_path.stem)
    print(f"Guessed log file: {log_file_path}")
    save_state_folder = save_state_path(java_root_path, log_file_path)
    time_ms = running_time(log_file_path)

    reformat_output(save_state_folder, data_set_path.name, time_ms, output_file_path)

