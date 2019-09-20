### Python interface for fmm designed with SWIG

Check the jupyter notebook file [fmm_demo.ipynb](fmm_demo.ipynb) for a demo.

#### Requirements

- Linux/Unix environment (tested on Ubuntu 16.04)
- swig 4.0.0
- Python 2.7

In addition to the requirements of fmm, install `python-dev` with

```
sudo apt-get install python-dev
```

#### Swig installation

To install SWIG, download the swig file `swig-4.0.0.tar.gz` from http://www.swig.org/download.html.

```
# Install dependency
sudo apt-get install build-essential libpcre3-dev libpcre3

# Build swig
wget http://prdownloads.sourceforge.net/swig/swig-4.0.1.tar.gz
tar -xf swig-4.0.0.tar.gz
cd swig-4.0.0/
./configure
sudo make
sudo make install

# Verify your installation
swig -version
```

#### Installation of fmm Python API

To install the python extension, under the project folder run

```
    cd python
    mkdir build
    cd build
    cmake ..
    make
```

Add the `build` folder to the environment variable `PYTHONPATH`:

```
    echo 'export PYTHONPATH=${PYTHONPATH}:PATH_TO_BUILD_FOLDER' >> ~/.bashrc
    source ~/.bashrc
```

#### Verification of installation

Under the `python` folder in the project run

```
    python fmm_test.py
```

#### Run fmm as a web demo.

Check [OSM map matching](https://github.com/cyang-kth/osm_mapmatching).


#### Common error with installation

1. Run `python fmm_test.py` failure

The problem could be multiple version of Python cannot be recognized correctly in
cmake. Make sure that the default python version

```
which python
```

is the same as the Python library used in cmake build.

```
cmake ..  
```

To manually specify the Python version used in CMake,
following this answer: https://stackoverflow.com/a/16045924/1484942.

Edit the file `fmm/python/build/CMakeCache.txt` to replace python3.5 with python2.7.

```
PYTHON_INCLUDE_DIR:PATH=/usr/include/python2.7
PYTHON_LIBRARY:FILEPATH=/usr/lib/x86_64-linux-gnu/libpython2.7.so
_fmm_LIB_DEPENDS:STATIC=general;/usr/lib/x86_64-linux-gnu/libpython2.7.so;general;/usr/lib/libgdal.so;general;/usr/lib/x86_64-linux-gnu/libboost_serialization.so;
FIND_PACKAGE_MESSAGE_DETAILS_PythonLibs:INTERNAL=[/usr/lib/x86_64-linux-gnu/libpython2.7.so][/usr/include/python2.7][v2.7.12()]
```

### Acknowledgement

Great Acknowledgement is given to [Edi Buslovich](https://github.com/edibusl) for improving the installation instructions.

### Reference

- Swig and Python documentation: http://www.swig.org/Doc1.3/Python.html#Python_nn13
- Swig and C++: http://www.swig.org/Doc4.0/SWIGPlus.html
