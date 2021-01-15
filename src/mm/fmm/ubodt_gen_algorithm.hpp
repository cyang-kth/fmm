/**
 * Fast map matching.
 *
 * ubodg_gen command line program
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */

#ifndef FMM_SRC_MM_FMM_UBODT_GEN_ALGORITHM_HPP_
#define FMM_SRC_MM_FMM_UBODT_GEN_ALGORITHM_HPP_

#include "mm/fmm/ubodt_gen_app_config.hpp"
#include "network/network.hpp"
#include "network/network_graph.hpp"

#ifdef BOOST_OS_WINDOWS
#include <boost/throw_exception.hpp>
#endif
#include <boost/archive/binary_oarchive.hpp>

namespace FMM {
namespace MM {
class UBODTGenAlgorithm {
public:
  UBODTGenAlgorithm(const NETWORK::Network &network,
                    const NETWORK::NetworkGraph &graph) :
    network_(network), ng_(graph){
  };
  std::string generate_ubodt(const std::string &filename, double delta,
                             bool binary = true, bool use_omp = true) const;
  /**
   * Run precomputation in a single thread and save result to a file
   * @param filename output file name
   * @param delta    upper bound value
   * @param binary   whether store binary data or not
   */
  void precompute_ubodt_single_thead(
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
  const NETWORK::Network &network_;
  const NETWORK::NetworkGraph &ng_;
}; // UBODTGenAlgorithm
}; // MM
}; // FMM

#endif //FMM_SRC_MM_FMM_UBODT_GEN_ALGORITHM_HPP_
