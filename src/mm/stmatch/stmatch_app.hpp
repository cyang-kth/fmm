#ifndef MM_SRC_MM_STMATCH_STMATCH_APP_H_
#define MM_SRC_MM_STMATCH_STMATCH_APP_H_

#include "mm/stmatch/stmatch_app_config.hpp"
#include "mm/stmatch/stmatch_algorithm.hpp"
#include "io/gps_reader.hpp"
#include "io/mm_writer.hpp"

namespace MM {
class STMATCHApp {
public:
  STMATCHApp(const STMATCHAppConfig &config);
  void run();
 private:
  const STMATCHAppConfig &config_;
  Network network_;
  NetworkGraph ng_;
};
}

#endif
