/**
 * Content
 * Definition of Data types used in the FMM python extension
 *
 * @author: Can Yang
 * @version: 2019.06.14
 */

#ifndef MM_PYTHON_TYPES_HPP
#define MM_PYTHON_TYPES_HPP

#include <vector>
#include <string>
#include "../src/geometry_type.hpp"

namespace MM {

struct MatchResult {
  std::vector<int> opath;
  std::vector<int> cpath;
  std::string mgeom;
  std::string pgeom;
};

/**
 *  Candidate class, used for verifying the result in Python
 */
struct CandidateElement {
  int index;       // point index in trajectory
  int edge_id;       // edge id
  int source;
  int target;
  double distance;       // distance to that edge
  double length;
  double offset;
  double ep;       // emission probability
};

typedef std::vector<CandidateElement> CandidateSet;

struct TLElement {
  int index;       // point index in trajectory
  int from_edge;
  int to_edge;
  double sp_dist;
  double eu_dist;
  double tp;       // transition probability
  double e_prob;       // emission probability
  double cumu_prob;       // cumulative emission probability
};

typedef std::vector<TLElement> TransitionLattice;
}
#endif /* MM_PYTHON_TYPES_HPP */
