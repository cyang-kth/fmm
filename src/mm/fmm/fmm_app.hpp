/**
 * Fast map matching.
 *
 * fmm command line program
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */


#ifndef FMM_FMM_APP_H_
#define FMM_FMM_APP_H_

#include "fmm_app_config.hpp"
#include "fmm_algorithm.hpp"

namespace FMM{
namespace MM{
/**
 * Class of fmm command line program
 */
class FMMApp {
 public:
  /**
   * Create FMMApp from configuration data
   * @param config Configuration of the FMMApp defining network, graph
   * and UBODT.
   */
  FMMApp(const FMMAppConfig &config) :
      config_(config),
      network_(config_.network_config.file,
               config_.network_config.id,
               config_.network_config.source,
               config_.network_config.target),
      ng_(network_),
      ubodt_(UBODT::read_ubodt_file(config_.ubodt_file)){};
  /**
   * Run the fmm program
   */
  void run();
 private:
  const FMMAppConfig &config_;
  NETWORK::Network network_;
  NETWORK::NetworkGraph ng_;
  std::shared_ptr<UBODT> ubodt_;
};
}
}

#endif
