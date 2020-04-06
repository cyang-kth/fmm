%module fmm
%include "std_string.i"
%include "std_vector.i"
%include "std_shared_ptr.i"
%shared_ptr(FMM::MM::UBODT)



%{
/* Put header files here or function declarations like below */
#include "core/geometry.hpp"
#include "network/type.hpp"
#include "network/network.hpp"
#include "network/network_graph.hpp"
#include "python/pyfmm.hpp"
#include "mm/mm_type.hpp"
#include "mm/fmm/fmm_algorithm.hpp"
#include "mm/stmatch/stmatch_algorithm.hpp"
#include "mm/fmm/ubodt.hpp"
using namespace FMM;
using namespace FMM::CORE;
using namespace FMM::NETWORK;
using namespace FMM::MM;
%}

%template(IntVector) std::vector<int>;
%template(UnsignedIntVector) std::vector<unsigned int>;
%template(DoubleVector) std::vector<double>;
%template(PyCandidateVector) std::vector<FMM::PYTHON::PyCandidate>;
// %template(DoubleVVector) vector<vector<double> >;
// %template(DoubleVVVector) vector<vector<vector<double> > >;
// %template(IntSet) set<int>;


%include "core/geometry.hpp"
%include "mm/mm_type.hpp"
%include "network/type.hpp"
%include "network/network.hpp"
%include "python/pyfmm.hpp"
%include "mm/fmm/ubodt.hpp"
%include "network/network_graph.hpp"
%include "mm/fmm/fmm_algorithm.hpp"
%include "mm/stmatch/stmatch_algorithm.hpp"
