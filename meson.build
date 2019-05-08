project('morph', 'cpp', version: '0.1')

#
#   Fetch Conan dependencies.
#

tbb_dep = subproject('TBB').get_variable('tbb_dep')
googletest_dep = subproject('gtest').get_variable('gtest_dep')
eigen_dep = subproject('eigen').get_variable('eigen_dep')
sdl2_dep = subproject('sdl2').get_variable('sdl2_dep')
gtest_dep = subproject('gtest').get_variable('gtest_dep')

thread_dep = dependency('threads')
python_dep = dependency('python3')
thread_dep = dependency('threads')

subdir('src')
