/**
 * Content
 * Configuration Class for FMM
 *
 * @author: Can Yang
 * @version: 2019.03.27
 */
#ifndef MM_STMATCH_CONFIG_HPP
#define MM_STMATCH_CONFIG_HPP

#include "config/gps_config.hpp"
#include "config/network_config.hpp"
#include "config/result_config.hpp"

namespace MM
{

struct STMATCHConfig {
  int k;
  double radius;
  double gps_error;
  // maximum speed of the vehicle
  double vmax;
  // factor multiplied to vmax*deltaT to limit the search of shortest path
  double factor;
  std::string to_string() const;
  static STMATCHConfig load_from_xml(
    const boost::property_tree::ptree &xml_data);
  static STMATCHConfig load_from_arg(
    const cxxopts::ParseResult &arg_data);
};

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
  void print() const ;
  bool validate() const ;
  int get_gps_format() const ;
  NetworkConfig network_config;
  GPSConfig gps_config;
  ResultConfig result_config;
  STMATCHConfig stmatch_config;
  // 0-trace,1-debug,2-info,3-warn,4-err,5-critical,6-off
  int log_level;
}; // STMATCHAppConfig

} // MM
#endif //MM_STMATCH_CONFIG_HPP
