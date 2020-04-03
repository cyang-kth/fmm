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

namespace MM {

struct FMMAlgorConfig{
  int k;
  double radius;
  double gps_error;
  bool validate () const;
  void print() const;
  static FMMAlgorConfig load_from_xml(
      const boost::property_tree::ptree &xml_data);
  static FMMAlgorConfig load_from_arg(
      const cxxopts::ParseResult &arg_data);
};

class FMM {
 public:
  FMM(const Network &network, const NetworkGraph &graph,
      std::shared_ptr<UBODT> ubodt)
      : network_(network), graph_(graph), ubodt_(ubodt) {
  };
  // Procedure of HMM based map matching algorithm.
  MatchResult match_traj(const Trajectory &traj,
                         const FMMAlgorConfig &config);
  PyMatchResult match_wkt(
      const std::string &wkt,const FMMAlgorConfig &config);
 protected:
  double get_sp_dist(const Candidate *ca,
                     const Candidate *cb);
  // Update the transition graph
  void update_tg(TransitionGraph *tg,
                 const Trajectory &traj,
                 const FMMAlgorConfig &config);

  void update_layer(int level, TGLayer *la_ptr, TGLayer *lb_ptr,
                    double eu_dist);
 private:
  const Network &network_;
  const NetworkGraph &graph_;
  std::shared_ptr<UBODT> ubodt_;
};

}

#endif //MM_SRC_MM_FMM_FMM_H_
