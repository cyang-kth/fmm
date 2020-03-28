/**
 * Content
 * Configuration Class for FMM
 *
 * @author: Can Yang
 * @version: 2019.03.27
 */
#ifndef MM_FMM_CONFIG_HPP
#define MM_FMM_CONFIG_HPP

#include "config/gps_config.hpp"
#include "config/network_config.hpp"
#include "config/result_config.hpp"

namespace MM
{

struct FMMConfig{
  int k;
  double radius; //meter
  double gps_error; //meter
};

/**
 * Configuration class for map matching
 */
class FMMAppConfig
{
public:
  FMMAppConfig(int argc, char **argv);
  void load_xml(const std::string &file);
  void load_arg(int argc, char **argv);
  ResultConfig &get_result_config(){
    return result_config;
  };
  bool validate();
  static void print_help();
  void print();
  int get_gps_format();
  NetworkConfig network_config;
  GPSConfig gps_config;
  ResultConfig result_config;
  FMMConfig fmm_config;
  // UBODT configurations
  std::string ubodt_file;
  // 0-trace,1-debug,2-info,3-warn,4-err,5-critical,6-off
  int log_level;
}; // FMMAppConfig

} // MM
#endif //MM_FMM_CONFIG_HPP
