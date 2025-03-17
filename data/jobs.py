# screw python's dict syntax
def make_args(**kwargs):
    return kwargs


def our_c(c1, c2, c3):
    return make_args(c1=c1, c2=c2, c3=c3)


def agarwal_c(c1, c2, c3):
    """
    We use c2 for distance and c3 for cover.
    Agarwal uses c2 for cover and c3 for distance.
    """
    return make_args(c1=c1, c2=c3, c3=c2)


def get_jobs(job_name, run_job):
    """
    Run one or more jobs, based on job_name.
    For each job to run, do run_job(job_type, job_args).
    See script/run.py for valid job types.
    """
    no_simp = '-s 0.0'
    multithread = '-t 20'
    match job_name:
        case 'compile':
            run_job('compile_our_code')
            run_job('compile_movetk')
            run_job('compile_java')
            run_job('compile_java_bbgll')

        case 'socg_bbgll_athens':
            run_job('java_input_bbgll', make_args(data_set='athens_small.out'))
            run_job('java_input_bbgll', make_args(data_set='athens_small.out', rtree=True))
            for distance in [50.0, 100.0, 200.0]:
                for size in [20, 40, 80]:
                    run_job('bbgll', make_args(data_set='athens_small.out', distance=distance, minsize=size))
                    run_job('java_bbgll', make_args(data_set='athens_small.out', distance=int(distance), minsize=size, rtree=True))
                    run_job('java_bbgll', make_args(data_set='athens_small.out', distance=int(distance), minsize=size, rtree=False))
                    run_job('movetk', make_args(data_set='athens_small.out', distance=distance, minsize=size))

        case 'socg_bbgll_chicago_four':
            run_job('java_input_bbgll', make_args(data_set='chicago_4_1.out'))
            run_job('java_input_bbgll', make_args(data_set='chicago_4_1.out', rtree=True))
            for distance in [50.0, 100.0, 200.0]:
                for size in [40, 80, 160]:
                    run_job('bbgll', make_args(data_set='chicago_4_1.out', distance=distance, minsize=size))
                    run_job('java_bbgll', make_args(data_set='chicago_4_1.out', distance=int(distance), minsize=size, rtree=True))
                    run_job('java_bbgll', make_args(data_set='chicago_4_1.out', distance=int(distance), minsize=size, rtree=False))
                    run_job('movetk', make_args(data_set='chicago_4_1.out', distance=distance, minsize=size))

        case 'socg_bbgll_berlin_ten':
            run_job('java_input_bbgll', make_args(data_set='berlin_large_10_1.out'))
            run_job('java_input_bbgll', make_args(data_set='berlin_large_10_1.out', rtree=True))
            for distance in [50.0, 100.0, 200.0]:
                for size in [10, 20, 40]:
                    run_job('bbgll', make_args(data_set='berlin_large_10_1.out', distance=distance, minsize=size))
                    run_job('java_bbgll', make_args(data_set='berlin_large_10_1.out', distance=int(distance), minsize=size, rtree=True))
                    run_job('java_bbgll', make_args(data_set='berlin_large_10_1.out', distance=int(distance), minsize=size, rtree=False))
                    run_job('movetk', make_args(data_set='berlin_large_10_1.out', distance=distance, minsize=size))

        case 'socg_bbgll_synth':
            for ds in ['synth_200_50_05.out', 'synth_200_50_09.out', 'synth_200_50_095.out', 'synth_100_100_05.out', 'synth_100_100_09.out', 'synth_100_100_095.out']:
                run_job('java_input_bbgll', make_args(data_set=ds))
                run_job('java_input_bbgll', make_args(data_set=ds, rtree=True))
                for distance in [100.0]:
                    for size in [10]:
                        run_job('bbgll', make_args(data_set=ds, distance=distance, minsize=size))
                        run_job('java_bbgll', make_args(data_set=ds, distance=int(distance), minsize=size, rtree=True))
                        run_job('java_bbgll', make_args(data_set=ds, distance=int(distance), minsize=size, rtree=False))
                        run_job('movetk', make_args(data_set=ds, distance=distance, minsize=size))

        case 'socg_table_athens':
            # java runs the same for all constants
            run_job('java_input', make_args(data_set='athens_small.out'))
            run_job('java', make_args(data_set='athens_small.out'))
            run_job('basic', our_c(1.0, 0.00003, 128) | make_args(data_set='athens_small.out', prog_args=['-l 4']), parallel=True)
            run_job('basic', our_c(1.0, 0.00003, 128) | make_args(data_set='athens_small.out', prog_args=['-l 8']), parallel=True)
            run_job('basic', our_c(1.0, 0.00003, 128) | make_args(data_set='athens_small.out', prog_args=['-l 16']), parallel=True)
            run_job('basic', our_c(1.0, 0.00003, 128) | make_args(data_set='athens_small.out', prog_args=['-l 32']), parallel=True)
            run_job('basic', our_c(1.0, 0.00003, 128) | make_args(data_set='athens_small.out', prog_args=['-l 64']), parallel=True)
            run_job('basic', our_c(1.0, 0.00003, 128) | make_args(data_set='athens_small.out', prog_args=['-s 2']), parallel=True)
            run_job('basic', our_c(1.0, 0.00003, 128) | make_args(data_set='athens_small.out', prog_args=['-s 4']), parallel=True)
            run_job('basic', our_c(1.0, 0.00003, 128) | make_args(data_set='athens_small.out', prog_args=['-s 8']), parallel=True)
            run_job('basic', our_c(1.0, 0.00003, 128) | make_args(data_set='athens_small.out', prog_args=['-s 16']), parallel=True)
            run_job('rightstep', our_c(1.0, 0.00003, 128) | make_args(data_set='athens_small.out', mode='means'), parallel=True)
            run_job('agarwal_our_score', agarwal_c(1.0, 0.00003, 128) | make_args(data_set='athens_small.out'))

            run_job('basic', our_c(1.0, 0.0003, 128) | make_args(data_set='athens_small.out', prog_args=['-l 4']), parallel=True)
            run_job('basic', our_c(1.0, 0.0003, 128) | make_args(data_set='athens_small.out', prog_args=['-l 8']), parallel=True)
            run_job('basic', our_c(1.0, 0.0003, 128) | make_args(data_set='athens_small.out', prog_args=['-l 16']), parallel=True)
            run_job('basic', our_c(1.0, 0.0003, 128) | make_args(data_set='athens_small.out', prog_args=['-l 32']), parallel=True)
            run_job('basic', our_c(1.0, 0.0003, 128) | make_args(data_set='athens_small.out', prog_args=['-l 64']), parallel=True)
            run_job('basic', our_c(1.0, 0.0003, 128) | make_args(data_set='athens_small.out', prog_args=['-s 2']), parallel=True)
            run_job('basic', our_c(1.0, 0.0003, 128) | make_args(data_set='athens_small.out', prog_args=['-s 4']), parallel=True)
            run_job('basic', our_c(1.0, 0.0003, 128) | make_args(data_set='athens_small.out', prog_args=['-s 8']), parallel=True)
            run_job('basic', our_c(1.0, 0.0003, 128) | make_args(data_set='athens_small.out', prog_args=['-s 16']), parallel=True)
            run_job('rightstep', our_c(1.0, 0.0003, 128) | make_args(data_set='athens_small.out', mode='means'), parallel=True)
            run_job('agarwal_our_score', agarwal_c(1.0, 0.0003, 128) | make_args(data_set='athens_small.out'))

            run_job('basic', our_c(1.0, 0.003, 128) | make_args(data_set='athens_small.out', prog_args=['-l 4']), parallel=True)
            run_job('basic', our_c(1.0, 0.003, 128) | make_args(data_set='athens_small.out', prog_args=['-l 8']), parallel=True)
            run_job('basic', our_c(1.0, 0.003, 128) | make_args(data_set='athens_small.out', prog_args=['-l 16']), parallel=True)
            run_job('basic', our_c(1.0, 0.003, 128) | make_args(data_set='athens_small.out', prog_args=['-l 32']), parallel=True)
            run_job('basic', our_c(1.0, 0.003, 128) | make_args(data_set='athens_small.out', prog_args=['-l 64']), parallel=True)
            run_job('basic', our_c(1.0, 0.003, 128) | make_args(data_set='athens_small.out', prog_args=['-s 2']), parallel=True)
            run_job('basic', our_c(1.0, 0.003, 128) | make_args(data_set='athens_small.out', prog_args=['-s 4']), parallel=True)
            run_job('basic', our_c(1.0, 0.003, 128) | make_args(data_set='athens_small.out', prog_args=['-s 8']), parallel=True)
            run_job('basic', our_c(1.0, 0.003, 128) | make_args(data_set='athens_small.out', prog_args=['-s 16']), parallel=True)
            run_job('rightstep', our_c(1.0, 0.003, 128) | make_args(data_set='athens_small.out', mode='means'), parallel=True)
            run_job('agarwal_our_score', agarwal_c(1.0, 0.003, 128) | make_args(data_set='athens_small.out'))

        case 'socg_table_athens_centers':
            timeout = 24 * 60 * 60 # 1 day
            ds = 'athens_small.out'
            m = 'centers'

            # java runs the same for all constants
            run_job('java_input', make_args(data_set='athens_small.out'))
            run_job('java', make_args(data_set='athens_small.out'))

            for c2 in [0.003, 0.03, 0.3]:
                c_us = our_c(1, c2, 128)
                c_a = agarwal_c(1, c2, 128)
                for l in [4,8,16,32,64]:
                    run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-l {l}'], mode=m, timeout=timeout), parallel= (l != 64))
                for s in [2,4,8,16]:
                    run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-s {s}'], mode=m, timeout=timeout), parallel=True)
                run_job('rightstep', c_us | make_args(data_set=ds, mode=m, timeout=timeout), parallel=False)

        case 'socg_table_chicago':
            timeout = 24 * 60 * 60 # 1 day
            ds = 'chicago.out'
            m = 'means'

            # java doesn't depend on c1/c2/c3
            run_job('java_input', make_args(data_set=ds))
            run_job('java', make_args(data_set=ds, timeout=timeout))

            for c2 in [0.00003, 0.0003, 0.003]:
                c_us = our_c(1, c2, 888)
                c_a = agarwal_c(1, c2, 888)
                for l in [4,8,16,32,64]:
                    run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-l {l}'], mode=m, timeout=timeout), parallel=True)
                for s in [2,4,8,16]:
                    run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-s {s}'], mode=m, timeout=timeout), parallel=True)
                run_job('rightstep', c_us | make_args(data_set=ds, mode=m, timeout=timeout), parallel=True)
                run_job('agarwal_our_score', c_a | make_args(data_set=ds, timeout=timeout))

        case 'socg_table_chicago_centers':
            timeout = 24 * 60 * 60 # 1 day
            ds = 'chicago.out'
            m = 'centers'

            for c2 in [10.0, 1.0, 0.1]:
                c_us = our_c(1, c2, 888)
                for l in [4,8,16,32,64]:
                    run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-l {l}'], mode=m, timeout=timeout), parallel=True)
                for s in [2,4,8,16]:
                    run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-s {s}'], mode=m, timeout=timeout), parallel=True)
                run_job('rightstep', c_us | make_args(data_set=ds, mode=m, timeout=timeout), parallel=False)

        case 'socg_table_chicago_four':
            timeout = 24 * 60 * 60 # 1 day
            ds = 'chicago_4_1.out'
            m = 'means'

            # java doesn't depend on c1/c2/c3
            run_job('java_input', make_args(data_set=ds))
            run_job('java', make_args(data_set=ds, timeout=timeout))

            for c2 in [0.00003, 0.0003, 0.003]:
                c_us = our_c(1, c2, 222)
                c_a = agarwal_c(1, c2, 222)
                for l in [4,8,16,32,64]:
                    run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-l {l}'], mode=m, timeout=timeout), parallel=True)
                for s in [2,4,8,16]:
                    run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-s {s}'], mode=m, timeout=timeout), parallel=True)
                run_job('rightstep', c_us | make_args(data_set=ds, mode=m, timeout=timeout), parallel=True)
                run_job('agarwal_our_score', c_a | make_args(data_set=ds, timeout=timeout))

        case 'socg_table_chicago_four_centers':
            timeout = 24 * 60 * 60 # 1 day
            ds = 'chicago_4_1.out'
            m = 'centers'

            for c2 in [0.01, 0.1, 1.0]:
                c_us = our_c(1, c2, 222)
                for l in [4,8,16,32,64]:
                    run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-l {l}'], mode=m, timeout=timeout), parallel=True)
                for s in [2,4,8,16]:
                    run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-s {s}'], mode=m, timeout=timeout), parallel=True)
                run_job('rightstep', c_us | make_args(data_set=ds, mode=m, timeout=timeout), parallel=False)

        case 'socg_table_drifter':
            timeout = 24 * 60 * 60 # 1 day
            ds = 'drifter.out'
            m = 'means'
            for i, c2 in enumerate([0.01, 0.1, 1.0, 10]):
                c_us = our_c(1, c2, 2011)
                run_job('rightstep', c_us | make_args(data_set=ds, mode=m, timeout=timeout), parallel=False)

        case 'socg_table_drifter_centers':
            timeout = 24 * 60 * 60 # 1 day
            ds = 'drifter.out'
            m = 'centers'
            for i, c2 in enumerate([10, 30, 100, 300, 1000]):
                c_us = our_c(1, c2, 2011)
                run_job('rightstep', c_us | make_args(data_set=ds, mode=m, timeout=timeout), parallel=False)

        case 'socg_table_unid':
            timeout = 24 * 60 * 60 # 1 day
            ds = 'unid_00.out'
            m = 'means'

            # java doesn't depend on c1/c2/c3
            run_job('java_input', make_args(data_set=ds))
            run_job('java', make_args(data_set=ds, timeout=timeout)) # out of memory -> java waits for ever -> java waits for ever

            for i, c2 in enumerate([0.001, 0.01, 0.1]):
                c_us = our_c(1, c2, 362)
                c_a = agarwal_c(1, c2, 362)
                for l in [4,8,16,32,64]:
                    run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-l {l}'], mode=m, timeout=timeout), parallel=True)
                for s in [2,4,8,16]:
                    run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-s {s}'], mode=m, timeout=timeout), parallel=True)
                run_job('rightstep', c_us | make_args(data_set=ds, mode=m, timeout=timeout), parallel=True)
                run_job('agarwal_our_score', c_a | make_args(data_set=ds, timeout=timeout))

        case 'socg_table_unid_centers':
            timeout = 24 * 60 * 60 # 1 day
            ds = 'unid_00.out'
            m = 'centers'

            # 300 -> singeltons, 30 -> very good, 3 -> too coarse
            for i, c2 in enumerate([3, 30, 300]):
                c_us = our_c(1, c2, 362)
                for l in [4,8,16,32,64]:
                    run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-l {l}'], mode=m, timeout=timeout), parallel=True)
                for s in [2,4,8,16]:
                    run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-s {s}'], mode=m, timeout=timeout), parallel=True)
                run_job('rightstep', c_us | make_args(data_set=ds, mode=m, timeout=timeout), parallel=False)

        case 'socg_table_synth_zero':
            timeout = 24 * 60 * 60 # 1 day
            ds = 'synth_200_50.out'
            m = 'means'

            # java doesn't depend on c1/c2/c3
            run_job('java_input', make_args(data_set=ds))
            run_job('java', make_args(data_set=ds, timeout=timeout)) # infinite loop after finding 0 bundles.

            # 0.1 yields a point cluster, 10.0 yields singletons
            for i, c2 in enumerate([0.3, 1.0, 3.0]):
                c_us = our_c(1, c2, 50)
                c_a = agarwal_c(1, c2, 50)
                for l in [4,8,16,32,64]:
                    run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-l {l}'], mode=m, timeout=timeout), parallel=True)
                for s in [2,4,8,16]:
                    run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-s {s}'], mode=m, timeout=timeout), parallel=True)
                run_job('rightstep', c_us | make_args(data_set=ds, mode=m, timeout=timeout), parallel=True)
                run_job('agarwal_our_score', c_a | make_args(data_set=ds, timeout=timeout))

        case 'socg_table_synth_one':
            timeout = 24 * 60 * 60 # 1 day
            m = 'means'

            for ds in ['synth_200_50_05.out', 'synth_200_50_09.out', 'synth_200_50_095.out']:
                # java doesn't depend on c1/c2/c3
                run_job('java_input', make_args(data_set=ds))
                run_job('java', make_args(data_set=ds, timeout=timeout))

                for i, c2 in enumerate([0.001, 0.0003, 0.0001]):
                    skip = (('09' not in ds) or (i != 2))
                    c_us = our_c(1, c2, 50)
                    c_a = agarwal_c(1, c2, 50)
                    for l in [4,8,16,32,64]:
                        run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-l {l}'], mode=m, timeout=timeout), parallel=(l != 64))
                    for s in [2,4,8,16]:
                        run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-s {s}'], mode=m, timeout=timeout), parallel=True)
                    run_job('rightstep', c_us | make_args(data_set=ds, mode=m, timeout=timeout), parallel=True)
                    run_job('agarwal_our_score', c_a | make_args(data_set=ds, timeout=timeout))

        case 'socg_table_synth_two':
            timeout = 24 * 60 * 60 # 1 day
            m = 'means'

            for ds in ['synth_100_100_05.out', 'synth_100_100_09.out', 'synth_100_100_095.out']:
                # java doesn't depend on c1/c2/c3
                run_job('java_input', make_args(data_set=ds))
                run_job('java', make_args(data_set=ds, timeout=timeout))

                for i, c2 in enumerate([0.0003, 0.0001, 0.00003]):
                    c_us = our_c(1, c2, 100)
                    c_a = agarwal_c(1, c2, 100)
                    for l in [4,8,16,32,64]:
                        run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-l {l}'], mode=m, timeout=timeout), parallel=(l != 64))
                    for s in [2,4,8,16]:
                        run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-s {s}'], mode=m, timeout=timeout), parallel=True)
                    run_job('rightstep', c_us | make_args(data_set=ds, mode=m, timeout=timeout))
                    run_job('agarwal_our_score', c_a | make_args(data_set=ds, timeout=timeout), parallel=True)

        case 'socg_table_berlin_ten':
            timeout = 24 * 60 * 60 # 1 day
            ds = 'berlin_large_10_1.out'
            m = 'means'

            # java doesn't depend on c1/c2/c3
            run_job('java_input', make_args(data_set=ds))
            run_job('java', make_args(data_set=ds, timeout=timeout))

            for i, c2 in enumerate([0.00003, 0.0003, 0.003]):
                c_us = our_c(1, c2, 2717)
                c_a = agarwal_c(1, c2, 2717)
                for l in [4,8,16,32,64]:
                    run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-l {l}'], mode=m, timeout=timeout), parallel=(l != 64))
                for s in [2,4,8,16]:
                    run_job('basic', c_us | make_args(data_set=ds, prog_args=[f'-s {s}'], mode=m, timeout=timeout), parallel=True)
                run_job('rightstep', c_us | make_args(data_set=ds, mode=m, timeout=timeout), parallel=True)
                run_job('agarwal_our_score', c_a | make_args(data_set=ds, timeout=timeout))

        case 'socg_table_berlin':
            timeout = 24 * 60 * 60 # 1 day
            ds = 'berlin_large.out'
            m = 'means'

            # java doesn't depend on c1/c2/c3
            run_job('java_input', make_args(data_set=ds))
            run_job('java', make_args(data_set=ds, timeout=timeout))

            for i, c2 in enumerate([0.00003, 0.0003, 0.003]):
                c_us = our_c(1, c2, 27188)
                c_a = agarwal_c(1, c2, 27188)
                run_job('rightstep', c_us | make_args(data_set=ds, mode=m, timeout=timeout), parallel=True)

        case 'socg_table_bbgll':
            for (distance, size, ds) in [(100.0, 40, 'athens_small.out'), (100.0, 45, 'chicago_4_1.out'), (100.0, 45, 'berlin_large_10_1.out')]:
                run_job('movetk', make_args(data_set=ds, distance=100.0, minsize=40))
                run_job('bbgll', make_args(data_set=ds, distance=100.0, minsize=40))  # ok
                run_job('java_input_bbgll', make_args(data_set=ds))
                run_job('java_bbgll', make_args(data_set=ds))


        case _:
            assert False, f"Invalid job name {job_name}"
