/**
 * Content
 * Configuration Class for FMM
 *
 * @author: Can Yang
 * @version: 2019.03.27
 */
#ifndef MM_STMATCH_CONFIG_HPP
#define MM_STMATCH_CONFIG_HPP

namespace MM
{

struct STMATCHConfig{
  int k;
  double radius;
  double gps_error;
  // maximum speed of the vehicle
  double vmax;
  // factor multiplied to vmax*deltaT to limit the search of shortest path
  double factor;
};

} // MM
#endif //MM_STMATCH_CONFIG_HPP
