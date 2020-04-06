/**
 * Fast map matching.
 * 
 * Definition of input trajectory format
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef FMM_GPS_HPP
#define FMM_GPS_HPP

#include "core/geometry.hpp"

#include <vector>

namespace FMM {

namespace CORE{

/**
 * %Trajectory class
 *
 * A GPS trajectory represented with id, geometry and timestamps
 */
struct Trajectory{
  int id; /**< Id of the trajectory */
  LineString geom; /**< Geometry of the trajectory */
  std::vector<double> timestamps; /**< Timestamps of the trajectory */
};

} // CORE

} // FMM
#endif /* FMM_GPS_HPP */
