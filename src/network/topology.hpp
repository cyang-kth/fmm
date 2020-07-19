#ifndef FMM_TOPOLOGY_HPP
#define FMM_TOPOLOGY_HPP

#include "network/network.hpp"

namespace FMM {
/**
 * Classes related with network and graph
 */
namespace NETWORK {

struct IntersectionNode{
  NodeID id;
  FMM::CORE::Point point;
};

struct EdgeIntersectionData{
  NodeID id;
  double offset;
};

class TopologyGenerator{
public:
  static std::vector<FMM::CORE::LineString> read_geom_data(
    const std::string &network_file){

  };
  static std::vector<Edge> create_topology(
    const std::vector<FMM::CORE::LineString> &data, double tolerance,
    bool split_intersection = false) {

  };
};

}; // NETWORK

}; // FMM

#endif
