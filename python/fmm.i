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
#include "mm/mm_result.hpp"
#include "mm/stmatch/stmatch_algorithm.hpp"
#include "mm/fmm/ubodt.hpp"
using namespace MM;
%}

%template(IntVector) std::vector<int>;
%template(UnsignedIntVector) std::vector<unsigned int>;
%template(DoubleVector) std::vector<double>;
%template(PyCandidateVector) std::vector<MM::PyCandidate>;
// %template(DoubleVVector) vector<vector<double> >;
// %template(DoubleVVVector) vector<vector<vector<double> > >;
// %template(IntSet) set<int>;

namespace MM{

%typemap(in,numinputs=0) PyMatchResult& %{
  $1 = new PyMatchResult;
%}

%typemap(argout) PyMatchResult& (PyObject* o) %{
  o = SWIG_NewPointerObj($1, $1_descriptor, SWIG_POINTER_OWN);
  $result = SWIG_Python_AppendOutput($result, o);
%}

}



%include "core/gps.hpp"
%include "core/geometry.hpp"
%include "network/type.hpp"
%include "network/network.hpp"
%include "network/network_graph.hpp"
%include "mm/mm_result.hpp"
%include "mm/fmm/fmm_algorithm.hpp"
%include "mm/stmatch/stmatch_algorithm.hpp"
%include "mm/fmm/ubodt.hpp"
