// gcc -pthread -fno-strict-aliasing -g -O2 -DNDEBUG -g -fwrapv -O3 -Wall -Wstrict-prototypes -fPIC -I/home/cyang/anaconda2/include/python2.7 -c spammodule.c -o build/temp.linux-x86_64-2.7/spammodule.o
// gcc -pthread -shared -L/home/cyang/anaconda2/lib -Wl,-rpath=/home/cyang/anaconda2/lib,--no-as-needed build/temp.linux-x86_64-2.7/spammodule.o -L/home/cyang/anaconda2/lib -lpython2.7 -o build/lib.linux-x86_64-2.7/spam.so
// gcc -fPIC -I/home/cyang/anaconda2/include/python2.7 -c mymodule.c -o build/temp.linux-x86_64-2.7/spammodule.o
// gcc -pthread -shared -L/home/cyang/anaconda2/lib -Wl,-rpath=/home/cyang/anaconda2/lib,--no-as-needed build/temp.linux-x86_64-2.7/spammodule.o -L/home/cyang/anaconda2/lib -lpython2.7 -o build/lib.linux-x86_64-2.7/spam.so

#include <Python.h>
#include "gdal/ogrsf_frmts.h" // C++ API for GDAL
#include <iostream>

static PyObject * mygdalError;

static PyObject *
test(PyObject *self, PyObject * args)
{
	const char *command;
	int sts;

	if(!PyArg_ParseTuple(args, "s", &command))
		return NULL;

    OGRLineString* cutoffline = new OGRLineString();
    cutoffline->addPoint(2.0,1.0);
    cutoffline->addPoint(2.0,0.0);
    cutoffline->addPoint(3.0,0.5);
    std::cout<<"Number of points "<< cutoffline->getNumPoints()<<"\n";
    std::cout<<"Length "<< cutoffline->get_Length()<<"\n";
    delete cutoffline;
    
	if ( sts < 0 ) {
		PyErr_SetString(mygdalError, "System command failed");
		return NULL;
	}
	return PyLong_FromLong(sts);
}

PyMODINIT_FUNC
initmygdal(void)
{
	PyObject *m;
	static PyMethodDef mygdalMethods[] = {
		{"test", test, METH_VARARGS, "Run test"},
		{NULL, NULL, 0, NULL}
	};
	m = Py_InitModule("mygdal", mygdalMethods);
	if ( m == NULL )
		return;

	mygdalError = PyErr_NewException("mygdal.error", NULL, NULL);
	Py_INCREF(mygdalError);
	PyModule_AddObject(m, "error", mygdalError);
}

int
main(int argc, char *argv[])
{
	Py_SetProgramName(argv[0]);
	Py_Initialize();
	initmygdal();
}
