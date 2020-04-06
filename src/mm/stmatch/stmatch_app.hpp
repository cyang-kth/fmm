#ifndef FMM_SRC_FMM_STMATCH_STMATCH_APP_H_
#define FMM_SRC_FMM_STMATCH_STMATCH_APP_H_

#include "mm/stmatch/stmatch_app_config.hpp"
#include "mm/stmatch/stmatch_algorithm.hpp"
#include "io/gps_reader.hpp"
#include "io/mm_writer.hpp"

namespace FMM {
namespace MM{
/**
 * Class of stmatch command line program
 */
class STMATCHApp {
public:
  STMATCHApp(const STMATCHAppConfig &config);
  /**
   * Run the stmatch program
   */
  void run();
 private:
  const STMATCHAppConfig &config_;
  NETWORK::Network network_;
  NETWORK::NetworkGraph ng_;
};
}
}

#endif
