//
// Created by Can Yang on 2020/3/8.
//

#ifndef MM_INCLUDE_MM_MM_RESULT_HPP_
#define MM_INCLUDE_MM_MM_RESULT_HPP_

#include "network/type.hpp"

namespace MM{

struct MatchedCandidate{
  Candidate c;
  double ep;
  double tp; // transition probability to previous matched candidate
  double sp_dist;
};

typedef std::vector<MatchedCandidate> MatchedCandidatePath;

struct MatchResult {
  int id;
  MatchedCandidatePath opt_candidate_path;
  O_Path opath;
  C_Path cpath;
  std::vector<int> indices; // index of opath edge in cpath
  LineString mgeom;
};

// POD type for python map matching result

struct PyCandidate{
  int index;       // point index in trajectory
  int edge_id;       // edge id
  int source;
  int target;
  double error;
  double offset;
  double length;
  double ep;
  double tp;
  double spdist;
};

struct PyMatchResult {
  int id;
  O_Path opath;
  C_Path cpath;
  // std::vector<PyCandidate> candidates;
  std::vector<int> indices; // index of opath edge in cpath
  // LineString mgeom;
  // LineString pgeom;
};

};

#endif //MM_INCLUDE_MM_MM_RESULT_HPP_
