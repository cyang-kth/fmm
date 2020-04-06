/**
 * Fast map matching.
 *
 * ubodg_gen command line program 
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */

#ifndef FMM_SRC_MM_FMM_UBODT_GEN_APP_HPP_
#define FMM_SRC_MM_FMM_UBODT_GEN_APP_HPP_

#include <boost/archive/binary_oarchive.hpp>
#include "mm/fmm/ubodt_gen_app_config.hpp"
#include "network/network.hpp"
#include "network/network_graph.hpp"

namespace FMM {
namespace MM{
/**
 * ubodt_gen command line program
 */
class UBODTGenApp {
 public:
  /**
   * Constructor
   * @param config Configuration data
   */
  UBODTGenApp(const UBODTGenAppConfig &config) :
      config_(config),
      network_(config_.network_config.file,
               config_.network_config.id,
               config_.network_config.source,
               config_.network_config.target),
      graph_(network_) {};
  /**
   * Run the precomputation
   */
  void run() const;
  /**
   * Run precomputation in a single thread and save result to a file
   * @param filename output file name
   * @param delta    upper bound value
   * @param binary   whether store binary data or not
   */
  void precompute_ubodt(
      const std::string &filename, double delta, bool binary = true) const;
  /**
   * Run precomputation parallelly and save result to a file
   * @param filename output file name
   * @param delta    upper bound value
   * @param binary   whether store binary data or not
   */
  void precompute_ubodt_omp(const std::string &filename, double delta,
                            bool binary = true) const;

 private:
  const UBODTGenAppConfig &config_;
  NETWORK::Network network_;
  NETWORK::NetworkGraph graph_;
  /**
   * Write the routing result to a binary stream
   * @param stream output binary stream
   * @param s      source node
   * @param pmap   predecessor map
   * @param dmap   distance map
   */
  void write_result_binary(boost::archive::binary_oarchive &stream,
                           NETWORK::NodeIndex s,
                           NETWORK::PredecessorMap &pmap,
                           NETWORK::DistanceMap &dmap) const;
  /**
   * Write the routing result to a csv stream
   * @param stream output csv stream
   * @param s      source node
   * @param pmap   predecessor map
   * @param dmap   distance map
   */
  void write_result_csv(std::ostream &stream,
                        NETWORK::NodeIndex s,
                        NETWORK::PredecessorMap &pmap,
                        NETWORK::DistanceMap &dmap) const;
};
}
}

#endif //FMM_SRC_MM_FMM_UBODT_GEN_APP_HPP_
