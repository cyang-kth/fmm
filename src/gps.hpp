/**
 * Content
 * Definition of input trajectory format
 *      
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef MM_GPS_HPP
#define MM_GPS_HPP
#include "geometry_type.hpp"
#include <vector>
#include <utility>
namespace MM {

/**
 *   A trajectory stores two fields:
 *       id: int
 *       geom: a pointer to OGRLineString
 *       
 *   It is constructed with ID and a pointer to an OGRLineString object.
 *   When it is destructed, the geom pointer will be freed.
 */
class Trajectory{
public:
    Trajectory():id(0),geom(NULL){}; // Default constructor for vector of trajectories
    Trajectory(int e_id,LineString *e_geom):id(e_id),geom(e_geom){};
    ~Trajectory(){
#ifdef USE_BG_GEOMETRY
        delete geom;
#else
        if (geom!=NULL) OGRGeometryFactory::destroyGeometry(geom);
#endif           
    };
    int id; // Trip id
    LineString *geom; // It will be freed by the destructor of the trajectory
}; // Trajectory
} // MM
#endif /* MM_GPS_HPP */
