"""
Script that extracts bits of data from output and log files
and creates plots based on this data.
"""
from math import sqrt
import os
import re
import sys

from script.common import find_file, format_floats, frechet_distance_squared, git_root_directory, parse_clusters, parse_data_set, strip_directory
from script.score import compute_score, score_clusters


def get_data_set(out_file):
    with open(out_file) as outfile:
        return strip_directory(outfile.readline().strip())


def get_num_points(input_file):
    with input_file.open('r') as infile:
        num_points = len(infile.readlines())
        print(f"{input_file.stem} has {num_points} points.")
        return num_points
        # this is really slow:
        # data_set = parse_data_set(infile)
        # return sum(len(points) for points in data_set.values())


"""
Time as reported by the algorithm, in ms.
Does not include IO.
"""
def get_time(out_file):
    with out_file.open('r') as infile:
        infile.readline()
        infile.readline()
        return float(infile.readline())


"""
Total time as measured by /usr/bin/time
"""
def get_raw_time(log_file):
    with log_file.open('r') as infile:
        for line in infile.readlines():
            if match := re.search("User time \\(seconds\\)\\: ([0-9\\.]+)", line):
                return float(match.group(1))


def get_memory(log_file):
    with log_file.open('r') as infile:
        for line in infile.readlines():
            if match := re.search('Maximum resident set size \\(kbytes\\): ([0-9]+)', line):
                # convert kbytes to gbytes
                return int(match.group(1)) / 1000000



"""
Try to guess the constants c1 c2 c3 based on the output file
Doesn't work for java, should work for the other programs.
"""
def deduce_c(input_file):
    patterns = [
        "rightstep (?P<c1>[0-9\\.e\\-]+) (?P<c2>[0-9\\.e\\-]+) (?P<c3>[0-9\\.e\\-]+)",
        "basic_length (?P<c1>[0-9\\.e\\-]+) (?P<c2>[0-9\\.e\\-]+) (?P<c3>[0-9\\.e\\-]+) [0-9\\.e\\-]+ [0-9\\.e\\-]+ (?P<l>[0-9\\.e\\-]+) (?P<s>[0-9\\.e\\-]+)",
        "agarwal (?P<c1>[0-9\\.e\\-]+) (?P<c3>[0-9\\.e\\-]+) (?P<c2>[0-9\\.e\\-]+)",
    ]
    with input_file.open('r') as infile:
        for line in infile.readlines():
            for pattern in patterns:
                if match := re.search(pattern, line):
                    groups = match.groupdict()
                    c = tuple(map(float, (match.group("c1"), match.group("c2"),  match.group("c3"),  groups.get('l', 'nan'),  groups.get('s', 'nan'))))
                    print(f"deduced score constants: {c}")
                    return c

    print(f"failed to deduce score constants")
    return None

def get_table_stats(input_file, out_file, *, c_means=None, c_centers=None, c_fallback=None):
    with input_file.open('r') as infile:
        trajectories = parse_data_set(infile)
    with out_file.open('r') as outfile:
        _, clusters = parse_clusters(outfile)

    stats = {}
    stats['num_clusters'] = len(clusters)

    deduce_c_means = (c_means == 'deduce')
    deduce_c_centers = (c_centers == 'deduce')
    if deduce_c_means:
        c_means = deduce_c(out_file) or c_fallback
        stats['c'] = c_means
    if deduce_c_centers:
        c_centers = deduce_c(out_file) or c_fallback
        stats['c'] = c_centers


    distances = [sqrt(frechet_distance_squared(trajectories, reference, covered)) for reference, subtrajectories in clusters.items() for covered in subtrajectories]
    pos_distances = [d for d in distances if d > 0]
    stats['max_frechet'] = (max(distances) if distances else float('-inf'))
    stats['avg_frechet'] = (sum(pos_distances) / len(pos_distances) if pos_distances else float('nan'))
    if c_means:
        stats['score_means'] = score_clusters(trajectories, None, clusters, c=c_means[:3], centers=False)
    if c_centers:
        stats['score_centers'] = score_clusters(trajectories, None, clusters, c=c_centers[:3], centers=True)

    cluster_sizes = [len(subtrajectories) for subtrajectories in clusters.values()]
    stats['max_cluster'] = (max(cluster_sizes) if cluster_sizes else float('-inf'))
    stats['avg_cluster'] = (sum(cluster_sizes) / len(cluster_sizes) if cluster_sizes else float('nan'))

    print(f"stats: {stats}")

    return stats


def float_error(x, y):
    denom = max(1, abs(x), abs(y))
    return abs(x - y) / denom


def get_score(input_file, out_file, *, c_fallback=None, centers=False):
    with input_file.open('r') as infile:
        trajectories = parse_data_set(infile)
    with out_file.open('r') as outfile:
        _, clusters = parse_clusters(outfile)

    c = deduce_c(out_file)
    if c and c_fallback:
        assert all(float_error(x, y) < 1e-6 for x,y in zip(c, c_fallback))
    c = c or c_fallback

    return score_clusters(trajectories, None, clusters, c=c[:3], centers=centers)


def get_data_point(run_name, index, mode, **kwargs):
    out_file = find_file(f"{run_name}_{index}.out", must_exist=False)
    log_file = find_file(f"{run_name}_{index}.log")
    input_file = out_file and find_file(get_data_set(out_file))

    if not out_file:
        print(f"No ouput file: {run_name}_{index}")
        if mode != 'table':
            return '---'
        return {label: '---' for label in ['num_clusters', 'c', 'max_frechet', 'avg_frechet', 'score_means', 'score_centers', 'max_cluster', 'avg_cluster']}
    else:
        print(f"Found: {run_name}_{index}")


    match mode:
        case 'points':
            return get_num_points(input_file)

        case 'raw_time':
            return get_raw_time(log_file)
        case 'time':
            return get_time(out_file)
        case 'memory':
            return get_memory(log_file)
        case 'score':
            return get_score(input_file, out_file, **kwargs)

        case 'table':
            return get_table_stats(input_file, out_file, **kwargs)

        case _:
            assert False, f"Invalid data type: {mode}"


def get_data(run_name, index_range, modes, **kwargs):
    if isinstance(modes, str):
        return [get_data_point(run_name, i, modes, **kwargs) for i in index_range]
    # support mode tuples
    return list(zip(*(get_data(run_name, index_range, mode, **kwargs) for mode in modes)))


def write_tikz_plot(name, outfile, data, step, *, label_x='', label_y='', params=[], newline=False, semilog=False, legend='inline'):
        def plot_one_line(legend, points):
            return ''.join(("""
            \\addplot+[only marks] coordinates { """, ' '.join(map(str, points)), """};
            \\addlegendentry{""", str(legend), """};"""))

        params_str = ''.join("""
                """ + e + "," for e in params)
        plot_str = ''.join(("""\
        \\begin{tikzpicture}
            \\begin{""", "semilogyaxis" if semilog else "axis", """}[
                title={},
                xlabel={""", label_x, """},
                ylabel={""", label_y, """},
                legend pos=north west,""", ("""
                legend to name=""" + str(name) + "_legend,") if legend != 'inline' else "", """
                legend columns=""", str(1 if legend == 'inline' else len(data)), """,
                scaled ticks=false,""",
                params_str, """
            ]
            """, '\n'.join(plot_one_line(legend, points) for legend, points in data.items()), """
            """, "\\legend{};" if not legend else "", """

            \\end{""", "semilogyaxis" if semilog else "axis", """}
        \\end{tikzpicture}%"""))

        # if step > 0:
        #     print("""        \\hfill""", file=outfile)
        print(plot_str, file=outfile)
        if newline:
            print(file=outfile)
        if legend == 'delayed':
            print("""\
        \\begin{tikzpicture}
            \\ref*{""" + str(name) + """_legend}
        \\end{tikzpicture}""", file=outfile)


def write_table(name, outfile, data, step, *, newline=False):
    num_rows = len(next(iter(data.values())))

    def row_str(row_index):
        return ' & '.join(str(column[row_index]) for column in data.values())

    plot_str = ''.join(("""\
    \\begin{tabular}{@{}""", 'l', 'r'*(len(data) - 1), """@{}}
        \\toprule
        """, ' & '.join(map(str, data.keys())), """\\\\
        \\midrule
        """,
        '\\\\\n'.join(row_str(row) for row in range(num_rows)),
        """\\\\
        \\bottomrule
    \\end{tabular}"""))

    print(plot_str, file=outfile)
    if newline:
        print(file=outfile)


def group_positions(data, width):
    groups = dict()
    for g in data['group']:
        if g not in groups:
            groups[g] = 0
        groups[g] += width
    sum = 0
    for g in groups.keys():
        groups[g] += sum
        sum = groups[g] + width

    return groups


def group_by_names(list_of_names):
    names = dict()
    for i, name in enumerate(list_of_names):
        if name not in names:
            names[name] = []
        names[name].append(i)
    return names


def populate_bar_plot(fig, ax, data, metric, *, width=1.0, legend='upper left', x_label='c2', yaxis=None, title=None, use_item_color=False):
    row = data[metric]
    yaxis = yaxis or metric
    title = title or metric

    groups = group_positions(data, width)
    print(groups)
    names = group_by_names(data['name'])
    def get_x(i):
        g = data['group'][i]
        x = groups[g]
        groups[g] += width
        return x

    for name, ids in names.items():
        color_dict = {'color': list(map(data['color'].__getitem__, ids))} if use_item_color else dict()
        ax.bar(list(map(get_x, ids)), list(map(data[metric].__getitem__, ids)), width, label=name, **color_dict)

    ax.set_ylabel(yaxis)
    ax.set_xlabel(x_label)
    ax.set_title(title)
    ax.legend(loc=legend)
    # ax.set_ylim(0, 1)
    print([(x + groups[name] - width)/2 for name, x in group_positions(data, width).items()], groups.keys())
    ax.set_xticks([(x + groups[name] - width)/2 for name, x in group_positions(data, width).items()], groups.keys())


def write_bar_plot(name, texfile, data, step, *, metric, dpi=300, fig_size=None, logplot=False, **kwargs):
    import matplotlib.pyplot as plt

    plot_file_name = f"{name}_{step}_{metric}.png"
    plot_file_path = git_root_directory() / 'data' / 'plots' / plot_file_name

    plt.tight_layout()
    fig, ax = plt.subplots()
    populate_bar_plot(fig, ax, data, metric, **kwargs)
    if logplot:
        ax.set_yscale('log')
    if fig_size:
        fig.set_size_inches(fig_size)
    fig.savefig(plot_file_path, dpi=dpi, bbox_inches='tight')


def write_plot(name, outfile, mode, data, step, **kwargs):
    match mode:
        case 'bar':
            write_bar_plot(name, outfile, data, step, **kwargs)

        case 'table':
            data = format_floats(data)
            write_table(name, outfile, data, step, **kwargs)

        case 'tikz':
            write_tikz_plot(name, outfile, data, step, **kwargs)

        case _:
            assert f"Invalid plot type: {mode}"


def main(args):
    sys.path.insert(1, os.path.realpath(os.path.pardir))
    from data.plots import get_plots

    with open(git_root_directory() / 'data' / 'plots' / f"{args.plot_name}.tex", "w") as outfile:
        print("""
    \\begin{figure} \\begin{center}""", file=outfile)
        step = 0

        def plot(mode, data, **kwargs):
            nonlocal step
            write_plot(args.plot_name, outfile, mode, data, step, **kwargs)
            step += 1

        caption = get_plots(args.plot_name, get_data, plot)

        if caption and caption != '':
            print("""
        \\caption{""", caption, "}", file=outfile, sep='')

        print("""
    \\end{center} \\end{figure}""", file=outfile)
