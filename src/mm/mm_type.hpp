//
// Created by Can Yang on 2020/3/8.
//

#ifndef MM_INCLUDE_MM_MM_TYPE_HPP_
#define MM_INCLUDE_MM_MM_TYPE_HPP_

#include "network/type.hpp"

namespace MM{

struct MatchResult {
  int id;
  O_Path opath;
  C_Path cpath;
  LineString mgeom;
};

};

#endif //MM_INCLUDE_MM_MM_TYPE_HPP_
