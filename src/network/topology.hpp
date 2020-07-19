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

typedef std::vector<FMM::CORE::LineString> EdgeGeomData;

typedef std::unordered_map<EdgeID,EdgeIntersectionData> TopologyData;

class TopologyGeneratorConfig
{
 public:
  /**
   * Constructor of the configuration from command line arguments.
   * The argument data are fetched from the main function directly.
   *
   * @param argc number of arguments
   * @param argv raw argument data
   *
   */
  TopologyGeneratorConfig(int argc, char **argv);
  /**
   * Validate the configuration
   * @return true if valid
   */
  inline bool validate() const;
  /**
   * Print configuration data
   */
  inline void print() const;
  /**
   * Print help information
   */
  static inline void print_help();
  std::string output_folder; /**< UBODT file name */
  CONFIG::NetworkConfig network_config;/**< Network data configuraiton */
  double tolerance;
  bool split_intersection = false;
  bool help_specified = false;  /**< Help is specified or not */
  int log_level = 2;  /**< log level, 0-trace,1-debug,2-info,
                          3-warn,4-err,5-critical,6-off */
  int step = 100; /**< progress report step */
}; // TopologyGeneratorConfig

class TopologyGenerator{
public:
  TopologyGenerator(const TopologyGeneratorConfig &config);
  inline int run(){
    EdgeGeomData geom_data = read_geom_data(
      config_.network_config.network_file);
    auto topo_data = create_topology(
      geom_data, config_.tolerance, config_.split_intersection);
    auto edges = create_edges(geom_data,topo_data);
    int status = export_result(edges,config_.output_folder);
  };

  static int export_result(const std::vector<Edge> &edges,
    const std::string &output_folder){

  };

  static EdgeGeomData read_geom_data(
    const std::string &network_file){

  };

  static TopologyData create_topology(
    const EdgeGeomData &data, double tolerance,
    bool split_intersection = false) {

  };

  static std::vector<Edge> create_edges(){
    TopologyData &topo_data, EdgeGeomData &geom_data
  };

private:
  const FMMAppConfig &config_;
};



}; // NETWORK

}; // FMM

#endif
