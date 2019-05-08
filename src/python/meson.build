py_src = [
    'pythonapi.cpp',
    'instance.cpp'
]

py = static_library(
    'py',
    py_src,
    include_directories: root_include_dir,
    dependencies: [python_dep]
)

py_dep = declare_dependency(link_with: py)

subdir('test')
