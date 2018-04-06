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
#include "gdal/ogrsf_frmts.h" // C++ API for GDAL
namespace MM {
struct Edge
{
    int id; // This is the id, which is continuous distributed 
    std::string id_attr; // This is the external ID attribute, which does not have to be continuous
    int source; // source node ID
    int target; // target node ID
    double length; // length of the edge polyline
    OGRLineString *geom; // a pointer to the edge geometry
};
double GPS_ERROR = 50;

struct Candidate
{
    float offset; // offset distance from the start of polyline to p'
    double dist; // distance from p to p'
    double obs_prob; // this is the emission probability
    Edge *edge; // candidate edge
    Candidate* prev; // optimal previous candidate used in Viterbi algorithm 
    float cumu_prob; // used in Viterbi, initially to be 0
};

/* Record type in UBODT */
struct record
{
    int source;
    int target;
    int first_n; // next_n in the paper
    int prev_n;
    int next_e;
    double cost;
    record *next; // the next record used in Hashtable
};

/* Transitiong graph*/

typedef std::vector<Candidate> Point_Candidates; // candidates of a point
typedef std::vector<Point_Candidates> Traj_Candidates; // candidates of a trajectory 

/* Result of map matching  */

// Optimal path containing candidates matched to each point in a trajectory 
typedef std::vector<Candidate*> O_Path; 

// Complete path, a contiguous sequence of edges traversed
typedef std::vector<int> C_Path; 

}
#endif /* MM_TYPES_HPP */
