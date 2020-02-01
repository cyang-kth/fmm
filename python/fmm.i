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

%include "python_types.hpp"

%template (CandidateVector) std::vector<MM::CandidateElement>;
%template (TransitionLattice) std::vector<MM::TLElement>;

namespace MM {
class MapMatcher {
public:
    MapMatcher(const std::string &config_file);
    MatchResult match_wkt(const std::string &wkt);
    CandidateSet search_candidate(const std::string &wkt);
    TransitionLattice build_transition_lattice(const std::string &wkt);
    ~MapMatcher();
    // Getter and setter to change the configuration in Python interactively.
    void set_gps_error(double error){
        config.gps_error = error;
    };
    double get_gps_error();
    void set_k(int k);
    int get_k();
    double get_radius();
    void set_radius(double r);
private:
    static MatchResult generate_result(Network *network_ptr,O_Path *o_path_ptr,
                                       T_Path *t_path_ptr, LineString *mgeom);
    MM::UBODT *ubodt;
    MM::Network *network;
    MapMatcherConfig config;
};
};
