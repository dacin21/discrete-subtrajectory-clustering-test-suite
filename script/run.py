import os
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from script.common import find_file, git_root_directory
from script.javafy import input_to_java, output_from_java


def get_data_folder():
    return git_root_directory() / 'data'


def java_root_directory(bbgll=False):
    return git_root_directory() / ('java_bbgll' if bbgll else 'java')


def get_java_data_folder(bbgll=False):
    return java_root_directory(bbgll) / 'data'


"""
commands is a list of string commands, to be run.
The last command should be 'exit\n', to ensure that the container exits.
"""
def run_in_podman(podman_script, commands, log_file_path, timeout=None):
    with log_file_path.open('w') as log_file:
        subprocess.run(' '.join(['bash', str(podman_script), 'bash'] + ([str(timeout)] if timeout else [])), input='\n'.join(commands), text=True, stdout=log_file, stderr=subprocess.STDOUT, shell=True)


"""
Turns a pathlib.Path object into a string to be used inside the podman container.
In the subtrajg-clustering/podman container, the git repo is mounted at /me
In the our_code/podman container, the data folder is mounted at /data
"""
def podman_path(path, data_folder=False):
    if data_folder:
        return '/data/' + str(path.relative_to(get_data_folder()))
    else:
        return '/me/' + str(path.relative_to(git_root_directory()))


def setup_some_paths(job_args):
    job_file_name = job_args['file_name']
    results_folder = get_data_folder() / 'results'
    log_file_path = results_folder / (job_file_name + '.log')

    return job_file_name, results_folder, log_file_path


def setup_paths(job_args, data_folder=False):
    job_file_name, results_folder, log_file_path = setup_some_paths(job_args)
    output_file_path = podman_path(results_folder / (job_file_name + '.out'), data_folder)
    data_set_path = podman_path(find_file(job_args['data_set']), data_folder)

    return job_file_name, results_folder, log_file_path, output_file_path, data_set_path


def run_agarwal(version, job_args):
    assert version in ['original_buggy', 'bugfixed', 'our_score']
    job_file_name, results_folder, log_file_path, output_file_path, data_set_path = setup_paths(job_args, False)

    use_pypy = job_args.get('pypy', False)
    timeout = job_args.get('timeout', None)

    commands = [
        "python --version",
        f"cd /me/agarwal/{version}/",
        f"/usr/bin/time -v python run.py {data_set_path} {output_file_path} {job_args['c1']} {job_args['c2']} {job_args['c3']}",
        "exit\n",
    ]
    if use_pypy:
        # need to source bash to keep the virtual environment.
        commands = [
            "source podman/setup_pypy.sh <<<\"\"",
        ] + commands

    run_in_podman(git_root_directory() / 'podman' / 'podman.sh', commands, log_file_path, timeout=timeout)


def compile_movetk(job_args):
    job_file_name, results_folder, log_file_path = setup_some_paths(job_args)

    commands = [
        "ls",
        "./compile.sh",
        "exit\n",
    ]
    run_in_podman(git_root_directory() / 'movetk' / 'podman' / 'podman.sh', commands, log_file_path)


def run_movetk(job_args):
    job_file_name, results_folder, log_file_path, output_file_path, data_set_path = setup_paths(job_args, True)
    timeout = job_args.get('timeout', None)

    commands = [
        "cd build",
        "ls",
        f"/usr/bin/time -v ./examples/clustering {data_set_path} {output_file_path} {job_args['distance']} {job_args['minsize']}",
        "exit\n",
    ]
    run_in_podman(git_root_directory() / 'movetk' / 'podman' / 'podman.sh', commands, log_file_path, timeout=timeout)


def compile_our_code(job_args):
    job_file_name, results_folder, log_file_path = setup_some_paths(job_args)

    commands = [
        "cd build",
        "ls",
        "meson setup",
        "meson compile",
        "exit\n",
    ]
    run_in_podman(git_root_directory() / 'our_code' / 'podman' / 'podman.sh', commands, log_file_path)


def run_bbgll(job_args):
    job_file_name, results_folder, log_file_path, output_file_path, data_set_path = setup_paths(job_args, True)
    timeout = job_args.get('timeout', None)

    commands = [
        "cd build",
        "ls",
        f"/usr/bin/time -v ./bbgll {job_args['distance']} {job_args['minsize']} {data_set_path} {output_file_path} ",
        "exit\n",
    ]
    run_in_podman(git_root_directory() / 'our_code' / 'podman' / 'podman.sh', commands, log_file_path, timeout=timeout)


def run_rightstep(job_args):
    job_file_name, results_folder, log_file_path, output_file_path, data_set_path = setup_paths(job_args, True)

    mode = ['means', 'centers'].index(job_args.get('mode', 'means'))
    timeout = job_args.get('timeout', None)

    commands = [
        "cd build",
        "ls",
        f"/usr/bin/time -v ./rightstep {data_set_path} {output_file_path} -c {job_args['c1']} {job_args['c2']} {job_args['c3']} -m {mode} {' '.join(job_args.get('prog_args', []))}",
        "exit\n",
    ]
    run_in_podman(git_root_directory() / 'our_code' / 'podman' / 'podman.sh', commands, log_file_path, timeout=timeout)


def run_basic(job_args):
    job_file_name, results_folder, log_file_path, output_file_path, data_set_path = setup_paths(job_args, True)

    mode = ['means', 'centers'].index(job_args.get('mode', 'means'))
    timeout = job_args.get('timeout', None)

    commands = [
        "cd build",
        "ls",
        f"/usr/bin/time -v ./basic {data_set_path} {output_file_path} -c {job_args['c1']} {job_args['c2']} {job_args['c3']} -m {mode} {' '.join(job_args.get('prog_args', []))}",
        "exit\n",
    ]
    run_in_podman(git_root_directory() / 'our_code' / 'podman' / 'podman.sh', commands, log_file_path, timeout=timeout)


def create_java_input(job_args, bbgll=False):
    rtree = job_args.get('rtree', False)
    input_file = find_file(job_args['data_set'])
    data_set_name = input_file.stem + ('_rtree' if rtree else '')
    input_to_java(input_file, get_java_data_folder(bbgll=bbgll) / data_set_name, concatenate_trajectories=(bbgll and not rtree))


def compile_java(job_args, bbgll=False):
    job_file_name, results_folder, log_file_path = setup_some_paths(job_args)

    commands = [
        "mvn package -Dmaven.test.failure.ignore=true",
        "exit\n",
    ]
    run_in_podman(java_root_directory(bbgll) / 'podman' / 'podman.sh', commands, log_file_path)


def run_java(job_args):
    job_file_name, results_folder, log_file_path = setup_some_paths(job_args)
    data_set_file = find_file(job_args['data_set'])
    timeout = job_args.get('timeout', None)

    commands = [
        # replacing -call with -cb doesn't write the bundles to disk.
        f"/usr/bin/time -v java -cp target/MapConstructionWeb-1.0-SNAPSHOT.jar mapconstruction.starter.Starter -p config.yml -d {data_set_file.stem} -call {' '.join(job_args.get('prog_args', []))}",
        "exit\n",
    ]
    run_in_podman(java_root_directory() / 'podman' / 'podman.sh', commands, log_file_path, timeout=timeout)
    output_from_java(java_root_directory(), data_set_file, results_folder / f"{job_file_name}.out")


def run_java_bbgll(job_args):
    job_file_name, results_folder, log_file_path = setup_some_paths(job_args)
    data_set_file = find_file(job_args['data_set'])
    timeout = job_args.get('timeout', None)
    rtree = (1 if job_args.get('rtree', False) else 0)
    data_set_name = data_set_file.stem + ('_rtree' if rtree else '')

    commands = [
        # pass bbgll argument to java
        f"export BBGLL_DELTA={job_args['distance']}",
        f"export BBGLL_SIZE={job_args['minsize']}",
        f"export USE_RTREE={rtree}",
        "echo $BBGLL_DELTA",
        "echo $BBGLL_SIZE",
        # replacing -call with -cb doesn't write the bundles to disk.
        f"/usr/bin/time -v java -cp target/MapConstructionWeb-1.0-SNAPSHOT.jar mapconstruction.starter.Starter -p config.yml -d {data_set_name} -call {' '.join(job_args.get('prog_args', []))}",
        "exit\n",
    ]
    run_in_podman(java_root_directory(bbgll=True) / 'podman' / 'podman.sh', commands, log_file_path, timeout=timeout)
    output_from_java(java_root_directory(bbgll=True), data_set_file, results_folder / f"{job_file_name}.out")


def generate_file_name(job_args):
    return f"{job_args['job_name']}_{job_args['step']}"


def run_job(args, job_type, job_args):
    print('run_job', job_type, job_args)
    job_args['job_name'] = args.job_name
    job_args['file_name'] = generate_file_name(job_args)

    match job_type:
        case 'agarwal_buggy':
            run_agarwal('original_buggy', job_args)
        case 'agarwal_fixed':
            run_agarwal('bugfixed', job_args)
        case 'agarwal_our_score':
            run_agarwal('our_score', job_args)

        case 'compile_movetk':
            compile_movetk(job_args)
        case 'movetk':
            run_movetk(job_args)

        case 'compile_our_code':
            compile_our_code(job_args)
        case 'rightstep':
            run_rightstep(job_args)
        case 'basic':
            run_basic(job_args)
        case 'bbgll':
            run_bbgll(job_args)

        case 'compile_java':
            compile_java(job_args)
        case 'java_input':
            create_java_input(job_args)
        case 'java':
            run_java(job_args)

        case 'compile_java_bbgll':
            compile_java(job_args, bbgll=True)
        case 'java_input_bbgll':
            create_java_input(job_args, bbgll=True)
        case 'java_bbgll':
            run_java_bbgll(job_args)

        case _:
            assert False, f"Invalid job type: {job_type}"


def main(args):
    sys.path.insert(1, os.path.realpath(os.path.pardir))
    from data.jobs import get_jobs
    step = -1
    executor = None
    futures = []
    def wait_for_all():
        for id, future in futures:
            future.result()
            print(f"Job {id} done")
        futures.clear()

    def run(job_type, job_args={}, skip=False, parallel=False):
        nonlocal step, executor
        step += 1
        job_args['step'] = step
        if skip:
            return
        if parallel:
            if not executor:
                executor = ThreadPoolExecutor()
            futures.append((step, executor.submit(run_job, args, job_type, job_args)))
        else:
            wait_for_all()
            run_job(args, job_type, job_args)
            print(f"Job {step} done")

    get_jobs(args.job_name, run)
    wait_for_all()
