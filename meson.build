project('morph', 'cpp', version: '0.1')

add_project_link_arguments('-lstdc++fs', language : 'cpp')
add_project_arguments('-fvisibility=hidden', language: 'cpp')

#
#   Fetch Conan dependencies.
#

tbb_dep = subproject('TBB').get_variable('tbb_dep')
eigen_dep = subproject('eigen').get_variable('eigen_dep')
sdl2_dep = subproject('sdl2').get_variable('sdl2_dep')
gtest_dep = subproject('gtest').get_variable('gtest_dep')
volk_dep = subproject('volk').get_variable('volk_dep')

python_dep = dependency('python3')
thread_dep = dependency('threads')

subdir('src')

if get_option('generate_docs')
    subdir('docs')
endif
