py_src = [
    'pythonapi.cpp',
    'instance.cpp',
    'internals.cpp'
]

py = static_library(
    'py',
    py_src,
    include_directories: root_include_dir,
    dependencies: [foundation_dep, python_dep]
)

py_dep = declare_dependency(link_with: py)

subdir('ui')
subdir('test')
