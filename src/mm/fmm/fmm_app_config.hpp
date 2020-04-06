/**
 * Fast map matching.
 *
 * fmm command line program configuration
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */

#ifndef FMM_FMM_APP_CONFIG_HPP_
#define FMM_FMM_APP_CONFIG_HPP_

#include "config/gps_config.hpp"
#include "config/network_config.hpp"
#include "config/result_config.hpp"
#include "mm/fmm/fmm_algorithm.hpp"

namespace FMM{
namespace MM{
/**
 * Configuration class of fmm command line program
 */
class FMMAppConfig
{
 public:
  /**
   * Constructor of the configuration from command line arguments.
   * The argument data are fetched from the main function directly.
   *
   * @param argc number of arguments
   * @param argv raw argument data
   *
   */
  FMMAppConfig(int argc, char **argv);
  /**
   * Load configuration from an XML file
   * @param file xml file name
   */
  void load_xml(const std::string &file);
  /**
   * Load configuration from arguments. The argument data
   * are fetched from the main function directly.
   * @param argc number of arguments
   * @param argv raw argument data
   */
  void load_arg(int argc, char **argv);
  /**
   * Validate the configuration
   * @return true if valid
   */
  bool validate() const;
  /**
   * Print configuration data
   */
  void print() const;
  /**
   * Print help information
   */
  static void print_help();
  CONFIG::NetworkConfig network_config;/**< Network data configuraiton */
  CONFIG::GPSConfig gps_config; /**< GPS data configuraiton */
  CONFIG::ResultConfig result_config;  /**< Result configuraiton */
  FastMapMatchConfig fmm_config; /**< Map matching configuraiton */
  std::string ubodt_file; /**< UBODT file name */
  bool use_omp = false; /**< If true, parallel map matching performed */
  bool help_specified = false;  /**< Help is specified or not */
  int log_level = 2;  /**< log level, 0-trace,1-debug,2-info,
                          3-warn,4-err,5-critical,6-off */
  int step = 100; /**< progress report step */
}; // FMMAppConfig
}
}


#endif //FMM_SRC_MM_FMM_FMM_APP_CONFIG_HPP_
