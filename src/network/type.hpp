/**
 * Content
 * Definition of Data types used in the FMM algorithm
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef MM_TYPES_HPP
#define MM_TYPES_HPP

#include <vector>
#include <list>
#include <string>
#include <unordered_map>

#include "core/geometry.hpp"

namespace MM {

typedef int NodeID;
typedef int EdgeID;
typedef unsigned int NodeIndex;
typedef unsigned int EdgeIndex;

typedef std::vector<NodeID> NodeIDVec;
typedef std::unordered_map<NodeID,NodeIndex> NodeIndexMap;
typedef std::unordered_map<EdgeID,EdgeIndex> EdgeIndexMap;

struct Edge
{
  // This is the index of an edge, which is continuous [0,N-1]
  EdgeIndex index;
  // Edge ID, can be discontinuous integers
  EdgeID id;
  NodeIndex source;   // source node index
  NodeIndex target;   // target node index
  double length;   // length of the edge polyline
  LineString geom;   // the edge geometry
};

struct Candidate
{
  NodeIndex index; // The index is defined for a specific candidate
  double offset;   // offset distance from the start of polyline to p'
  double dist;   // distance from original point p to map matched point p'
  Edge *edge;   // candidate edge
  Point point;   // boost point
};

// candidates of a point
typedef std::vector<Candidate> Point_Candidates;
// candidates of a trajectory
typedef std::vector<Point_Candidates> Traj_Candidates;

typedef std::vector<const Candidate*> OptCandidatePath;

/* Result of map matching  */

// Optimal path containing candidates matched to each point in a trajectory
typedef std::vector<EdgeID> O_Path;
// typedef std::list<Candidate*> O_Path;

// Complete path, a contiguous sequence of edges traversed
typedef std::vector<EdgeID> C_Path;

// Complete path, a sequence of spatially contiguous edges traversed,
// with edge index stored
typedef std::vector<EdgeIndex> C_PathIndex;

// The traversed path stores also the location of GPS point
// inside the C_Path, thus edges traversed between two GPS
// observations can be accessed.
struct T_Path {
  C_Path cpath;
  std::vector<int> indices;
};

}
#endif /* MM_TYPES_HPP */
