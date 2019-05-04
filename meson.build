project('morph', 'cpp', version: '0.1')


#
#   Fetch Conan dependencies.  
#

eigen_dep = subproject('eigen').get_variable('eigen_dep')
sdl2_dep = subproject('sdl2').get_variable('sdl2_dep')

python_dep = dependency('python3')

subdir('src')
