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
#include <string>
#include "geometry_type.hpp"

namespace MM {

typedef int NodeID;
typedef int EdgeID;
typedef unsigned int NodeIndex;
typedef unsigned int EdgeIndex;

struct Edge
{
  // This is the index of an edge, which is continuous [0,N-1]
  EdgeIndex index;
  // Edge ID, can be discontinuous integers
  EdgeID id;
  NodeIndex source;   // source node ID
  NodeIndex target;   // target node ID
  double length;   // length of the edge polyline
  LineString *geom;   // a pointer to the edge geometry
};

struct Candidate
{
  float offset;   // offset distance from the start of polyline to p'
  double dist;   // distance from original point p to map matched point p'
  double obs_prob;   // this is the emission probability
  Edge *edge;   // candidate edge
  Candidate* prev;   // optimal previous candidate used in Viterbi algorithm
  float cumu_prob;   // used in Viterbi, initialized to be 0
  float sp_dist;   // sp distance to previous point, initialized to be 0
};

/* Record type in UBODT */
struct Record
{
  int source;
  int target;
  int first_n;   // next_n in the paper
  int prev_n;
  int next_e;
  double cost;
  Record *next;   // the next Record used in Hashtable
};

/* Transitiong graph*/

typedef std::vector<Candidate> Point_Candidates; // candidates of a point
typedef std::vector<Point_Candidates> Traj_Candidates; // candidates of a trajectory

/* Result of map matching  */

// Optimal path containing candidates matched to each point in a trajectory
typedef std::vector<Candidate*> O_Path;

// Complete path, a contiguous sequence of edges traversed
typedef std::vector<int> C_Path;

// The traversed path stores also the location of GPS point inside the C_Path, thus
// edges traversed between two GPS observations can be found.
struct T_Path {
  C_Path cpath;
  std::vector<int> indices;
};

}
#endif /* MM_TYPES_HPP */
