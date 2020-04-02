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
#include "mm/mm_result.hpp"
using namespace MM;
%}

namespace std {
   %template(IntVector) vector<int>;
   %template(UnsignedIntVector) vector<unsigned int>;
   %template(DoubleVector) vector<double>;
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
%include "mm/mm_result.hpp"
