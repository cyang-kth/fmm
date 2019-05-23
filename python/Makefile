build_fmm:
	swig -python -c++ fmm.i
	g++ -DCS_DEBUG_LEVEL='3' -O3 -fpic -std=c++11 -c fmm.hpp fmm_wrap.cxx -I/usr/include/python2.7 -lgdal -lboost_serialization
	g++ -shared fmm_wrap.o -o _fmm.so -lgdal -lboost_serialization
build_example:
	swig -python example.i
	gcc -fpic -c example.c example_wrap.c -I/usr/include/python2.7
	gcc -shared example.o example_wrap.o -o _example.so
clean:
	rm ./*_wrap.c
	rm ./*.o
	rm ./*.so
	rm ./*.py
	rm ./*.pyc
