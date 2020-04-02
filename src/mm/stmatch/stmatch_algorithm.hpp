#ifndef MM_STMATCH_ALGORITHM_HPP
#define MM_STMATCH_ALGORITHM_HPP

#include "network/network.hpp"
#include "network/network_graph.hpp"
#include "mm/composite_graph.hpp"
#include "mm/transition_graph.hpp"

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "cxxopts/cxxopts.hpp"

namespace MM {

struct STMATCHAlgorConfig {
  int k;
  double radius;
  double gps_error;
  // maximum speed of the vehicle
  double vmax;
  // factor multiplied to vmax*deltaT to limit the search of shortest path
  double factor;
  bool validate() const;
  void print() const;
  static STMATCHAlgorConfig load_from_xml(
      const boost::property_tree::ptree &xml_data);
  static STMATCHAlgorConfig load_from_arg(
      const cxxopts::ParseResult &arg_data);
};

class STMATCH {
 public:
  STMATCH(const Network &network, const NetworkGraph &graph) :
      network_(network), graph_(graph) {

  };
  std::string match_wkt(const std::string &wkt,
      const STMATCHAlgorConfig &config);
  // Procedure of HMM based map matching algorithm.
  MatchResult match_traj(const Trajectory &traj,
                         const STMATCHAlgorConfig &config);
 protected:
  void update_tg(TransitionGraph *tg,
                 const CompositeGraph &cg,
                 const Trajectory &traj,
                 const STMATCHAlgorConfig &config);

  void update_layer(int level, TGLayer *la_ptr, TGLayer *lb_ptr,
                    const CompositeGraph &cg,
                    double eu_dist,
                    double delta);

  /**
   * Return distances from source to all targets and with an upper bound of
   * delta to stop the search
   * @param  source  source node index
   * @param  targets a vector of N target node indices
   * @param  delta   upperbound of search
   * @return         a vector of N indices
   */
  std::vector<double> shortest_path_upperbound(
      int level,
      const CompositeGraph &cg, NodeIndex source,
      const std::vector<NodeIndex> &targets, double delta);
  /**
   * Create a complete path from the optimal element path in the transition
   * graph
   * @param tg_opath a vector of TGElement* in transition graph
   * @param indices the indices to be updated to store the index of matched
   * edge in the cpath.
   * @return a complete path traversed by the trajectory
   */
  C_Path build_cpath(const TGOpath &tg_opath, std::vector<int> *indices);
 private:
  const Network &network_;
  const NetworkGraph &graph_;
};// STMATCH

} // MM

#endif
