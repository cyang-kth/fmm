#ifndef MM_FMM_UBODT_CONFIG
#define MM_FMM_UBODT_CONFIG

#include "config/network_config.hpp"

namespace MM{

class UBODTGenAppConfig
{
 public:
  UBODTGenAppConfig(int argc, char **argv);
  void load_xml(const std::string &file);
  void load_arg(int argc, char **argv);
  void print() const;
  bool validate() const;
  bool is_binary_output() const;
  static void print_help();
  NetworkConfig network_config;
  double delta;
  std::string result_file;
  // 0-trace,1-debug,2-info,3-warn,4-err,5-critical,6-off
  int log_level = 2;
  bool use_omp = false;
  bool help_specified;
}; // UBODT_Config

}

#endif