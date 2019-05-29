NORMAL_FLAGS = -std=gnu++11 -O3 -Wno-unknown-pragmas
LIBS = -lgdal -lboost_serialization
PYTHON_BG_FLAGS = -I../include -DUSE_BG_GEOMETRY
build:initialize open_python_bg_flag
	swig -python -c++ -outdir dist -o extensions/fmm_wrap.cxx fmm.i
	g++ $(NORMAL_FLAGS) -fPIC -c extensions/fmm_wrap.cxx -o build/fmm_wrap.o -I. -I/usr/include/python2.7 $(LIBS)
	g++ -shared build/fmm_wrap.o -o dist/_fmm.so $(LIBS)
initialize:
	mkdir -p extensions
	mkdir -p build
	mkdir -p dist
open_python_bg_flag:
	$(eval NORMAL_FLAGS += $(PYTHON_BG_FLAGS))
test_fmm_python:
	python fmm_test.py
clean:
	rm -r build
	rm -r extensions
	rm -r dist
	rm -f *.pyc
