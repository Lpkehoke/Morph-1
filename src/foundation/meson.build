foundation_src = [
    'taskqueue.cpp'
]

foundation = static_library(
    'foundation',
    foundation_src,
    include_directories: root_include_dir,
    dependencies: [tbb_dep]
)

foundation_dep = declare_dependency(link_with: foundation)
