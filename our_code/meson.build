project('frechet',
        'cpp',
        default_options: ['cpp_std=c++20',
                          'warning_level=3',
                          'werror=true',
                          'b_ndebug=if-release',
                          'buildtype=release'],
        version: '0.0.1')

cpp = meson.get_compiler('cpp')

src_inc_dir = include_directories('src')
ext_inc_dir = include_directories('ext')

sources = [
        'src/profiling.cpp'
]

# Enable profiling
def_profiling = 'PROFILING'
def_google_profiling = 'USE_PERFTOOLS_PROFILER'

feature_defs = []
if get_option('profiling')
    feature_defs += ['-D' + def_profiling]
endif

if get_option('perftools')
    feature_defs += ['-D' + def_google_profiling]
    link_google_prof = ['-lprofiler']
else
    link_google_prof = []
endif

if get_option('fsanitize')
    sanitize_flags = ['-fsanitize=undefined,address']
else
    sanitize_flags = []
endif

compiler_args = feature_defs + sanitize_flags
linker_args = link_google_prof + sanitize_flags

message('Definitions:', feature_defs)

boost = dependency('boost')
cgal = dependency('CGAL')
omp = dependency('openmp')

kcluster = executable('kcluster',
                      ['src/kcluster.cpp'] + sources,
                      include_directories: [src_inc_dir, ext_inc_dir],
                      cpp_args: compiler_args,
                      link_args: linker_args,
                      dependencies: [boost, cgal, omp])

rightstep = executable('rightstep',
                                      ['src/subtrajectory_clustering.cpp'] + sources,
                                      include_directories: [src_inc_dir, ext_inc_dir],
                                      cpp_args: compiler_args,
                                      link_args: linker_args,
                                      dependencies: [boost, cgal, omp])

basic = executable('basic',
                                      ['src/basic_clustering.cpp'] + sources,
                                      include_directories: [src_inc_dir, ext_inc_dir],
                                      cpp_args: compiler_args,
                                      link_args: linker_args,
                                      dependencies: [boost, cgal, omp])

bbgll = executable('bbgll',
                                      ['src/trajectory_bbgll.cpp'] + sources,
                                      include_directories: [src_inc_dir, ext_inc_dir],
                                      cpp_args: compiler_args,
                                      link_args: linker_args,
                                      dependencies: [boost, cgal, omp])
#--------------#
# Unit Testing #
#--------------#

gtest = dependency('gtest', required: false)
gtest_main = dependency('gtest_main', required: false)

if gtest.found() and gtest_main.found()

  test_inc_dir = include_directories('test')

  free_space_test = executable('free_space_test',
                               ['test/free_space_test.cpp'] + sources,
                               include_directories: [src_inc_dir,
                                                     ext_inc_dir,
                                                     test_inc_dir],
                               dependencies: [gtest, gtest_main, boost, cgal])
  test('free_space', free_space_test, protocol: 'gtest')

  subtrajectory_cluster_test = executable('subtrajectory_cluster_test',
                               ['test/subtrajectory_cluster_test.cpp'] + sources,
                               include_directories: [src_inc_dir,
                                                     ext_inc_dir,
                                                     test_inc_dir],
                               dependencies: [gtest, gtest_main, boost, cgal, omp])
  test('subtrajectory_cluster', subtrajectory_cluster_test, protocol: 'gtest')

  kcluster_test = executable('kcluster_test',
                               ['test/kcluster_test.cpp'] + sources,
                               include_directories: [src_inc_dir,
                                                     ext_inc_dir,
                                                     test_inc_dir],
                               dependencies: [gtest, gtest_main, boost, cgal, omp])
  test('kcluster', kcluster_test, protocol: 'gtest')

  range_search_test = executable('range_search_test',
                               ['test/range_search_test.cpp'] + sources,
                               include_directories: [src_inc_dir,
                                                     ext_inc_dir,
                                                     test_inc_dir],
                               dependencies: [gtest, gtest_main, boost, cgal])
  test('range_search', range_search_test, protocol: 'gtest')

  frechet_distance_test = executable('frechet_distance_test',
                               ['test/frechet_distance_test.cpp'] + sources,
                               include_directories: [src_inc_dir,
                                                     ext_inc_dir,
                                                     test_inc_dir],
                               dependencies: [gtest, gtest_main, boost, cgal])
  test('frechet_distance', frechet_distance_test, protocol: 'gtest')

  free_space_incremental_test = executable('free_space_incremental_test',
                               ['test/free_space_incremental_test.cpp'] + sources,
                               include_directories: [src_inc_dir,
                                                     ext_inc_dir,
                                                     test_inc_dir],
                              cpp_args: ['-fsanitize=address,undefined'],
                              link_args: ['-fsanitize=address,undefined'],
                               dependencies: [gtest, gtest_main, boost, cgal])
  test('free_space_incremental', free_space_incremental_test, protocol: 'gtest')
endif
