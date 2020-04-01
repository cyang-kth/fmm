//
// Created by Can Yang on 2020/3/8.
//

#ifndef MM_INCLUDE_MM_MM_RESULT_HPP_
#define MM_INCLUDE_MM_MM_RESULT_HPP_

#include "network/type.hpp"

namespace MM{

struct MatchedCandidate{
  const Candidate *c;
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

};

#endif //MM_INCLUDE_MM_MM_RESULT_HPP_
