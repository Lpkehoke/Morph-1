from conans import ConanFile, Meson, tools
from scripts import generate_subproject

import os


class Morph(ConanFile):
    settings = 'os', 'compiler', 'build_type', 'arch', 'cppstd'

    requires = (
        'eigen/3.3.7@conan/stable',
        'glad/0.1.29@bincrafters/stable',
        'gtest/1.8.1@bincrafters/stable',
        'sdl2/2.0.9@bincrafters/stable',
        'TBB/2019_U4@conan/stable'
    )

    generators = {}

    def configure(self):
        self.options['glad'].profile = 'core'
        self.options['glad'].api_type = 'gl'
        self.options['glad'].api_version = '4.1'
        self.options['glad'].spec = 'gl'
        self.options['glad'].no_loader = False
        self.options['glad'].shared = True

    def create_meson_subprojects(self):
        subprojects_path = os.path.join(
            os.path.dirname(os.path.abspath(__file__)),
            'subprojects')

        if not os.path.exists(subprojects_path):
            os.mkdir(subprojects_path)
        
        deps = self.deps_cpp_info.deps

        for dependency_name in deps:
            lib_names = self.deps_cpp_info[dependency_name].libs
            lib_dirs = self.deps_cpp_info[dependency_name].lib_paths
            include_dirs = self.deps_cpp_info[dependency_name].include_paths

            project_dir = os.path.join(subprojects_path, dependency_name)
            project_file = os.path.join(project_dir, 'meson.build')

            if not os.path.exists(project_dir):
                os.mkdir(project_dir)

            with open(project_file, 'w', newline='') as project:
                payload = generate_subproject(dependency_name, lib_names, lib_dirs, include_dirs)
                project.writelines(payload)

    def build(self):
        self.create_meson_subprojects()

        meson = Meson(self)
        meson.configure(build_folder='build')
        meson.build()

    def package(self):
        self.copy('*.so', dst='', keep_path=False)

        # copy tbb libs
        tbb_lib_dir = self.deps_cpp_info['TBB'].libdirs[0]
        self.copy('*.so*', src=tbb_lib_dir, dst='', keep_path=False)
