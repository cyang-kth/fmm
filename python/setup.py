#!/usr/bin/env python

from setuptools import setup
from setuptools.dist import Distribution
from setuptools.command.install import install
from wheel.bdist_wheel import bdist_wheel as _bdist_wheel
from _version import __version__


# Force building a non-pure (py3-none-any) wheel when extensions were not
# specified in setup configs. What we want is a wheel with platform-specific
# tag such as cp38-cp38-linux_x86_64.
class bdist_wheel(_bdist_wheel):
    def finalize_options(self):
        _bdist_wheel.finalize_options(self)
        self.root_is_pure = False


# Workaround a distutils bug, please refer to https://github.com/google/or-tools/issues/616
class InstallPlatlib(install):
    def finalize_options(self):
        install.finalize_options(self)
        self.install_lib = self.install_platlib


def wheel_name(**kwargs):
    dist = Distribution(attrs=kwargs)
    bdist_wheel_cmd = dist.get_command_obj('bdist_wheel')
    bdist_wheel_cmd.ensure_finalized()
    distname = bdist_wheel_cmd.wheel_dist_name
    tag = '-'.join(bdist_wheel_cmd.get_tag())
    return '%s-%s.whl' % (distname, tag)


def get_wheel_name():
    return wheel_name(**setup_args)


setup_args = dict(
    name='FastMM',
    version=__version__,
    description='Fast map matching, an open source framework in C++',
    url='https://github.com/cyang-kth/fmm',
    packages=['fmm'],
    package_dir={'fmm': '.'},
    package_data={'fmm': ['*.so', '*.dll', '*.dylib']},
    cmdclass={'bdist_wheel': bdist_wheel, 'install': InstallPlatlib})


if __name__ == '__main__':
    setup(**setup_args)
