import jinja2
import os
import typing


def generate_subproject(
      dependency_name: str,
      lib_names: typing.List[str],
      lib_dirs: typing.List[str],
      include_dirs: typing.List[str]) -> str:
    loader = jinja2.FileSystemLoader(os.path.dirname(os.path.abspath(__file__)))
    env = jinja2.Environment(loader=loader, trim_blocks=True, lstrip_blocks=True)
    template = env.get_template('meson.build.template')

    has_threads_dependency = 'pthread' in lib_names
    lib_names = list(filter(lambda elem: elem != 'pthread', lib_names))

    context = {
        'conan_dependency_name': dependency_name,
        'lib_names': lib_names,
        'lib_dirs': lib_dirs,
        'include_dirs': include_dirs,
        'has_threads_dependency': has_threads_dependency
    }

    return template.render(context)
