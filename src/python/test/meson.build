py_test_src = [
    'module.cpp'
]

py_test = shared_library(
    '_test',
    py_test_src,
    include_directories: root_include_dir,
    dependencies: [python_dep],
    c_args: hidden_visibility_args,
    name_prefix: ''
)
