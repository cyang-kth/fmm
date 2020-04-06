//
// Created by Can Yang on 2020/3/22.
//

#ifndef MM_SRC_MM_FMM_FMM_ALGORITHM_H_
#define MM_SRC_MM_FMM_FMM_ALGORITHM_H_

#include "network/network.hpp"
#include "network/network_graph.hpp"
#include "mm/transition_graph.hpp"
#include "mm/fmm/ubodt.hpp"
#include "python/pyfmm.hpp"

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "cxxopts/cxxopts.hpp"

namespace FMM {
namespace MM{
/**
 * Configuration class for fmm algorithm
 */
struct FastMapMatchConfig{
  FastMapMatchConfig(int k_arg = 8, double r_arg = 300, double gps_error = 50);
  int k;
  double radius;
  double gps_error;
  bool validate () const;
  void print() const;
  static FastMapMatchConfig load_from_xml(
      const boost::property_tree::ptree &xml_data);
  static FastMapMatchConfig load_from_arg(
      const cxxopts::ParseResult &arg_data);
};

/**
 * %FMM algorithm/model
 */
class FastMapMatch {
 public:
  FastMapMatch(const NETWORK::Network &network,
      const  NETWORK::NetworkGraph &graph,
      std::shared_ptr<UBODT> ubodt)
      : network_(network), graph_(graph), ubodt_(ubodt) {
  };
  /**
   * Match a trajectory to the road network
   * @param  traj   input trajector data
   * @param  config configuration of stmatch algorithm
   */
  MatchResult match_traj(const CORE::Trajectory &traj,
                         const FastMapMatchConfig &config);
  /**
   * Match a wkt linestring to the road network.
   */
  PYTHON::PyMatchResult match_wkt(
      const std::string &wkt,const FastMapMatchConfig &config);
 protected:
  /**
   * Get shortest path distance between two candidates
   * @param  ca
   * @param  cb
   * @return  shortest path value
   */
  double get_sp_dist(const Candidate *ca,
                     const Candidate *cb);
  /**
   * Update probabilities in a transition graph
   */
  void update_tg(TransitionGraph *tg,
                 const CORE::Trajectory &traj,
                 const FastMapMatchConfig &config);
  /**
   * Update probabilities between two layers a and b in the transition graph
   * @param level   the index of layer a
   * @param la_ptr  layer a
   * @param lb_ptr  layer b next to a
   * @param eu_dist Euclidean distance between two observed point
   */
  void update_layer(int level, TGLayer *la_ptr, TGLayer *lb_ptr,
                    double eu_dist);
 private:
  const NETWORK::Network &network_;
  const NETWORK::NetworkGraph &graph_;
  std::shared_ptr<UBODT> ubodt_;
};
}
}

#endif //MM_SRC_MM_FMM_FMM_H_
