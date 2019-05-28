include ../Makefile

PYTHON_BG_FLAGS = -I../include -DUSE_BG_GEOMETRY

PY_DIR = .
SWIG_OUTPUT_DIR = extensions
open_python_bg_flag:
	$(eval NORMAL_FLAGS += $(PYTHON_BG_FLAGS))
build_boost_example:
	g++ -O3 linestring_example.cpp -o bin/linestring_example
build_mygdal_cpp:
	g++ -O3 -std=c++11 mygdal.cpp -o bin/mygdal.exe -lgdal -I../include
build_mygdal:
	# g++ -O3 -std=c++11 gdal.cpp -o gdal.exe -lgdal
	swig -python -c++ mygdal.i
	g++ -O2 -fPIC -std=c++11 -c mygdal.hpp mygdal_wrap.cxx -I/usr/include/python2.7 -lgdal
	g++ -pthread -shared -L/home/cyang/anaconda2/lib -Wl,-rpath=/home/cyang/anaconda2/lib -shared mygdal_wrap.o -lgdal -L/home/cyang/anaconda2/lib -lpython2.7 -o _mygdal.so
build_fmm_python:open_python_bg_flag
	swig -python -c++ -outdir . -o extensions/fmm_wrap.cxx fmm.i
	g++ $(NORMAL_FLAGS) -fPIC -c fmm.hpp extensions/fmm_wrap.cxx -I. -I/usr/include/python2.7 $(LIBS)
	g++ -shared fmm_wrap.o -o _fmm.so $(LIBS)
test_fmm_python:
	python fmm_test.py
build_example:
	swig -python example.i
	gcc -fpic -c example.c example_wrap.c -I/usr/include/python2.7
	gcc -shared example.o example_wrap.o -o _example.so
clean_python:
	rm -f ./*_wrap.c
	rm -f ./*_wrap.cxx
	rm -f ./*.gch
	rm -f ./*.o
	rm -f ./*.so
	rm -f ./*.pyc
