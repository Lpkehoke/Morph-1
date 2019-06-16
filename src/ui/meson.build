ui_src = [
    'application.cpp'
]

ui = static_library(
    'ui',
    ui_src,
    include_directories: root_include_dir,
    dependencies: [eigen_dep, sdl2_dep, vk_dep]
)

ui_dep = declare_dependency(link_with: ui)
