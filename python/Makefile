PY_DIR = .
SWIG_OUTPUT_DIR = extensions

build_boost_example:
	g++ -O3 linestring_example.cpp -o bin/linestring_example
build_mygdal_cpp:
	g++ -O3 -std=c++11 mygdal.cpp -o bin/mygdal.exe -lgdal -I../include
build_mygdal:
	# g++ -O3 -std=c++11 gdal.cpp -o gdal.exe -lgdal
	swig -python -c++ mygdal.i
	g++ -O2 -pthread -fPIC -std=c++11 -c mygdal.hpp mygdal_wrap.cxx -I/usr/include/python2.7 -lgdal
	g++ -pthread -shared -L/home/cyang/anaconda2/lib -Wl,-rpath=/home/cyang/anaconda2/lib -shared mygdal_wrap.o -lgdal -L/home/cyang/anaconda2/lib -lpython2.7 -o _mygdal.so
build_fmm:
	swig -python -c++ -outdir . -o extensions/fmm_wrap.cxx fmm.i
	g++ -DCS_DEBUG_LEVEL='3' -O2 -fpic -std=c++11 -c fmm.hpp extensions/fmm_wrap.cxx -I. -I/usr/include/python2.7 -lgdal -lboost_serialization
	g++ -shared fmm_wrap.o -o _fmm.so -lgdal -lboost_serialization
test_fmm:
	python fmm_test.py
build_example:
	swig -python example.i
	gcc -fpic -c example.c example_wrap.c -I/usr/include/python2.7
	gcc -shared example.o example_wrap.o -o _example.so
clean:
	rm -f ./*_wrap.c
	rm -f ./*_wrap.cxx
	rm -f ./*.gch
	rm -f ./*.o
	rm -f ./*.so
	rm -f ./*.pyc
