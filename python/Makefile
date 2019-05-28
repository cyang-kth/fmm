NORMAL_FLAGS = -std=gnu++11 -O3 -Wno-unknown-pragmas
LIBS = -lgdal -lboost_serialization
PYTHON_BG_FLAGS = -I../include -DUSE_BG_GEOMETRY

PY_DIR = .
SWIG_OUTPUT_DIR = extensions

build_fmm_python:open_python_bg_flag
	swig -python -c++ -outdir . -o extensions/fmm_wrap.cxx fmm.i
	g++ $(NORMAL_FLAGS) -fPIC -c fmm.h extensions/fmm_wrap.cxx -I. -I/usr/include/python2.7 $(LIBS)
	g++ -shared fmm_wrap.o -o _fmm.so $(LIBS)
open_python_bg_flag:
	$(eval NORMAL_FLAGS += $(PYTHON_BG_FLAGS))
test_fmm_python:
	python fmm_test.py
clean_python:
	rm -f ./*_wrap.c
	rm -f ./*_wrap.cxx
	rm -f ./*.gch
	rm -f ./*.o
	rm -f ./*.so
	rm -f ./*.pyc
