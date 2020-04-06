/**
 * Fast map matching.
 *
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

/**
 * Vector of node id
 */
typedef std::vector<NodeID> NodeIDVec;
/**
 * Map of node index
 */
typedef std::unordered_map<NodeID,NodeIndex> NodeIndexMap;
/**
 * Map of edge index
 */
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

} // NETWORK
} // MM
#endif /* MM_TYPES_HPP */
