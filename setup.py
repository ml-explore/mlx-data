# Copyright © 2023 Apple Inc.

import datetime
import os
import subprocess
from pathlib import Path
import platform
import sys
import sysconfig
import pybind11

from setuptools import Extension, find_namespace_packages, setup
from setuptools.command.build_ext import build_ext


def get_version(version):
    if "PYPI_RELEASE" not in os.environ:
        today = datetime.date.today()
        version = f"{version}.dev{today.year}{today.month}{today.day}"

        if "DEV_RELEASE" not in os.environ:
            git_hash = (
                subprocess.run(
                    "git rev-parse --short HEAD".split(),
                    capture_output=True,
                    check=True,
                )
                .stdout.strip()
                .decode()
            )
            version = f"{version}+{git_hash}"

    return version


class CMakeExtension(Extension):
    def __init__(self, name: str, sourcedir: str = "") -> None:
        super().__init__(name, sources=[])
        self.sourcedir = os.fspath(Path(sourcedir).resolve())


class CMakeBuild(build_ext):
    def build_extension(self, ext) -> None:
        # Build other extensions
        if not isinstance(ext, CMakeExtension):
            super().build_extension(ext)
            return

        if platform.system() == "Windows":
            cmake_python_library = "{}/libs/python{}.lib".format(
                sysconfig.get_config_var("prefix"),
                sysconfig.get_config_var("VERSION"),
            )
            if not os.path.exists(cmake_python_library):
                cmake_python_library = "{}/libs/python{}.lib".format(
                    sys.base_prefix,
                    sysconfig.get_config_var("VERSION"),
                )
        else:
            cmake_python_library = "{}/{}".format(
                sysconfig.get_config_var("LIBDIR"),
                sysconfig.get_config_var("INSTSONAME"),
            )
        cmake_python_include_dir = sysconfig.get_path("include")

        ext_fullpath = Path.cwd() / self.get_ext_fullpath("dummy")
        extdir = ext_fullpath.parent.resolve()
        extdir.mkdir(parents=True, exist_ok=True)

        cmake_args = [
            "-DMLX_BUILD_PYTHON_BINDINGS=ON",
            "-DMLX_DATA_VERSION={}".format(self.distribution.get_version()),
            "-DCMAKE_INSTALL_PREFIX={}".format(extdir),
            "-DPython_EXECUTABLE={}".format(sys.executable),
            "-DPython_LIBRARIES={}".format(cmake_python_library),
            "-DPython_INCLUDE_DIRS={}".format(cmake_python_include_dir),
            "-DCMAKE_BUILD_TYPE={}".format("Debug" if self.debug else "Release"),
            "-DCMAKE_PREFIX_PATH={}".format(pybind11.get_cmake_dir()),
        ]
        if "CMAKE_ARGS" in os.environ:
            cmake_args += [item for item in os.environ["CMAKE_ARGS"].split(" ") if item]

        # Finally run install
        Path(self.build_temp).mkdir(parents=True, exist_ok=True)
        subprocess.run(
            ["cmake", ext.sourcedir] + cmake_args, cwd=self.build_temp, check=True
        )
        subprocess.run(
            ["cmake", "--build", ".", "--target", "install"],
            cwd=self.build_temp,
            check=True,
        )


if __name__ == "__main__":
    packages = find_namespace_packages(where="python", exclude=["src", "tests"])
    package_dir = {"": "python"}

    setup(
        name="mlx-data",
        version=get_version("0.0.1"),
        author="Ronan Collobert",
        author_email="collobert@apple.com",
        url="https://github.com/ml-explore/mlx-data",
        license="MIT",
        description="Universal data loaders",
        packages=packages,
        package_dir=package_dir,
        ext_modules=[CMakeExtension("mlx.data._c")],
        cmdclass={"build_ext": CMakeBuild},
        zip_safe=False,
    )
