%module fmm
%include "std_string.i"
%include "std_vector.i"

%{
/* Put header files here or function declarations like below */
#include "fmm.h"
%}

namespace std {
   %template(IntVector) vector<int>;
   // %template(DoubleVector) vector<double>;
   // %template(DoubleVVector) vector<vector<double> >;
   // %template(DoubleVVVector) vector<vector<vector<double> > >;
   // %template(IntSet) set<int>;
}


%include "fmm.h"
