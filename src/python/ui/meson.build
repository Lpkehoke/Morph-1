py_ui_src = [
    'module.cpp'
]

py_ui = shared_library(
    '_ui',
    py_ui_src,
    include_directories: root_include_dir,
    dependencies: [ui_dep, py_dep, python_dep],
    name_prefix: ''
)
