### Python interface for fmm designed with SWIG

#### Build a python example

```
# swig -python example.i
swig -python example.i
gcc -fpic -c example.c example_wrap.c -I/usr/include/python2.7
gcc -shared example.o example_wrap.o -o _example.so
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
