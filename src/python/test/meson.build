py_test_src = [
    'module.cpp'
]

py_test = shared_library(
    '_test',
    py_test_src,
    include_directories: root_include_dir,
    dependencies: [py_dep, python_dep],
    name_prefix: ''
)
