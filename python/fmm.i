%module fmm
%include "std_string.i"
%include "std_vector.i"

%{
/* Put header files here or function declarations like below */
#include "network/type.hpp"
#include "core/gps.hpp"
#include "core/geometry.hpp"
#include "network/network.hpp"
#include "network/network_graph.hpp"
#include "mm/fmm/fmm_algorithm.hpp"
#include "mm/stmatch/stmatch_algorithm.hpp"
#include "mm/fmm/ubodt.hpp"
using namespace MM;
%}

%typemap(in,numinputs=0) MatchResult& (MatchResult tmp) %{
  $1 = &tmp;
%}

%typemap(argout) MatchResult& (PyObject* o) %{
  o = SWIG_NewPointerObj($1, $1_descriptor, SWIG_POINTER_OWN);
  $result = SWIG_Python_AppendOutput($result, o);
%}

%typemap(in,numinputs=0) PythonResult& (PythonResult tmp) %{
  $1 = &tmp;
%}

%typemap(argout) PythonResult& (PyObject* o) %{
  o = SWIG_NewPointerObj($1, $1_descriptor, SWIG_POINTER_OWN);
  $result = SWIG_Python_AppendOutput($result, o);
%}


namespace std {
   %template(IntVector) vector<int>;
   %template(UnsignedIntVector) vector<unsigned int>;
   %template(DoubleVector) vector<double>;
   %template(MatchedCandidateVector) vector<MatchedCandidate>;
   // %template(DoubleVVector) vector<vector<double> >;
   // %template(DoubleVVVector) vector<vector<vector<double> > >;
   // %template(IntSet) set<int>;
}

%include "core/gps.hpp"
%include "core/geometry.hpp"
%include "network/type.hpp"
%include "network/network.hpp"
%include "network/network_graph.hpp"
%include "mm/fmm/fmm_algorithm.hpp"
%include "mm/stmatch/stmatch_algorithm.hpp"
%include "mm/fmm/ubodt.hpp"
