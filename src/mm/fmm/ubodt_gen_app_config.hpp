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
  int log_level;
}; // UBODT_Config

}

#endif