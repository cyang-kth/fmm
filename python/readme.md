### Python interface for fmm designed with SWIG

#### Build a python example

```
# swig -python example.i
swig -python example.i
gcc -fpic -c example.c example_wrap.c -I/usr/include/python2.7
gcc -shared example.o example_wrap.o -o _example.so
```

#### Verify the result

```
python -c "import example;print example.fact(5)"
python -c "import mygdal;mygdal.test()"
```

#### Debug

```
valgrind --log-file="debug.log" python demo.py
```

To figure out the reason, we need to read this links
- https://github.com/OSGeo/gdal/blob/master/gdal/swig/include/python/ogr_python.i
- https://github.com/OSGeo/gdal/blob/master/gdal/swig/python/GNUmakefile


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
