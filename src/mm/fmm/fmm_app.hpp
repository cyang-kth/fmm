#ifndef MM_SRC_MM_FMM_FMM_APP_H_
#define MM_SRC_MM_FMM_FMM_APP_H_

#include "fmm_app_config.hpp"
#include "fmm_algorithm.hpp"

namespace MM {
class FMMApp {
 public:
  FMMApp(const FMMAppConfig &config) :
      config_(config),
      network_(config_.network_config.file,
               config_.network_config.id,
               config_.network_config.source,
               config_.network_config.target),
      ng_(network_),
      ubodt_(UBODT::read_ubodt_file(config_.ubodt_file)){};
  void run();
 private:
  const FMMAppConfig &config_;
  Network network_;
  NetworkGraph ng_;
  std::shared_ptr<UBODT> ubodt_;
};
}

#endif
