"""
Visualization script that builds a heatmap from a trajectories file.

Input format
    Each line is of the form <trajID> <timestamp> <latitude> <longitude>
        <trajID>    trajectory id, integer starting with 0
        <time>      POSIX timestamp, 64 bit integer
        <lat>       latitude, float
        <long>      longitude, float
"""
from sys import stderr

import plotly.express as px
import plotly.graph_objects as go

from script.common import find_file, parse_clusters, parse_data_set

def visualize_old(infile):
    data = []
    for line in infile.readlines():
        tokens = line.strip().split()
        lat, long = map(float, tokens[2:])
        # if 39 <= lat <= 41 and 115 <= long <= 118:
        # if 39.4 <= lat <= 40.8 and 115.8 <= long <= 117.4:  # figure 1 in data set paper
        if 39.75 <= lat <= 40.1 and 116.15 <= long <= 116.6:  # figure 2 in data set paper
            data.append({'lat': lat, 'long': long})

    fig = px.density_heatmap(data, y='lat', x='long', range_color=[0, 1000])
    fig.show()



def visualize_trajectories(trajectories):
    """
    Visualize the data set with plotly.
    """
    data_points = [{'trajectory': traj_id, 't': t, 'x': long, 'y': lat} for traj_id, v in trajectories.items() for t, lat, long in v]
    fig = px.line(data_points, x='x', y='y', color='trajectory', text='t')
    fig.update_traces(textposition='bottom right')
    fig.show()


def visualize_clusters(trajectories, header, clusters, title, ref_only, fast_cluster):
    """
    Visualize the data set and a clustering with plotly.
    """
    fig = go.Figure()
    # draw trajectories in dark gray
    first = True
    for traj_id, v in trajectories.items():
        data = [{'trajectory': traj_id, 't': t, 'x': long, 'y': lat} for t, lat, long in v]
        x = [e['x'] for e in data]
        y = [e['y'] for e in data]
        fig.add_trace(go.Scatter(x=x, y=y, name='trajectories', legendgroup='trajectories', showlegend=first, line_color='#666666'))
        first = False

    def extract_points(traj, l, r):
        x, y = zip(*((long, lat) for t, lat, long in trajectories[traj][l:r+1]))
        return {'x': x, 'y': y}
    # draw pathlets in bright colors and covers in dark colors
    for i, pathlet_cover in enumerate(clusters.items()):
        pathlet, cover = pathlet_cover
        hue = i * 240 // len(clusters)
        first = True
        if not ref_only:
            if fast_cluster:
                points = {coord: [pt for subtrajectory in cover if subtrajectory != pathlet for pt in extract_points(*subtrajectory)[coord]] for coord in 'xy'}
                fig.add_trace(go.Scatter(points, name=f'sub {i}', legendgroup=f'subtrajectory {i}', showlegend=first, line_color=f'hsl({hue},50%,30%)'))
            else:
                for subtrajectory in cover:
                    if subtrajectory != pathlet:
                        fig.add_trace(go.Scatter(**extract_points(*subtrajectory), name=f'sub {i}', legendgroup=f'subtrajectory {i}', showlegend=first, line_color=f'hsl({hue},50%,30%)'))
                        first = False

        # draw pathlet last so that is appears on top
        fig.add_trace(go.Scatter(**extract_points(*pathlet), name=f'pathlet {i}', line_color=f'hsl({hue},100%,50%)'))

    fig.update_layout(
        title=dict(text=title, font=dict(size=30))
    )
    fig.show()


def main(args):
    file_path = find_file(args.file, ensure_unique=True)
    if args.cluster:
        with file_path.open('r') as infile:
            header, clusters = parse_clusters(infile)

        trajectory_file_path = find_file(header[0])
        with trajectory_file_path.open('r') as infile:
            trajectories = parse_data_set(infile)

        if args.ignoreboundaries:
            trajectories = {0: [p for trajectory in trajectories.values() for p in trajectory]}

        visualize_clusters(trajectories, header, clusters, file_path.name, ref_only=args.refonly, fast_cluster = args.fastcluster)
    else:
        with file_path.open('r') as infile:
            trajectories = parse_data_set(infile)

        visualize_trajectories(trajectories)
