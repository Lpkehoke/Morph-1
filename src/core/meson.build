core_src = [
  'logger.cpp',
]

core = static_library(
    'core',
    core_src,
    include_directories: root_include_dir,
    dependencies: [foundation_dep, tbb_dep]
)

core_dep = declare_dependency(link_with: core)
