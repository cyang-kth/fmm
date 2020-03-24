/**
 * Content
 * Configuration Class defined for the two application
 *
 * @author: Can Yang
 * @version: 2019.03.27
 */
#ifndef MM_APP_CONFIG_HPP
#define MM_APP_CONFIG_HPP

#include "mm/mm_interface.hpp"

#include <string>
#include <set>

namespace MM
{

/**
 * Configuration class for map matching
 */
class MMAppConfig
{
public:
  MMAppConfig(int argc, char **argv);

  MMConfig get_mm_config() const;
  MMConfig get_fmm_config() const;
  // Get output configuration
  OutputConfig get_result_config() const;
  // 0 for CSV and 1 for GeoPackage or shp
  int GetOutputFormat();
  // 0 for CSV and 1 for GeoPackage or shp
  int GetInputFormat();

  // Validate the configuration for mm_app
  bool validate() const;

  bool validate_network() const;

  bool validate_gps() const;

  bool validate_script() const;

  bool validate_ubodt() const;

  inline bool is_help_specified() const{
    return help_specified;
  };

  // Print the members of the config
  void print() const;

  // Print help information
  static void print_help();
  // Convert a list of string into a set
  static std::set<std::string> string2set(const std::string &s,
                                          char delim=',');

  static const std::vector<std::string> LOG_LEVESLS;

  static const std::vector<std::string> ALGORITHM_IDS;

  void initialize_xml(const std::string &file);
  void initialize_arg(int argc, char **argv);
private:
  // Network file
  std::string network_file;
  std::string network_id;
  std::string network_source;
  std::string network_target;

  // GPS file
  std::string gps_file;
  std::string gps_id;
  std::string gps_geom;
  std::string gps_timestamp;

  // GPS point form
  std::string gps_x;
  std::string gps_y;
  bool gps_point = false; // The GPS data stores point or not

  // Result file
  std::string result_file;

  // Parameters
  double gps_error; //meter
  double weight;
  double vmax; // m/s
  double factor;

  MM_ALGOR_TAG algorithm_id;
  // Used by hashtable in UBODT

  // Used by Rtree search
  int k;
  double radius; //meter

  // If the data (Both network and GPS) is projected (in unit of meters) or not
  // in unit of geodetic degrees.
  bool data_projected = false;

  // Other configurations
  // progress report step
  int step = 0;
  std::string script_file; // script file
  // 0-trace,1-debug,2-info,3-warn,4-err,5-critical,6-off
  int log_level = 5;
  std::string log_file; // Log file for mm_app
  std::string ubodt_file;
  std::string mm_log_file;
  int mm_log_level = 5;
  bool help_specified = false;
  // Whether export the nodes visited or not.
  bool verbose = false;
  // The limit on nodes visited per point
  int node_limit = 10000;
  OutputConfig result_config;
  friend class MMApp;
  friend class MMAppShell;
  friend class FMMAppShell;
}; // MMAppConfig

} // MM
#endif //MM_APP_CONFIG_HPP
