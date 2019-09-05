### Python interface for fmm designed with SWIG

Check the jupyter notebook file [fmm_demo.ipynb](fmm_demo.ipynb) for a demo.

#### Requirements

- Linux/Unix environment (tested on Ubuntu 14.04)
- swig 4.0.0
- Python 2.7

#### Installation

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

#### Swig installation

Install SWIG, download the swig file as swig-4.0.0.tar.gz

```
tar -xf swig-4.0.0.tar.gz
cd swig-4.0.0/
./configure
sudo make
sudo make install
```

Verification of installation

```
$ swig -version
SWIG Version 4.0.0
Compiled with g++ [x86_64-pc-linux-gnu]
Configured options: +pcre
Please see http://www.swig.org for reporting bugs and further information
```

#### Common error with installation

If an error pops as missing libpcre.so.1, we can copy from anaconda

```
sudo mv ~/anaconda2/lib/libpcre.so.* /usr/lib
```

### Reference

- Swig and Python documentation: http://www.swig.org/Doc1.3/Python.html#Python_nn13
- Swig and C++: http://www.swig.org/Doc4.0/SWIGPlus.html
