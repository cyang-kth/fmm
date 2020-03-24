//
// Created by Can Yang on 2020/3/8.
//

#ifndef MM_INCLUDE_MM_MM_TYPE_HPP_
#define MM_INCLUDE_MM_MM_TYPE_HPP_

#include "network/type.hpp"

namespace MM{
struct MatchResult {
  int id;
  O_Path opath;
  C_Path cpath;
  LineString mgeom;
};

struct OutputConfig {
  // Optimal path, the edge matched to each point
  bool write_opath = false;
  // Complete path, a path traversed by the trajectory
  bool write_cpath = true;
  // The geometry of the complete path
  bool write_mgeom = true;
};

// The unit here is the same as the GPS data
struct MMConfig {
  int k; // Number of candidates
  double radius; // Search radius
  double gps_error; // gps error
  double weight; // weight on gps error
  double vmax; // maximum speed of the vehicle
  double factor; // factor multiplied to vmax * deltaT
  inline friend std::ostream &operator<<(std::ostream &os,
      const MMConfig &config){
    os << "\n";
    os << "k: " << config.k << "\n"
       << "r: " << config.radius << "\n"
       << "gps_error: " << config.gps_error << "\n"
       << "weight: " << config.weight << "\n"
       << "factor: " << config.factor << "\n";
    return os;
  };
};

}

#endif //MM_INCLUDE_MM_MM_TYPE_HPP_
