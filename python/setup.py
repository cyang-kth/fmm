from distutils.core import setup, Extension

mygdal_module = Extension('mygdal',
		sources = ['mygdalmodule.cpp'],
        libraries=['gdal'])

setup ( name = 'Mygdal',
	version = '1.0',
	description = 'Sample module.',
	ext_modules = [ mygdal_module ])
