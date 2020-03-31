//
// Created by Can Yang on 2020/3/8.
//

#ifndef MM_INCLUDE_MM_MM_RESULT_HPP_
#define MM_INCLUDE_MM_MM_RESULT_HPP_

#include "network/type.hpp"
#include "mm/transition_graph.hpp"

namespace MM{

struct MatchResult {
  int id;
  TGOpath opt_candidate_path;
  O_Path opath;
  C_Path cpath;
  std::vector<int> indices; // index of opath edge in cpath
  LineString mgeom;
};

};

#endif //MM_INCLUDE_MM_MM_RESULT_HPP_
