def get_plots(plot_name, get_data, write_plot):
    """
    Create one or more plots, based on plot_name.
    get_data(job_name, index_range, mode) returns [value_1, value_2, ...].
    write_plot(mode, data) creates a plot where data = {'line_name' : [(x1, y1), (x2, y2), ...]}
    See script/plot.py for valid job types.
    """
    match plot_name:
        case 'socg_table_athens':
            # [{'stat_name': stat}]
            for it in range(3):
                ids = list(range(2+11*it, 13+11*it)) + [1]
                data_set = 'socg_table_athens'
                stats = get_data(data_set, ids, 'table', c_means='deduce', c_centers=None, c_fallback=(1.0, 0.00003 * 10**it, 128))
                data = {
                    'Name': ['basic-length']*5 + ['basic-size']*4 + ['rightstep', 'agarwal', 'java'],
                    'Seconds': get_data(data_set, ids, 'time'),
                    'GBytes': get_data(data_set, ids, 'memory'),
                } | {
                    pretty: [stats[i][ugly] for i in range(len(ids))] for pretty, ugly in [('$c2$', 'c'), ('Clusters', 'num_clusters'), ('Max Frechet', 'max_frechet'), ('Avg Frechet', 'avg_frechet'), ('kMeans', 'score_means'), ('Max Size', 'max_cluster'), ('Avg Size', 'avg_cluster')]
                }
                print(data)
                write_plot('table', data)

            return "Various statistics on the clusterings obtained on Athens small with different parameters. kMeans only."

        case 'socg_table_athens_centers':
            # [{'stat_name': stat}]
            for it in range(3):
                ids = list(range(2+10*it, 2+10*(it+1))) + [1]
                data_set = 'socg_table_athens_centers'
                stats = get_data(data_set, ids, 'table', c_means=None, c_centers='deduce', c_fallback=(1.0, 0.03 * 10**it, 128))
                data = {
                    'Name': ['basic-length']*5 + ['basic-size']*4 + ['rightstep', 'java'],
                    'Seconds': get_data(data_set, ids, 'time'),
                    'GBytes': get_data(data_set, ids, 'memory'),
                } | {
                    pretty: [stats[i][ugly] for i in range(len(ids))] for pretty, ugly in [('$c2$', 'c'), ('Clusters', 'num_clusters'), ('Max Frechet', 'max_frechet'), ('Avg Frechet', 'avg_frechet'), ('kCenters', 'score_centers'), ('Max Size', 'max_cluster'), ('Avg Size', 'avg_cluster')]
                }
                print(data)
                write_plot('table', data)

            return "Various statistics on the clusterings obtained on Athens small with different parameters. kCenters only."

        case 'socg_table_chicago':
            # [{'stat_name': stat}]
            for it in range(3):
                ids = list(range(2+11*it, 13+11*it)) + [1]
                data_set = 'socg_table_chicago'
                stats = get_data(data_set, ids, 'table', c_means='deduce', c_centers=None, c_fallback=(1.0, 0.00003 * 10**it, 888))
                data = {
                    'Name': ['basic-length']*5 + ['basic-size']*4 + ['rightstep', 'agarwal', 'java'],
                    'Seconds': get_data(data_set, ids, 'time'),
                    'GBytes': get_data(data_set, ids, 'memory'),
                } | {
                    pretty: [stats[i][ugly] for i in range(len(ids))] for pretty, ugly in [('$c2$', 'c'), ('Clusters', 'num_clusters'), ('Max Frechet', 'max_frechet'), ('Avg Frechet', 'avg_frechet'), ('kMeans', 'score_means'), ('Max Size', 'max_cluster'), ('Avg Size', 'avg_cluster')]
                }
                print(data)
                write_plot('table', data)

            return "Various statistics on the clusterings obtained on Chicago with different parameters. kMeans only."

        case 'socg_table_chicago_centers':
            # [{'stat_name': stat}]
            for it in range(3):
                ids = list(range(10*it, 10*(it+1)))
                data_set = 'socg_table_chicago_centers'
                stats = get_data(data_set, ids, 'table', c_means=None, c_centers='deduce', c_fallback=(1.0, 0.1 * 10**it, 888))
                data = {
                    'Name': ['basic-length']*5 + ['basic-size']*4 + ['rightstep'],
                    'Seconds': get_data(data_set, ids, 'time'),
                    'GBytes': get_data(data_set, ids, 'memory'),
                } | {
                    pretty: [stats[i][ugly] for i in range(len(ids))] for pretty, ugly in [('$c2$', 'c'), ('Clusters', 'num_clusters'), ('Max Frechet', 'max_frechet'), ('Avg Frechet', 'avg_frechet'), ('kCenters', 'score_centers'), ('Max Size', 'max_cluster'), ('Avg Size', 'avg_cluster')]
                }
                print(data)
                write_plot('table', data)

            return "Various statistics on the clusterings obtained on Chicago with different parameters. kCenters only."


        case 'socg_table_chicago_four':
            # [{'stat_name': stat}]
            for it in range(3):
                ids = list(range(2+11*it, 13+11*it)) + [1]
                data_set = 'socg_table_chicago_four'
                stats = get_data(data_set, ids, 'table', c_means='deduce', c_centers=None, c_fallback=(1.0, 0.00003 * 10**it, 222))
                data = {
                    'Name': ['basic-length']*5 + ['basic-size']*4 + ['rightstep', 'agarwal', 'java'],
                    'Seconds': get_data(data_set, ids, 'time'),
                    'GBytes': get_data(data_set, ids, 'memory'),
                } | {
                    pretty: [stats[i][ugly] for i in range(len(ids))] for pretty, ugly in [('$c2$', 'c'), ('Clusters', 'num_clusters'), ('Max Frechet', 'max_frechet'), ('Avg Frechet', 'avg_frechet'), ('kMeans', 'score_means'), ('Max Size', 'max_cluster'), ('Avg Size', 'avg_cluster')]
                }
                print(data)
                write_plot('table', data)

            return "Various statistics on the clusterings obtained on Chicago with $1/4$ trajectories, with different parameters. kMeans only."

        case 'socg_table_chicago_four_centers':
            # [{'stat_name': stat}]
            for it in range(3):
                ids = list(range(10*it, 10*(it+1)))
                data_set = 'socg_table_chicago_four_centers'
                stats = get_data(data_set, ids, 'table', c_means=None, c_centers='deduce', c_fallback=(1.0, 0.1 * 10**it, 222))
                data = {
                    'Name': ['basic-length']*5 + ['basic-size']*4 + ['rightstep'],
                    'Seconds': get_data(data_set, ids, 'time'),
                    'GBytes': get_data(data_set, ids, 'memory'),
                } | {
                    pretty: [stats[i][ugly] for i in range(len(ids))] for pretty, ugly in [('$c2$', 'c'), ('Clusters', 'num_clusters'), ('Max Frechet', 'max_frechet'), ('Avg Frechet', 'avg_frechet'), ('kCenters', 'score_centers'), ('Max Size', 'max_cluster'), ('Avg Size', 'avg_cluster')]
                }
                print(data)
                write_plot('table', data)

            return "Various statistics on the clusterings obtained on Chicago with $1/4$ trajectories, with different parameters. kCenters only."

        case 'socg_table_drifter':
            ids = list(range(4))
            data_set = 'socg_table_drifter'
            stats = get_data(data_set, ids, 'table', c_means='deduce', c_centers=None)
            data = {
                'Name': ['rightstep']*4,
                'Seconds': get_data(data_set, ids, 'time'),
                'GBytes': get_data(data_set, ids, 'memory'),
            } | {
                pretty: [stats[i][ugly] for i in range(len(ids))] for pretty, ugly in [('$c2$', 'c'), ('Clusters', 'num_clusters'), ('Max Frechet', 'max_frechet'), ('Avg Frechet', 'avg_frechet'), ('kMeans', 'score_means'), ('Max Size', 'max_cluster'), ('Avg Size', 'avg_cluster')]
            }
            print(data)
            write_plot('table', data)

            return "Various statistics on the clusterings obtained on Drifter with different parameters. kMeans only."

        case 'socg_table_drifter_centers':
            ids = list(range(4))
            data_set = 'socg_table_drifter_centers'
            stats = get_data(data_set, ids, 'table', c_means='deduce', c_centers=None)
            data = {
                'Name': ['rightstep']*4,
                'Seconds': get_data(data_set, ids, 'time'),
                'GBytes': get_data(data_set, ids, 'memory'),
            } | {
                pretty: [stats[i][ugly] for i in range(len(ids))] for pretty, ugly in [('$c2$', 'c'), ('Clusters', 'num_clusters'), ('Max Frechet', 'max_frechet'), ('Avg Frechet', 'avg_frechet'), ('kMeans', 'score_means'), ('Max Size', 'max_cluster'), ('Avg Size', 'avg_cluster')]
            }
            print(data)
            write_plot('table', data)

            return "Various statistics on the clusterings obtained on Drifter with different parameters. kCenters only."

        case 'socg_table_unid':
            # [{'stat_name': stat}]
            for it in range(3):
                ids = list(range(2+11*it, 13+11*it)) + [1]
                data_set = 'socg_table_unid'
                stats = get_data(data_set, ids, 'table', c_means='deduce', c_centers=None, c_fallback=(1.0, 0.00003 * 10**it, 362))
                data = {
                    'Name': ['basic-length']*5 + ['basic-size']*4 + ['rightstep', 'agarwal', 'java'],
                    'Seconds': get_data(data_set, ids, 'time'),
                    'GBytes': get_data(data_set, ids, 'memory'),
                } | {
                    pretty: [stats[i][ugly] for i in range(len(ids))] for pretty, ugly in [('$c2$', 'c'), ('Clusters', 'num_clusters'), ('Max Frechet', 'max_frechet'), ('Avg Frechet', 'avg_frechet'), ('kMeans', 'score_means'), ('Max Size', 'max_cluster'), ('Avg Size', 'avg_cluster')]
                }
                print(data)
                write_plot('table', data)

            return "Various statistics on the clusterings obtained on UniD with different parameters. kMeans only."

        case 'socg_table_unid_centers':
            # [{'stat_name': stat}]
            for it in range(3):
                ids = list(range(10*it, 10*(it+1)))
                data_set = 'socg_table_unid_centers'
                stats = get_data(data_set, ids, 'table', c_means=None, c_centers='deduce', c_fallback=(1.0, 0.1 * 10**it, 362))
                data = {
                    'Name': ['basic-length']*5 + ['basic-size']*4 + ['rightstep'],
                    'Seconds': get_data(data_set, ids, 'time'),
                    'GBytes': get_data(data_set, ids, 'memory'),
                } | {
                    pretty: [stats[i][ugly] for i in range(len(ids))] for pretty, ugly in [('$c2$', 'c'), ('Clusters', 'num_clusters'), ('Max Frechet', 'max_frechet'), ('Avg Frechet', 'avg_frechet'), ('kCenters', 'score_centers'), ('Max Size', 'max_cluster'), ('Avg Size', 'avg_cluster')]
                }
                print(data)
                write_plot('table', data)

            return "Various statistics on the clusterings obtained on UniD with different parameters. kCenters only."

        case 'socg_table_berlin_ten':
            # [{'stat_name': stat}]
            for it in range(3):
                ids = list(range(2+11*it, 13+11*it)) + [1]
                data_set = 'socg_table_synth_one'
                stats = get_data(data_set, ids, 'table', c_means='deduce', c_centers=None, c_fallback=(1.0, [0.001, 0.0003, 0.0001][it], 2717))
                data = {
                    'Name': ['basic-length']*5 + ['basic-size']*4 + ['rightstep', 'agarwal', 'java'],
                    'Seconds': get_data(data_set, ids, 'time'),
                    'GBytes': get_data(data_set, ids, 'memory'),
                } | {
                    pretty: [stats[i][ugly] for i in range(len(ids))] for pretty, ugly in [('$c2$', 'c'), ('Clusters', 'num_clusters'), ('Max Frechet', 'max_frechet'), ('Avg Frechet', 'avg_frechet'), ('kMeans', 'score_means'), ('Max Size', 'max_cluster'), ('Avg Size', 'avg_cluster')]
                }
                print(data)
                write_plot('table', data)

            return "Various statistics on the clusterings obtained on the berlin data set with 1/10 trajectories and with different parameters. kMeans only."

        case 'socg_table_synth_one_A':
            # [{'stat_name': stat}]
            for it in range(3):
                zero = 0
                ids = list(range(zero+2+11*it, zero+13+11*it)) + [zero+1]
                data_set = 'socg_table_synth_one'
                stats = get_data(data_set, ids, 'table', c_means='deduce', c_centers=None, c_fallback=(1.0, [0.001, 0.0003, 0.0001][it], 50))
                data = {
                    'Name': ['basic-length']*5 + ['basic-size']*4 + ['rightstep', 'agarwal', 'java'],
                    'Seconds': get_data(data_set, ids, 'time'),
                    'GBytes': get_data(data_set, ids, 'memory'),
                } | {
                    pretty: [stats[i][ugly] for i in range(len(ids))] for pretty, ugly in [('$c2$', 'c'), ('Clusters', 'num_clusters'), ('Max Frechet', 'max_frechet'), ('Avg Frechet', 'avg_frechet'), ('kMeans', 'score_means'), ('Max Size', 'max_cluster'), ('Avg Size', 'avg_cluster')]
                }
                print(data)
                write_plot('table', data)

            return "Various statistics on the clusterings obtained on a synethic data set (200 inital points, 50 trajectores, 50\\% sampling rate) with different parameters. kMeans only."

        case 'socg_table_synth_one_B':
            # [{'stat_name': stat}]
            for it in range(3):
                zero = 35
                ids = list(range(zero+2+11*it, zero+13+11*it)) + [zero+1]
                data_set = 'socg_table_synth_one'
                stats = get_data(data_set, ids, 'table', c_means='deduce', c_centers=None, c_fallback=(1.0, [0.001, 0.0003, 0.0001][it], 50))
                data = {
                    'Name': ['basic-length']*5 + ['basic-size']*4 + ['rightstep', 'agarwal', 'java'],
                    'Seconds': get_data(data_set, ids, 'time'),
                    'GBytes': get_data(data_set, ids, 'memory'),
                } | {
                    pretty: [stats[i][ugly] for i in range(len(ids))] for pretty, ugly in [('$c2$', 'c'), ('Clusters', 'num_clusters'), ('Max Frechet', 'max_frechet'), ('Avg Frechet', 'avg_frechet'), ('kMeans', 'score_means'), ('Max Size', 'max_cluster'), ('Avg Size', 'avg_cluster')]
                }
                print(data)
                write_plot('table', data)

            return "Various statistics on the clusterings obtained on a synethic data set (200 inital points, 50 trajectores, 90\\% sampling rate) with different parameters. kMeans only."

        case 'socg_table_synth_one_C':
            # [{'stat_name': stat}]
            for it in range(3):
                zero = 70
                ids = list(range(zero+2+11*it, zero+13+11*it)) + [zero+1]
                data_set = 'socg_table_synth_one'
                stats = get_data(data_set, ids, 'table', c_means='deduce', c_centers=None, c_fallback=(1.0, [0.001, 0.0003, 0.0001][it], 50))
                data = {
                    'Name': ['basic-length']*5 + ['basic-size']*4 + ['rightstep', 'agarwal', 'java'],
                    'Seconds': get_data(data_set, ids, 'time'),
                    'GBytes': get_data(data_set, ids, 'memory'),
                } | {
                    pretty: [stats[i][ugly] for i in range(len(ids))] for pretty, ugly in [('$c2$', 'c'), ('Clusters', 'num_clusters'), ('Max Frechet', 'max_frechet'), ('Avg Frechet', 'avg_frechet'), ('kMeans', 'score_means'), ('Max Size', 'max_cluster'), ('Avg Size', 'avg_cluster')]
                }
                print(data)
                write_plot('table', data)

            return "Various statistics on the clusterings obtained on a synethic data set (200 inital points, 50 trajectores, 95\\% sampling rate) with different parameters. kMeans only."

        case 'socg_table_synth_two_A':
            # [{'stat_name': stat}]
            for it in range(3):
                zero = 0
                ids = list(range(zero+2+11*it, zero+13+11*it)) + [zero+1]
                data_set = 'socg_table_synth_two'
                stats = get_data(data_set, ids, 'table', c_means='deduce', c_centers=None, c_fallback=(1.0, [0.0003, 0.0001, 0.00003][it], 100))
                data = {
                    'Name': ['basic-length']*5 + ['basic-size']*4 + ['rightstep', 'agarwal', 'java'],
                    'Seconds': get_data(data_set, ids, 'time'),
                    'GBytes': get_data(data_set, ids, 'memory'),
                } | {
                    pretty: [stats[i][ugly] for i in range(len(ids))] for pretty, ugly in [('$c2$', 'c'), ('Clusters', 'num_clusters'), ('Max Frechet', 'max_frechet'), ('Avg Frechet', 'avg_frechet'), ('kMeans', 'score_means'), ('Max Size', 'max_cluster'), ('Avg Size', 'avg_cluster')]
                }
                print(data)
                write_plot('table', data)

            return "Various statistics on the clusterings obtained on a synethic data set (100 inital points, 100 trajectores, 50\\% sampling rate) with different parameters. kMeans only."

        case 'socg_table_synth_two_B':
            # [{'stat_name': stat}]
            for it in range(3):
                zero = 35
                ids = list(range(zero+2+11*it, zero+13+11*it)) + [zero+1]
                data_set = 'socg_table_synth_two'
                stats = get_data(data_set, ids, 'table', c_means='deduce', c_centers=None, c_fallback=(1.0, [0.0003, 0.0001, 0.00003][it], 100))
                data = {
                    'Name': ['basic-length']*5 + ['basic-size']*4 + ['rightstep', 'agarwal', 'java'],
                    'Seconds': get_data(data_set, ids, 'time'),
                    'GBytes': get_data(data_set, ids, 'memory'),
                } | {
                    pretty: [stats[i][ugly] for i in range(len(ids))] for pretty, ugly in [('$c2$', 'c'), ('Clusters', 'num_clusters'), ('Max Frechet', 'max_frechet'), ('Avg Frechet', 'avg_frechet'), ('kMeans', 'score_means'), ('Max Size', 'max_cluster'), ('Avg Size', 'avg_cluster')]
                }
                print(data)
                write_plot('table', data)

            return "Various statistics on the clusterings obtained on a synethic data set (100 inital points, 100 trajectores, 90\\% sampling rate) with different parameters. kMeans only."

        case 'socg_table_synth_two_C':
            # [{'stat_name': stat}]
            for it in range(3):
                zero = 70
                ids = list(range(zero+2+11*it, zero+13+11*it)) + [zero+1]
                data_set = 'socg_table_synth_two'
                stats = get_data(data_set, ids, 'table', c_means='deduce', c_centers=None, c_fallback=(1.0, [0.0003, 0.0001, 0.00003][it], 100))
                data = {
                    'Name': ['basic-length']*5 + ['basic-size']*4 + ['rightstep', 'agarwal', 'java'],
                    'Seconds': get_data(data_set, ids, 'time'),
                    'GBytes': get_data(data_set, ids, 'memory'),
                } | {
                    pretty: [stats[i][ugly] for i in range(len(ids))] for pretty, ugly in [('$c2$', 'c'), ('Clusters', 'num_clusters'), ('Max Frechet', 'max_frechet'), ('Avg Frechet', 'avg_frechet'), ('kMeans', 'score_means'), ('Max Size', 'max_cluster'), ('Avg Size', 'avg_cluster')]
                }
                print(data)
                write_plot('table', data)

            return "Various statistics on the clusterings obtained on a synethic data set (100 inital points, 100 trajectores, 95\\% sampling rate) with different parameters. kMeans only."

        case 'socg_table_bbgll_athens':
            data_set = 'socg_bbgll_athens'
            it = 2
            for distance in [50, 100, 200]:
                for size in [20, 40, 80]:
                    ids = list(range(it, it+4))[::-1]
                    it += 4
                    data = {
                        'Name': ['MoveTK', 'Map-construct', 'Map-construct-rtree', 'SC-$\\ell$'],
                        'Seconds': get_data(data_set, ids, 'time'),
                        'GBytes': get_data(data_set, ids, 'memory'),
                        '$\\Delta$': [distance]*len(ids),
                        '$m$': [size]*len(ids),
                    }
                    print(data)
                    write_plot('table', data)

            return "The four bbgll implementations on athens-small with different parameters."

        case 'socg_table_bbgll_chicago_four':
            data_set = 'socg_bbgll_chicago_four'
            it = 2
            for distance in [50, 100, 200]:
                for size in [40, 80, 160]:
                    ids = list(range(it, it+4))[::-1]
                    it += 4
                    data = {
                        'Name': ['MoveTK', 'Map-construct', 'Map-construct-rtree', 'SC-$\\ell$'],
                        'Seconds': get_data(data_set, ids, 'time'),
                        'GBytes': get_data(data_set, ids, 'memory'),
                        '$\\Delta$': [distance]*len(ids),
                        '$m$': [size]*len(ids),
                    }
                    print(data)
                    write_plot('table', data)

            return "The four bbgll implementations on chicago-4 with different parameters."

        case 'socg_table_bbgll_berlin_ten':
            data_set = 'socg_bbgll_berlin_ten'
            it = 2
            for distance in [50, 100, 200]:
                for size in [10, 20, 40]:
                    ids = list(range(it, it+4))[::-1]
                    it += 4
                    data = {
                        'Name': ['MoveTK', 'Map-construct', 'Map-construct-rtree', 'SC-$\\ell$'],
                        'Seconds': get_data(data_set, ids, 'time'),
                        'GBytes': get_data(data_set, ids, 'memory'),
                        '$\\Delta$': [distance]*len(ids),
                        '$m$': [size]*len(ids),
                    }
                    print(data)
                    write_plot('table', data)

            return "The four bbgll implementations on berlin-10 with different parameters."

        case 'socg_bar_athens':
            data_set = 'socg_table_athens'
            ids = []
            group = []
            data = {}
            for it in range(3):
                ids = list(range(2+11*it, 13+11*it)) + [1]
                c = (1, 0.00003 * 10**it, 128)


                data_block = {
                    'name': ['basic-length']*5 + ['basic-size']*4 + ['rightstep', 'agarwal', 'java'],
                    'group': ["{:.8f}".format(c[1]).rstrip('0')] * len(ids),
                    'seconds': get_data(data_set, ids, 'time'),
                    'gbytes': get_data(data_set, ids, 'memory'),
                    'score': get_data(data_set, ids, 'score', c_fallback=c)
                }
                for key, value in data_block.items():
                    if key not in data:
                        data[key] = []
                    data[key].extend(value)

            for metric, yaxis, title in [('seconds', 'Seconds', 'Time'), ('gbytes', 'GBytes', 'Memory'), ('score', 'Score (lower is better)', 'kMeans Score')]:
                write_plot('bar', data, metric=metric, yaxis=yaxis, title=title)

            return "Various statistics on the clusterings obtained on Athens small with different parameters. kMeans only."

        case 'socg_bar_chicago_four':
            data_set = 'socg_table_chicago_four'
            ids = []
            group = []
            data = {}
            for it in range(3):
                ids = list(range(2+11*it, 13+11*it)) + [1]
                c = (1, 0.00003 * 10**it, 222)

                data_block = {
                    'name': ['basic-length']*5 + ['basic-size']*4 + ['rightstep', 'agarwal', 'java'],
                    'group': ["{:.8f}".format(c[1]).rstrip('0')] * len(ids),
                    'seconds': get_data(data_set, ids, 'time'),
                    'gbytes': get_data(data_set, ids, 'memory'),
                    'score': get_data(data_set, ids, 'score', c_fallback=c)
                }
                for key, value in data_block.items():
                    if key not in data:
                        data[key] = []
                    data[key].extend(value)

            for metric, yaxis, title in [('seconds', 'Seconds', 'Time'), ('gbytes', 'GBytes', 'Memory'), ('score', 'Score (lower is better)', 'kMeans Score')]:
                write_plot('bar', data, metric=metric, yaxis=yaxis, title=title)

            return "Various statistics on the clusterings obtained on Athens small with different parameters. kMeans only."

        case 'socg_logbar_score':
            for it in range(3):
                data = dict()

                def add_data(data_block):
                    for key, value in data_block.items():
                        if key not in data:
                            data[key] = []
                        data[key].extend(value)

                def add_group(data_set, data_set_name, c, ids, names):
                    pretty_c2 = "$c_2=${:.8f}".format(c[1]).rstrip('0')
                    group_name = f"{data_set_name}\n{pretty_c2}"
                    data_block = {
                        'name': names,
                        'group': [group_name] * len(ids),
                        'score': [(0 if e == '---' else e) for e in get_data(data_set, ids, 'score', c_fallback=c)]
                    }
                    add_data(data_block)


                kmeans_names = ['SC-$\\ell$']*5 + ['SC-$m$']*4 + ['PSC', 'Envelope', 'Map-construct']
                kmeans_ids = list(range(2+11*it, 13+11*it)) + [1]
                synth_ids = lambda zero: list(range(zero+2+11*it, zero+13+11*it)) + [zero+1]
                c2 = 0.00003 * 10**it
                add_group('socg_table_athens', 'athens', (1.0, c2, 128), kmeans_ids, kmeans_names)
                add_group('socg_table_berlin_ten', 'berlin-10', (1.0, c2, 2717), kmeans_ids, kmeans_names)
                add_group('socg_table_chicago_four', 'chicago-4', (1.0, c2, 222), kmeans_ids, kmeans_names)
                c2 = [0.001, 0.0003, 0.0001][it]
                add_group('socg_table_synth_one', 'synth-A (50)', (1.0, c2, 50), synth_ids(0), kmeans_names)
                add_group('socg_table_synth_one', 'synth-A (90)', (1.0, c2, 50), synth_ids(35), kmeans_names)
                add_group('socg_table_synth_one', 'synth-A (95)', (1.0, c2, 50), synth_ids(70), kmeans_names)
                c2 = [0.0003, 0.0001, 0.00003][it]
                add_group('socg_table_synth_two', 'synth-B (50)', (1.0, c2, 100), synth_ids(0), kmeans_names)
                add_group('socg_table_synth_two', 'synth-B (90)', (1.0, c2, 100), synth_ids(35), kmeans_names)
                add_group('socg_table_synth_two', 'synth-B (95)', (1.0, c2, 100), synth_ids(70), kmeans_names)

                write_plot('bar', data, metric='score', yaxis='Score (lower is better)', title='kMeans Score (0 if DNF)', x_label=None, fig_size=(14, 15), logplot=True)

        case 'socg_logbar_time':
            for it in range(3):
                data = dict()

                def add_data(data_block):
                    for key, value in data_block.items():
                        if key not in data:
                            data[key] = []
                        data[key].extend(value)

                def add_group(data_set, data_set_name, c, ids, names):
                    pretty_c2 = "$c_2=${:.8f}".format(c[1]).rstrip('0')
                    group_name = f"{data_set_name}\n{pretty_c2}"
                    data_block = {
                        'name': names,
                        'group': [group_name] * len(ids),
                        'time': [(24*60*60 if e == '---' else e) for e in get_data(data_set, ids, 'time')]
                    }
                    add_data(data_block)


                kmeans_names = ['SC-$\\ell$']*5 + ['SC-$m$']*4 + ['PSC', 'Envelope', 'Map-construct']
                kmeans_ids = list(range(2+11*it, 13+11*it)) + [1]
                synth_ids = lambda zero: list(range(zero+2+11*it, zero+13+11*it)) + [zero+1]
                c2 = 0.00003 * 10**it
                add_group('socg_table_athens', 'athens', (1.0, c2, 128), kmeans_ids, kmeans_names)
                add_group('socg_table_berlin_ten', 'berlin-10', (1.0, c2, 2717), kmeans_ids, kmeans_names)
                add_group('socg_table_chicago_four', 'chicago-4', (1.0, c2, 222), kmeans_ids, kmeans_names)
                add_group('socg_table_chicago', 'chicago', (1.0, c2, 888), kmeans_ids, kmeans_names)
                # add_group('socg_table_drifter', 'drifter', (1.0, 0.1, ???), ???, ???)
                c2 = 0.001 * 10**it
                # add_group('socg_table_unid', 'UniD', (1.0, c2, 362), kmeans_ids, kmeans_names)
                c2 = [0.001, 0.0003, 0.0001][it]
                add_group('socg_table_synth_one', 'synth-A (50)', (1.0, c2, 50), synth_ids(0), kmeans_names)
                add_group('socg_table_synth_one', 'synth-A (90)', (1.0, c2, 50), synth_ids(35), kmeans_names)
                add_group('socg_table_synth_one', 'synth-A (95)', (1.0, c2, 50), synth_ids(70), kmeans_names)
                c2 = [0.0003, 0.0001, 0.00003][it]
                add_group('socg_table_synth_two', 'synth-B (50)', (1.0, c2, 100), synth_ids(0), kmeans_names)
                add_group('socg_table_synth_two', 'synth-B (90)', (1.0, c2, 100), synth_ids(35), kmeans_names)
                add_group('socg_table_synth_two', 'synth-B (95)', (1.0, c2, 100), synth_ids(70), kmeans_names)

                write_plot('bar', data, metric='time', yaxis='Seconds', title='Time (86400 if DNF)', x_label=None, fig_size=(14, 15), logplot=True)

        case 'socg_logbar_memory':
            for it in range(3):
                data = dict()

                def add_data(data_block):
                    for key, value in data_block.items():
                        if key not in data:
                            data[key] = []
                        data[key].extend(value)

                def add_group(data_set, data_set_name, c, ids, names):
                    pretty_c2 = "$c_2=${:.8f}".format(c[1]).rstrip('0')
                    group_name = f"{data_set_name}\n{pretty_c2}"
                    data_block = {
                        'name': names,
                        'group': [group_name] * len(ids),
                        'memory': [(0 if e == '---' or e is None else e) for e in get_data(data_set, ids, 'memory')]
                    }
                    add_data(data_block)


                kmeans_names = ['SC-$\\ell$']*5 + ['SC-$m$']*4 + ['PSC', 'Envelope', 'Map-construct']
                kmeans_ids = list(range(2+11*it, 13+11*it)) + [1]
                synth_ids = lambda zero: list(range(zero+2+11*it, zero+13+11*it)) + [zero+1]
                c2 = 0.00003 * 10**it
                add_group('socg_table_athens', 'athens', (1.0, c2, 128), kmeans_ids, kmeans_names)
                add_group('socg_table_berlin_ten', 'berlin-10', (1.0, c2, 2717), kmeans_ids, kmeans_names)
                add_group('socg_table_chicago_four', 'chicago-4', (1.0, c2, 222), kmeans_ids, kmeans_names)
                add_group('socg_table_chicago', 'chicago', (1.0, c2, 888), kmeans_ids, kmeans_names)
                # add_group('socg_table_drifter', 'drifter', (1.0, 0.1, ???), ???, ???)
                c2 = 0.001 * 10**it
                # add_group('socg_table_unid', 'UniD', (1.0, c2, 362), kmeans_ids, kmeans_names)
                c2 = [0.001, 0.0003, 0.0001][it]
                add_group('socg_table_synth_one', 'synth-A (50)', (1.0, c2, 50), synth_ids(0), kmeans_names)
                add_group('socg_table_synth_one', 'synth-A (90)', (1.0, c2, 50), synth_ids(35), kmeans_names)
                add_group('socg_table_synth_one', 'synth-A (95)', (1.0, c2, 50), synth_ids(70), kmeans_names)
                c2 = [0.0003, 0.0001, 0.00003][it]
                add_group('socg_table_synth_two', 'synth-B (50)', (1.0, c2, 100), synth_ids(0), kmeans_names)
                add_group('socg_table_synth_two', 'synth-B (90)', (1.0, c2, 100), synth_ids(35), kmeans_names)
                add_group('socg_table_synth_two', 'synth-B (95)', (1.0, c2, 100), synth_ids(70), kmeans_names)

                write_plot('bar', data, metric='memory', yaxis='GBytes', title='Memory (0 if DNF)', x_label=None, fig_size=(14, 15), logplot=True)

        case 'socg_bar_score':
            for it in range(3):
                data = dict()

                def add_data(data_block):
                    for key, value in data_block.items():
                        if key not in data:
                            data[key] = []
                        data[key].extend(value)

                def add_group(data_set, data_set_name, c, ids, names):
                    pretty_c2 = "$c_2=${:.8f}".format(c[1]).rstrip('0')
                    group_name = f"{data_set_name}\n{pretty_c2}"
                    data_block = {
                        'name': names,
                        'group': [group_name] * len(ids),
                        'score': [(0 if e == '---' or e is None else e) for e in get_data(data_set, ids, 'score', c_fallback=c)]
                    }
                    add_data(data_block)


                kmeans_names = ['SC-$\\ell$']*5 + ['SC-$m$']*4 + ['PSC', 'Envelope', 'Map-construct']
                kmeans_ids = list(range(2+11*it, 13+11*it)) + [1]
                synth_ids = lambda zero: list(range(zero+2+11*it, zero+13+11*it)) + [zero+1]
                c2 = 0.00003 * 10**it
                add_group('socg_table_athens', 'athens', (1.0, c2, 128), kmeans_ids, kmeans_names)
                add_group('socg_table_berlin_ten', 'berlin-10', (1.0, c2, 2717), kmeans_ids, kmeans_names)
                add_group('socg_table_chicago_four', 'chicago-4', (1.0, c2, 222), kmeans_ids, kmeans_names)
                c2 = [0.001, 0.0003, 0.0001][it]
                add_group('socg_table_synth_one', 'synth-A (50)', (1.0, c2, 50), synth_ids(0), kmeans_names)
                add_group('socg_table_synth_one', 'synth-A (90)', (1.0, c2, 50), synth_ids(35), kmeans_names)
                add_group('socg_table_synth_one', 'synth-A (95)', (1.0, c2, 50), synth_ids(70), kmeans_names)
                c2 = [0.0003, 0.0001, 0.00003][it]
                add_group('socg_table_synth_two', 'synth-B (50)', (1.0, c2, 100), synth_ids(0), kmeans_names)
                add_group('socg_table_synth_two', 'synth-B (90)', (1.0, c2, 100), synth_ids(35), kmeans_names)
                add_group('socg_table_synth_two', 'synth-B (95)', (1.0, c2, 100), synth_ids(70), kmeans_names)

                write_plot('bar', data, metric='score', yaxis='Score (lower is better)', title='kMeans Score (0 if DNF)', x_label=None, fig_size=(14, 15))

        case 'socg_bar_time':
            for it in range(3):
                data = dict()

                def add_data(data_block):
                    for key, value in data_block.items():
                        if key not in data:
                            data[key] = []
                        data[key].extend(value)

                def add_group(data_set, data_set_name, c, ids, names):
                    pretty_c2 = "$c_2=${:.8f}".format(c[1]).rstrip('0')
                    group_name = f"{data_set_name}\n{pretty_c2}"
                    data_block = {
                        'name': names,
                        'group': [group_name] * len(ids),
                        'time': [(24*60*60 if e == '---' or e is None else e) for e in get_data(data_set, ids, 'time')]
                    }
                    add_data(data_block)


                kmeans_names = ['SC-$\\ell$']*5 + ['SC-$m$']*4 + ['PSC', 'Envelope', 'Map-construct']
                kmeans_ids = list(range(2+11*it, 13+11*it)) + [1]
                synth_ids = lambda zero: list(range(zero+2+11*it, zero+13+11*it)) + [zero+1]
                c2 = 0.00003 * 10**it
                add_group('socg_table_athens', 'athens', (1.0, c2, 128), kmeans_ids, kmeans_names)
                add_group('socg_table_berlin_ten', 'berlin-10', (1.0, c2, 2717), kmeans_ids, kmeans_names)
                add_group('socg_table_chicago_four', 'chicago-4', (1.0, c2, 222), kmeans_ids, kmeans_names)
                add_group('socg_table_chicago', 'chicago', (1.0, c2, 888), kmeans_ids, kmeans_names)
                # add_group('socg_table_drifter', 'drifter', (1.0, 0.1, ???), ???, ???)
                c2 = 0.001 * 10**it
                # add_group('socg_table_unid', 'UniD', (1.0, c2, 362), kmeans_ids, kmeans_names)
                c2 = [0.001, 0.0003, 0.0001][it]
                add_group('socg_table_synth_one', 'synth-A (50)', (1.0, c2, 50), synth_ids(0), kmeans_names)
                add_group('socg_table_synth_one', 'synth-A (90)', (1.0, c2, 50), synth_ids(35), kmeans_names)
                add_group('socg_table_synth_one', 'synth-A (95)', (1.0, c2, 50), synth_ids(70), kmeans_names)
                c2 = [0.0003, 0.0001, 0.00003][it]
                add_group('socg_table_synth_two', 'synth-B (50)', (1.0, c2, 100), synth_ids(0), kmeans_names)
                add_group('socg_table_synth_two', 'synth-B (90)', (1.0, c2, 100), synth_ids(35), kmeans_names)
                add_group('socg_table_synth_two', 'synth-B (95)', (1.0, c2, 100), synth_ids(70), kmeans_names)

                write_plot('bar', data, metric='time', yaxis='Seconds', title='Time (86400 if DNF)', x_label=None, fig_size=(14, 15))

        case 'socg_bar_memory':
            for it in range(3):
                data = dict()

                def add_data(data_block):
                    for key, value in data_block.items():
                        if key not in data:
                            data[key] = []
                        data[key].extend(value)

                def add_group(data_set, data_set_name, c, ids, names):
                    pretty_c2 = "$c_2=${:.8f}".format(c[1]).rstrip('0')
                    group_name = f"{data_set_name}\n{pretty_c2}"
                    data_block = {
                        'name': names,
                        'group': [group_name] * len(ids),
                        'memory': [(0 if e == '---' or e is None else e) for e in get_data(data_set, ids, 'memory')]
                    }
                    add_data(data_block)


                kmeans_names = ['SC-$\\ell$']*5 + ['SC-$m$']*4 + ['PSC', 'Envelope', 'Map-construct']
                kmeans_ids = list(range(2+11*it, 13+11*it)) + [1]
                synth_ids = lambda zero: list(range(zero+2+11*it, zero+13+11*it)) + [zero+1]
                c2 = 0.00003 * 10**it
                add_group('socg_table_athens', 'athens', (1.0, c2, 128), kmeans_ids, kmeans_names)
                add_group('socg_table_berlin_ten', 'berlin-10', (1.0, c2, 2717), kmeans_ids, kmeans_names)
                add_group('socg_table_chicago_four', 'chicago-4', (1.0, c2, 222), kmeans_ids, kmeans_names)
                add_group('socg_table_chicago', 'chicago', (1.0, c2, 888), kmeans_ids, kmeans_names)
                # add_group('socg_table_drifter', 'drifter', (1.0, 0.1, ???), ???, ???)
                c2 = 0.001 * 10**it
                # add_group('socg_table_unid', 'UniD', (1.0, c2, 362), kmeans_ids, kmeans_names)
                c2 = [0.001, 0.0003, 0.0001][it]
                add_group('socg_table_synth_one', 'synth-A (50)', (1.0, c2, 50), synth_ids(0), kmeans_names)
                add_group('socg_table_synth_one', 'synth-A (90)', (1.0, c2, 50), synth_ids(35), kmeans_names)
                add_group('socg_table_synth_one', 'synth-A (95)', (1.0, c2, 50), synth_ids(70), kmeans_names)
                c2 = [0.0003, 0.0001, 0.00003][it]
                add_group('socg_table_synth_two', 'synth-B (50)', (1.0, c2, 100), synth_ids(0), kmeans_names)
                add_group('socg_table_synth_two', 'synth-B (90)', (1.0, c2, 100), synth_ids(35), kmeans_names)
                add_group('socg_table_synth_two', 'synth-B (95)', (1.0, c2, 100), synth_ids(70), kmeans_names)

                write_plot('bar', data, metric='memory', yaxis='GBytes', title='Memory (0 if DNF)', x_label=None, fig_size=(14, 15))

        case 'socg_logbar_bbgll_memory':
            data = dict()

            def add_data(data_block):
                for key, value in data_block.items():
                    if key not in data:
                        data[key] = []
                    data[key].extend(value)

            def add_group(data_set, data_set_name, distance, size, ids, names):
                pretty_d = "$\\Delta=${}".format(distance)
                pretty_s = "$m=${}".format(size)
                group_name = f"{data_set_name}\n{pretty_d}\n{pretty_s}"
                data_block = {
                    'name': names,
                    'color': ['C0', 'C1', 'C3', 'C2'],
                    'group': [group_name] * len(ids),
                    'memory': [(0 if e == '---' or e is None else e) for e in get_data(data_set, ids, 'memory')]
                }
                add_data(data_block)

            # list SC-m last to get the green color
            names = ['SC-$m$', 'Map-construct-rtree', 'Map-construct', 'MoveTK'][::-1]
            ids = list(range(18, 22))[::-1]
            synth_ids = lambda i: list(range(2 + 6*i, 6*(i+1)))[::-1]
            add_group('socg_bbgll_athens', 'athens', 100, 40, ids, names)
            add_group('socg_bbgll_chicago_four', 'chicago-4', 100, 80, ids, names)
            add_group('socg_bbgll_berlin_ten', 'berlin-10', 100, 20, ids, names)
            add_group('socg_bbgll_synth', 'synth-A (50)', 100, 10, synth_ids(0), names)
            add_group('socg_bbgll_synth', 'synth-A (90)', 100, 10, synth_ids(1), names)
            add_group('socg_bbgll_synth', 'synth-A (95)', 100, 10, synth_ids(2), names)
            add_group('socg_bbgll_synth', 'synth-B (50)', 100, 10, synth_ids(3), names)
            add_group('socg_bbgll_synth', 'synth-B (90)', 100, 10, synth_ids(4), names)
            add_group('socg_bbgll_synth', 'synth-B (95)', 100, 10, synth_ids(5), names)

            write_plot('bar', data, metric='memory', yaxis='GBytes', title='Memory (0 if DNF)', x_label=None, fig_size=(14, 15), use_item_color=True, logplot=True)

        case 'socg_logbar_bbgll_time':
            data = dict()

            def add_data(data_block):
                for key, value in data_block.items():
                    if key not in data:
                        data[key] = []
                    data[key].extend(value)

            def add_group(data_set, data_set_name, distance, size, ids, names):
                pretty_d = "$\\Delta=${}".format(distance)
                pretty_s = "$m=${}".format(size)
                group_name = f"{data_set_name}\n{pretty_d}\n{pretty_s}"
                data_block = {
                    'name': names,
                    'color': ['C0', 'C1', 'C3', 'C2'],
                    'group': [group_name] * len(ids),
                    'time': [(24*60*60 if e == '---' or e is None else e) for e in get_data(data_set, ids, 'time')]
                }
                add_data(data_block)

            # list SC-m last to get the green color
            names = ['SC-$m$', 'Map-construct-rtree', 'Map-construct', 'MoveTK'][::-1]
            ids = list(range(18, 22))[::-1]
            synth_ids = lambda i: list(range(2 + 6*i, 6*(i+1)))[::-1]
            add_group('socg_bbgll_athens', 'athens', 100, 40, ids, names)
            add_group('socg_bbgll_chicago_four', 'chicago-4', 100, 80, ids, names)
            add_group('socg_bbgll_berlin_ten', 'berlin-10', 100, 20, ids, names)
            add_group('socg_bbgll_synth', 'synth-A (50)', 100, 10, synth_ids(0), names)
            add_group('socg_bbgll_synth', 'synth-A (90)', 100, 10, synth_ids(1), names)
            add_group('socg_bbgll_synth', 'synth-A (95)', 100, 10, synth_ids(2), names)
            add_group('socg_bbgll_synth', 'synth-B (50)', 100, 10, synth_ids(3), names)
            add_group('socg_bbgll_synth', 'synth-B (90)', 100, 10, synth_ids(4), names)
            add_group('socg_bbgll_synth', 'synth-B (95)', 100, 10, synth_ids(5), names)

            write_plot('bar', data, metric='time', yaxis='Seconds', title='Time (86400 if DNF)', x_label=None, fig_size=(14, 15), use_item_color=True, logplot=True)

        case _:
            assert False, f"Invalid plot name: {plot_name}"
