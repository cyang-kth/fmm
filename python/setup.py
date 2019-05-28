from distutils.core import setup, Extension

'''
Install with
    CC=g++ python setup.py build
    python setup.py install
'''

mygdal_module = Extension('fmm',
		sources = ['fmm.i'],
        swig_opts=['-c++'],
        extra_compile_args =['-O3','-std=c++11','-shared','-Wno-unknown-pragmas',
        '-DUSE_BG_GEOMETRY','-I../include','-lgdal','-lboost_serialization'])

setup ( name = 'fmm',
	version = '0.1',
    author = "Can Yang, cyang@kth.se",
	description = 'Fast map matching program',
	ext_modules = [ mygdal_module ])
