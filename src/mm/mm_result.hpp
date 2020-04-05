//
// Created by Can Yang on 2020/3/8.
//

#ifndef FMM_INCLUDE_FMM_FMM_RESULT_HPP_
#define FMM_INCLUDE_FMM_FMM_RESULT_HPP_

#include "network/type.hpp"

namespace FMM {

/**
 * Class related with map matching
 */
namespace MM {

/**
 * A candidate matched to a point
 */
struct MatchedCandidate {
  Candidate c; /**< Candidate matched to a point */
  double ep;  /**< emission probability */
  double tp;  /**< transition probability to previous matched candidate */
  double sp_dist; /**< shortest path distance to previous matched candidate */
};

typedef std::vector<MatchedCandidate> MatchedCandidatePath;

/**
 * Map matched result representation
 */
struct MatchResult {
  int id; /**< */
  MatchedCandidatePath opt_candidate_path; /**< A vector of candidate matched
  to each point of a trajectory. It is stored in order to export more
  detailed map matching information. */
  O_Path opath; /**< the optimal path, containing id of edges matched to each
                     point in a trajectory */
  C_Path cpath; /**< the complete path, containing ids of a sequence of
                     topologically connected edges traversed by the
                     trajectory.  */
  std::vector<int> indices; /**< index of opath edge in cpath */
  LineString mgeom; /**< the geometry of the matched path */
};

};

};

#endif //FMM_INCLUDE_FMM_FMM_RESULT_HPP_
