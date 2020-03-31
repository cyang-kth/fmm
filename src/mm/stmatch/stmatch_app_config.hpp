//
// Created by Can Yang on 2020/4/1.
//

#ifndef FMM_SRC_MM_STMATCH_STMATCH_APP_CONFIG_HPP_
#define FMM_SRC_MM_STMATCH_STMATCH_APP_CONFIG_HPP_

#include "config/gps_config.hpp"
#include "config/network_config.hpp"
#include "config/result_config.hpp"

namespace MM{

/**
 * Configuration class for map matching
 */
class STMATCHAppConfig
{
 public:
  STMATCHAppConfig(int argc, char **argv);
  void load_xml(const std::string &file);
  void load_arg(int argc, char **argv);
  static void print_help();
  void print() const;
  bool validate() const;
  NetworkConfig network_config;
  GPSConfig gps_config;
  ResultConfig result_config;
  STMATCHConfig stmatch_config;
  // 0-trace,1-debug,2-info,3-warn,4-err,5-critical,6-off
  int log_level;
}; // STMATCHAppConfig

}

#endif //FMM_SRC_MM_STMATCH_STMATCH_APP_CONFIG_HPP_
