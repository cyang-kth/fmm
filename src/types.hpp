/**
 * Content
 * Definition of Data types used in the FMM algorithm
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */

#ifndef MM_TYPES_HPP
#define MM_TYPES_HPP

#include <vector>
#include <unordered_map>
#include "geometry_type.hpp"

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
  LineString geom;   // edge geometry
};

struct Candidate
{
  double offset;   // offset distance from the start of polyline to p'
  double dist;   // distance from original point p to map matched point p'
  double obs_prob;   // this is the emission probability
  Edge *edge;   // candidate edge
  Candidate* prev;   // optimal previous candidate used in Viterbi algorithm
  double cumu_prob;   // used in Viterbi, initialized to be 0
  double sp_dist;   // sp distance to previous point, initialized to be 0
};

// Record type in UBODT
// Every column stores index rather than ID.
// For verification of the result, run ubodt to generate the ID map.
struct Record
{
  NodeIndex source;
  NodeIndex target;
  NodeIndex first_n;   // next_n in the paper
  NodeIndex prev_n;
  EdgeIndex next_e;
  double cost;
  Record *next;   // the next Record used in Hashtable
};

/* Transitiong graph*/
// candidates of a point
typedef std::vector<Candidate> Point_Candidates;
// candidates of a trajectory
typedef std::vector<Point_Candidates> Traj_Candidates;

/* Result of map matching  */

// Optimal path containing candidates matched to each point in a trajectory
typedef std::vector<Candidate*> O_Path;

// Complete path, a sequence of spatially contiguous edges traversed
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
