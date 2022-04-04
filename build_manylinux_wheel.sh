#!/usr/bin/env bash

# Note: this script should run in manylinux2010 docker container. Please
# use docker/Dockerfile.manylinux2010 to build the image.

set -e

script_file=$(readlink -f $0)
script_dir=$(dirname $script_file)
cd "$script_dir"

version=$1
if [ -n "$version" ]; then
    echo "Rewriting python/_version.py"
    sed -i s"/^__version__\s*=.*/__version__ = '$version'/" python/_version.py
fi

echo "Building FMM package for cp38-cp38"
export PATH=/opt/python/cp38-cp38/bin:$PATH
mkdir -p build && cd build
cmake .. -DPYTHON_INCLUDE_DIR=$(python -c "from distutils.sysconfig import get_python_inc; print(get_python_inc())") -DPYTHON_LIBRARY=$(python -c "import distutils.sysconfig as sysconfig; print(sysconfig.get_config_var('LIBDIR'))")
make -j2
make install
auditwheel repair python/dist/*.whl

echo "Building FMM package for cp37-cp37m"
export PATH=/opt/python/cp37-cp37m/bin:$PATH
rm -rf python
cmake .. -DPYTHON_INCLUDE_DIR=$(python -c "from distutils.sysconfig import get_python_inc; print(get_python_inc())") -DPYTHON_LIBRARY=$(python -c "import distutils.sysconfig as sysconfig; print(sysconfig.get_config_var('LIBDIR'))")
make
auditwheel repair python/dist/*.whl

echo "Building FMM package for cp36-cp36m"
export PATH=/opt/python/cp36-cp36m/bin:$PATH
rm -rf python
cmake .. -DPYTHON_INCLUDE_DIR=$(python -c "from distutils.sysconfig import get_python_inc; print(get_python_inc())") -DPYTHON_LIBRARY=$(python -c "import distutils.sysconfig as sysconfig; print(sysconfig.get_config_var('LIBDIR'))")
make
auditwheel repair python/dist/*.whl
