/**
 * Content
 * Definition of input trajectory format
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef MM_GPS_HPP
#define MM_GPS_HPP

#include "core/geometry.hpp"

#include <vector>

namespace MM {

struct Trajectory{
  int id;
  LineString geom;
  std::vector<double> timestamps;
};


} // MM
#endif /* MM_GPS_HPP */
