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
  double radius;
  double gps_error;
  std::string to_string() const;
  static FMMConfig load_from_xml(
    const boost::property_tree::ptree &xml_data);
  static FMMConfig load_from_arg(
    const cxxopts::ParseResult &arg_data);
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
  bool validate() const;
  int get_gps_format() const;
  void print() const;
  static void print_help();
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
