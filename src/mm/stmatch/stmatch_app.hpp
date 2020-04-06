/**
 * Fast map matching.
 *
 * Stmatch command line program.
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */

#ifndef FMM_STMATCH_APP_H_
#define FMM_STMATCH_APP_H_

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
  /**
   * Create stmatch command application from configuration
   * @param config configuration of the command line app
   */
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
