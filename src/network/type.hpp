/**
 * Content
 * Definition of Data types used in the FMM algorithm
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef FMM_TYPES_HPP
#define FMM_TYPES_HPP

#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include "core/geometry.hpp"

namespace FMM {
namespace NETWORK{

typedef int NodeID; /**< Node ID in the network, can be discontinuous int */
typedef int EdgeID; /**< Edge ID in the network, can be discontinuous int */
typedef unsigned int NodeIndex; /**< Node Index in the network, range
                                 from [0,num_vertices-1 ]*/
typedef unsigned int EdgeIndex; /**< Edge Index in the network, range
                                 from [0,num_edges-1 ]*/

typedef std::vector<NodeID> NodeIDVec;
typedef std::unordered_map<NodeID,NodeIndex> NodeIndexMap;
typedef std::unordered_map<EdgeID,EdgeIndex> EdgeIndexMap;

/**
 * Road edge class
 */
struct Edge
{
  EdgeIndex index; /**< Index of an edge, which is continuous [0,N-1] */
  EdgeID id; /**< Edge ID, can be discontinuous integers */
  NodeIndex source; /**< source node index */
  NodeIndex target; /**< target node index */
  double length; /**< length of the edge polyline */
  FMM::CORE::LineString geom; /**< the edge geometry */
};

/**
 * Candidate edge matched to a GPS point
 */
struct Candidate
{
  NodeIndex index; /**< The index is defined for a specific candidate
                        the index starting from N where N is the numeber
                        of vertices in the graph */
  double offset; /**< offset distance from the start of polyline to p' */
  double dist; /**< distance from original point p to map matched point p' */
  Edge *edge;  /**< candidate edge */
  FMM::CORE::Point point; /**< boost point */
};

typedef std::vector<Candidate> Point_Candidates; /**< Point candidates */
typedef std::vector<Point_Candidates> Traj_Candidates;
/**< trajectory  candidates */

typedef std::vector<const Candidate*> OptCandidatePath;
/**< Optimal candidates*/

typedef std::vector<EdgeID> O_Path; /**< Optimal path, edge id matched to
each point in the trajectory */

typedef std::vector<EdgeID> C_Path; /**< Complete path, ids of
a sequence of topologically connected edges.*/

typedef std::vector<EdgeIndex> C_PathIndex;

} // NETWORK
} // MM
#endif /* MM_TYPES_HPP */
