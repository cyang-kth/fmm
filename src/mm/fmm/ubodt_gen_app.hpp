//
// Created by Can Yang on 2020/4/1.
//

#ifndef FMM_SRC_MM_FMM_UBODT_GEN_APP_HPP_
#define FMM_SRC_MM_FMM_UBODT_GEN_APP_HPP_

#include <boost/archive/binary_oarchive.hpp>
#include "mm/fmm/ubodt_gen_app_config.hpp"
#include "network/network.hpp"
#include "network/network_graph.hpp"

namespace MM {
class UBODTGenApp {
 public:
  UBODTGenApp(const UBODTGenAppConfig &config) :
      config_(config),
      network_(config_.network_config.file,
               config_.network_config.id,
               config_.network_config.source,
               config_.network_config.target),
      graph_(network_) {};
  void run() const;
  void precompute_ubodt(
      const std::string &filename, double delta, bool binary = true) const;
  void precompute_ubodt_omp(const std::string &filename, double delta,
                            bool binary = true) const;

 private:
  const UBODTGenAppConfig &config_;
  Network network_;
  NetworkGraph graph_;
  void write_result_binary(boost::archive::binary_oarchive &stream,
                           NodeIndex s,
                           PredecessorMap &pmap,
                           DistanceMap &dmap) const;
  void write_result_csv(std::ostream &stream,
                        NodeIndex s,
                        PredecessorMap &pmap, DistanceMap &dmap) const;
};
}

#endif //FMM_SRC_MM_FMM_UBODT_GEN_APP_HPP_
