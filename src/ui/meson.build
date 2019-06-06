ui_src = [
    'application.cpp'
]

ui = static_library(
    'ui',
    ui_src,
    include_directories: root_include_dir,
    dependencies: [sdl2_dep]
)

ui_dep = declare_dependency(link_with: ui)
