subdir('heterogeneous')

foundation_src = [
  'murmurhash.cpp',
  'taskqueue.cpp',
]

foundation = static_library(
    'foundation',
    [foundation_src, heterogeneous_src],
    include_directories: root_include_dir,
    dependencies: [tbb_dep]
)

foundation_dep = declare_dependency(link_with: foundation)
