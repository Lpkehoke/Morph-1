vk_src = [
    'vkrenderer.cpp'
]

vk = static_library(
    'vk',
    vk_src,
    include_directories: root_include_dir,
    dependencies: [volk_dep]
)

vk_dep = declare_dependency(link_with: vk)
