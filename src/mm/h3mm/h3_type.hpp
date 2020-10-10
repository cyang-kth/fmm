#ifndef H3_TYPE_HEADER
#define H3_TYPE_HEADER

#include "h3_header.hpp"
#include <inttypes.h>
#include <stdio.h>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <vector>

namespace FMM{
namespace MM{

struct H3MatchResult {
  int traj_id;
  std::vector<H3Index> hexs;
};

};
};
#endif
