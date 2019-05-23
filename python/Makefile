build_mygdal:
	# g++ -O3 -std=c++11 gdal.cpp -o gdal.exe -lgdal
	swig -Wall -python -c++ -lpointer.i mygdal.i 
	g++ -O3 -fpic -std=c++11 -c mygdal.hpp mygdal_wrap.cxx -I/usr/include/python2.7 -lgdal
	g++ -shared mygdal_wrap.o -o _mygdal.so -lgdal
build_fmm:
	swig -python -c++ -outdir . -o extensions/fmm_wrap.cxx fmm.i
	g++ -DCS_DEBUG_LEVEL='3' -O3 -fpic -std=c++11 -c fmm.hpp extensions/fmm_wrap.cxx -I. -I/usr/include/python2.7 -lgdal -lboost_serialization
	g++ -shared fmm_wrap.o -o _fmm.so -lgdal -lboost_serialization
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
