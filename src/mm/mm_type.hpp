/**
 * Fast map matching.
 *
 * Definition of map matching types
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */


#ifndef FMM_INCLUDE_FMM_FMM_RESULT_HPP_
#define FMM_INCLUDE_FMM_FMM_RESULT_HPP_

#include "network/type.hpp"

namespace FMM {

/**
 * Class related with map matching
 */
namespace MM {

/**
 * %Candidate edge matched to a GPS point
 */
struct Candidate
{
  FMM::NETWORK::NodeIndex index; /**< The index is defined for a specific
                        candidate the index starting from N where N is the
                        numeber of vertices in the graph */
  double offset; /**< offset distance from the start of polyline to p' */
  double dist; /**< distance from original point p to map matched point p' */
  NETWORK::Edge *edge;  /**< candidate edge */
  FMM::CORE::Point point; /**< boost point */
};

typedef std::vector<Candidate> Point_Candidates; /**< Point candidates */
typedef std::vector<Point_Candidates> Traj_Candidates;
/**< trajectory  candidates */

typedef std::vector<const Candidate*> OptCandidatePath;
/**< Optimal candidates*/

typedef std::vector<FMM::NETWORK::EdgeID> O_Path; /**< Optimal path, edge id matched to
each point in the trajectory */

typedef std::vector<FMM::NETWORK::EdgeID> C_Path; /**< Complete path, ids of
a sequence of topologically connected edges.*/

/**
 * A candidate matched to a point, the fields are stored to facilitate the
 * output of detailed MM information.
 */
struct MatchedCandidate {
  int index; /**< Index of point index in trajectory */
  EdgeID edge_id; /**< Edge id matched */
  NodeID source; /**< Edge source node ID */
  NodeID target; /**< Edge target node ID */
  double error; /**< Error of matching */
  double offset; /**< Matched point distance to start node of edge */
  double length; /**< Length of edge matched */
  double ep; /**< emission probability */
  double tp; /**< transition proability from previous matched candidate */
  double spdist; /**< shortest path distance from previous matched candidate */
  double px; /**< Projected point x */
  double py; /**< Projected point y */
};

/**
 * A vector of candidates, used for representing the matching result of
 * a trajectory.
 */
typedef std::vector<MatchedCandidate> MatchedCandidatePath;

/**
 * Map matched result representation
 */
struct MatchResult{
  int id; /**< id of the trajectory to be matched */
  MatchedCandidatePath opt_candidate_path; /**< A vector of candidate matched
  to each point of a trajectory. It is stored in order to export more
  detailed map matching information. */
  O_Path opath; /**< the optimal path,
                              containing id of edges matched to each
                              point in a trajectory */
  C_Path cpath; /**< the complete path, containing ids of a sequence of
                     topologically connected edges traversed by the
                     trajectory.  */
  std::vector<int> indices; /**< index of opath edge in cpath */
  CORE::LineString mgeom; /**< the geometry of the matched path */
};

struct PartialMatchResult {
  int id; /**< id of the trajectory to be matched */
  MatchedCandidatePath opt_candidate_path; /**< A vector of candidate matched
  to each point of a trajectory. It is stored in order to export more
  detailed map matching information. */
  O_Path opath; /**< the optimal path,
                              containing id of edges matched to each
                              point in a trajectory */
  C_Path cpath; /**< the complete path, containing ids of a sequence of
                     topologically connected edges traversed by the
                     trajectory.  */
  std::vector<int> indices; /**< index of opath edge in cpath */
  CORE::MultiLineString mgeom; /**< the geometry of the matched path */
};

};

};

#endif //FMM_INCLUDE_FMM_FMM_RESULT_HPP_
