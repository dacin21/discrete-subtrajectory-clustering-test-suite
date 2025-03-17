import argparse

"""
We avoid importing the indivisual modules unless they are being run.
This reduces issues due to specific libraries not being instaled.
For example, plotly is only used for visualization.
"""
def add_parser_fred(subparsers):
    def main_fred(args):
        import script.fred as fred
        fred.main(args)

    parser = subparsers.add_parser('fred', description='Similar to reduces, but uses Fred.curves.simplify')
    parser.set_defaults(fun=main_fred)

    parser.add_argument('filename')
    parser.add_argument('factor', type=int, help='keep every factor-th point on a trajectory.')


def add_parser_plot(subparsers):
    def main_plot(args):
        import script.plot as plot
        plot.main(args)

    parser = subparsers.add_parser('plot', description='generate a plot based on output and log files.')
    parser.set_defaults(fun=main_plot)

    parser.add_argument('plot_name')


def add_parser_reduce(subparsers):
    def main_reduce(args):
        import script.reduce as reduce
        reduce.main(args)

    parser = subparsers.add_parser('reduce', description='reduce (or increase) data set size by various means')
    parser.set_defaults(fun=main_reduce)

    parser.add_argument('filename')
    parser.add_argument('t', type=int, help='keep 1 of t trajectories')
    parser.add_argument('p', type=int, help='keep every k-th point on a trajectory')


def add_parser_run(subparsers):
    def main_run(args):
        import script.run as run
        run.main(args)

    parser = subparsers.add_parser('run', description='run algorithm(s) with parameter(s) on data files.')
    parser.set_defaults(fun=main_run)

    parser.add_argument('-c', '--cluster', action='store_true', help='cluster file instead of trajectory file')
    parser.add_argument('job_name')


def add_parser_score(subparsers):
    def main_score(args):
        import script.score as score
        score.main(args)

    parser = subparsers.add_parser('score', description='compute score of a clustering')
    parser.add_argument('-c', nargs=3, type=float, help='score function constants c1 c2 c3')
    parser.add_argument('-o', '--old_score', action='store_true', help='use old coverage score, based on trajectory lengths.')
    parser.add_argument('-t', '--concatenate', action='store_true', help='Concatenate all trajectories into a single one.')
    parser.set_defaults(fun=main_score)

    parser.add_argument('file')


def add_parser_visualize(subparsers):
    def main_visualize(args):
        import script.visualize as visualize
        visualize.main(args)

    parser = subparsers.add_parser('visualize', description='visualize trajectories and/or clusters')
    parser.set_defaults(fun=main_visualize)

    parser.add_argument('-c', '--cluster', action='store_true', help='cluster file instead of trajectory file')
    parser.add_argument('-f', '--fastcluster', action='store_true', help='use faster cluster visualization')
    parser.add_argument('-r', '--refonly', action='store_true', help='plot only the reference trajectories, not the whole clusters')
    parser.add_argument('-i', '--ignoreboundaries', action='store_true', help='Ignore trajectory boundaries: treat everything as a single trajectory with id 0.')
    parser.add_argument('file')


def build_argument_parser():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(required=True)

    add_parser_fred(subparsers)
    add_parser_plot(subparsers)
    add_parser_reduce(subparsers)
    add_parser_run(subparsers)
    add_parser_score(subparsers)
    add_parser_visualize(subparsers)

    return parser
