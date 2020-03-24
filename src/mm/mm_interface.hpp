#ifndef MM_INTERFACE_HPP
#define MM_INTERFACE_HPP

#include "mm/mm_type.hpp"
#include "algorithm/geom_algorithm.hpp"
#include "network/type.hpp"
#include "network/network.hpp"
#include "network/network_graph.hpp"
#include "util/debug.hpp"

namespace MM {

class MMInterface {
 public:
  MMInterface(const Network &network_arg, const NetworkGraph &ng_arg)
      : network(network_arg), ng(ng_arg) {
  };
  // Common interface for MM algorithm
  virtual MatchResult match_temporal_traj(const TemporalTrajectory &traj,
                                          const MMConfig &config) = 0;
  inline MatchResult match_traj(const Trajectory &traj,
                                const MMConfig &config) {
    // Add pseudo timestamps by estimation
    std::vector<double> timestamps = create_fake_timestamps(traj.geom,config);
    TemporalTrajectory temporal_trajectory{traj.id, traj.geom, timestamps};
    return match_temporal_traj(temporal_trajectory, config);
  }
  static inline std::vector<double> create_fake_timestamps(
      const LineString &geom,const MMConfig &config){
    std::vector<double> timestamps;
    int Npoints = geom.get_num_points();
    double cur_time = 0;
    timestamps.push_back(cur_time);
    std::vector<double> lengths = ALGORITHM::cal_eu_dist(geom);
    for (int i = 0; i < lengths.size(); ++i) {
      // Set the duration as half vmax
      cur_time+=lengths[i]/config.vmax/2;
      timestamps.push_back(cur_time);
    }
    return timestamps;
  };
protected:
  const Network &network;
  const NetworkGraph &ng;
};

} // MM

#endif
