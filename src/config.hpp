/**
 * Content
 * Configuration Class defined for the two application
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */
#ifndef MM_CONFIG_HPP
#define MM_CONFIG_HPP
// Boost propertytree library
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <set>
#include <sys/stat.h> // file exist test
#include <exception>
#include <iomanip>

#include "debug.h"
#include "util.hpp"

namespace MM
{

/**
 *  The configuration defined for output of the program
 */
struct ResultConfig {
  // Original geometry
  bool write_ogeom = false;
  // Optimal path, the edge matched to each point
  bool write_opath = false;
  // The distance from the source node of an edge to the matched point
  bool write_offset = false;
  // The distance from a raw GPS point to a matched GPS point
  bool write_error = false;
  // Complete path, a path traversed by the trajectory
  bool write_cpath = true;
  // Traversed path, the path traversed between
  // each two consecutive observations
  bool write_tpath = false;
  // The geometry of the complete path
  bool write_mgeom = true;
  // The distance travelled between two GPS observations
  bool write_spdist = false;
  // A linestring connecting the point matched for each edge.
  bool write_pgeom = false;
  // The emission probability of each matched point.
  bool write_ep = false;
  // The transition probability of each matched point.
  bool write_tp = false;
};

static const std::vector<std::string>
LOG_LEVESLS {"0-trace","1-debug","2-info",
             "3-warn","4-err","5-critical","6-off"};

/**
 * Configuration class for map matching
 */
class FMM_Config
{
public:
  /**
   * FILETYPE 0 for ini and 1 for XML
   */
  FMM_Config(const std::string &file)
  {
    SPDLOG_INFO("Start reading FMM configuration");
    // Create empty property tree object
    boost::property_tree::ptree tree;
    boost::property_tree::read_xml(file, tree);
    // Parse the XML into the property tree.
    // Without default value, the throwing version of get to find attribute.
    // If the path cannot be resolved, an exception is thrown.

    // UBODT
    ubodt_file = tree.get<std::string>("fmm_config.input.ubodt.file");
    // Check if delta is specified or not
    if (!tree.get_optional<bool>("fmm_config.input.ubodt.delta")
        .is_initialized()) {
      delta_defined = false;
      delta = 0.0;
    } else {
      delta = tree.get("fmm_config.input.ubodt.delta",5000.0);       //
      delta_defined = true;
    }
    binary_flag = UTIL::get_file_extension(ubodt_file);

    // Network
    network_file = tree.get<std::string>("fmm_config.input.network.file");
    network_id = tree.get("fmm_config.input.network.id", "id");
    network_source = tree.get("fmm_config.input.network.source", "source");
    network_target = tree.get("fmm_config.input.network.target", "target");

    // GPS
    gps_file = tree.get<std::string>("fmm_config.input.gps.file");
    gps_id = tree.get("fmm_config.input.gps.id", "id");

    // Other parameters
    k = tree.get("fmm_config.parameters.k", 8);
    radius = tree.get("fmm_config.parameters.r", 300.0);

    // HMM
    gps_error = tree.get("fmm_config.parameters.gps_error", 50.0);
    penalty_factor = tree.get("fmm_config.parameters.pf", 0.0);

    // Output
    result_file = tree.get<std::string>("fmm_config.output.file");
    log_level = tree.get("fmm_config.other.log_level",2);

    if (tree.get_child_optional("fmm_config.output.fields")) {
      // Fields specified
      // close the default output fields (cpath,mgeom are true by default)
      result_config.write_cpath = false;
      result_config.write_mgeom = false;
      if (tree.get_child_optional("fmm_config.output.fields.ogeom")) {
        result_config.write_ogeom = true;
      }
      if (tree.get_child_optional("fmm_config.output.fields.opath")) {
        result_config.write_opath = true;
      }
      if (tree.get_child_optional("fmm_config.output.fields.cpath")) {
        result_config.write_cpath = true;
      }
      if (tree.get_child_optional("fmm_config.output.fields.tpath")) {
        result_config.write_tpath = true;
      }
      if (tree.get_child_optional("fmm_config.output.fields.mgeom")) {
        result_config.write_mgeom = true;
      }
      if (tree.get_child_optional("fmm_config.output.fields.pgeom")) {
        result_config.write_pgeom = true;
      }
      if (tree.get_child_optional("fmm_config.output.fields.offset")) {
        result_config.write_offset = true;
      }
      if (tree.get_child_optional("fmm_config.output.fields.error")) {
        result_config.write_error = true;
      }
      if (tree.get_child_optional("fmm_config.output.fields.spdist")) {
        result_config.write_spdist = true;
      }
      if (tree.get_child_optional("fmm_config.output.fields.ep")) {
        result_config.write_ep = true;
      }
      if (tree.get_child_optional("fmm_config.output.fields.tp")) {
        result_config.write_tp = true;
      }
      if (tree.get_child_optional("fmm_config.output.fields.all")) {
        result_config.write_ogeom= true;
        result_config.write_opath = true;
        result_config.write_pgeom = true;
        result_config.write_offset = true;
        result_config.write_error = true;
        result_config.write_spdist = true;
        result_config.write_cpath = true;
        result_config.write_mgeom = true;
        result_config.write_tpath = true;
        result_config.write_ep = true;
        result_config.write_tp = true;
      }
    } else {
      SPDLOG_INFO("Default output fields used.");
    }
    SPDLOG_INFO("Finish with reading FMM configuration");
  };

  ResultConfig get_result_config(){
    return result_config;
  };

  void print()
  {
    std::cout << "------------------------------------------\n";
    std::cout << "Configuration parameters for map matching application: \n";
    std::cout << "Network_file: " << network_file << '\n';;
    std::cout << "Network id: " << network_id << '\n';
    std::cout << "Network source: " << network_source << '\n';
    std::cout << "Network target: " << network_target << '\n';
    std::cout << "ubodt_file: " << ubodt_file << '\n';
    if (delta_defined) {
      std::cout << "delta: " << delta << '\n';
    } else {
      std::cout << "delta: " << "undefined, to be inferred from ubodt file\n";
    }
    std::cout << "ubodt format(1 binary, 0 csv): " << binary_flag << '\n';
    std::cout << "gps_file: " << gps_file << '\n';
    std::cout << "gps_id: " << gps_id << '\n';
    std::cout << "k: " << k << '\n';
    std::cout << "radius: " << radius << '\n';
    std::cout << "gps_error: " << gps_error << '\n';
    std::cout << "penalty_factor: " << penalty_factor << '\n';
    std::cout << "log_level:" << LOG_LEVESLS[log_level] << '\n';
    std::cout << "result_file:" << result_file << '\n';
    std::cout << "Output fields:"<<'\n';
    if (result_config.write_ogeom)
      std::cout << std::left << std::setw(8) << ""  << "ogeom"<<'\n';
    if (result_config.write_opath)
      std::cout << std::left << std::setw(8) << ""  << "opath"<<'\n';
    if (result_config.write_pgeom)
      std::cout << std::left << std::setw(8) << "" << "pgeom"<<'\n';
    if (result_config.write_offset)
      std::cout << std::left << std::setw(8) << "" << "offset"<<'\n';
    if (result_config.write_error)
      std::cout << std::left << std::setw(8) << "" << "error"<<'\n';
    if (result_config.write_spdist)
      std::cout << std::left << std::setw(8) << "" << "spdist"<<'\n';
    if (result_config.write_cpath)
      std::cout << std::left << std::setw(8) << "" << "cpath"<<'\n';
    if (result_config.write_tpath)
      std::cout << std::left << std::setw(8) << "" << "tpath"<<'\n';
    if (result_config.write_mgeom)
      std::cout << std::left << std::setw(8) << "" << "mgeom"<<'\n';
    if (result_config.write_ep)
      std::cout << std::left << std::setw(8) << "" << "ep"<<'\n';
    if (result_config.write_tp)
      std::cout << std::left << std::setw(8) << "" << "tp"<<'\n';

    std::cout << "------------------------------------------\n";
  };
  bool validate_mm()
  {
    std::cout << "Validating configuration for map match application:\n";
    if (!UTIL::fileExists(gps_file))
    {
      SPDLOG_CRITICAL("GPS file {} not found",gps_file);
      return false;
    };
    if (!UTIL::fileExists(network_file))
    {
      SPDLOG_CRITICAL("Network file {} not found",network_file);
      return false;
    };
    if (!UTIL::fileExists(ubodt_file))
    {
      SPDLOG_CRITICAL("UBODT file {} not found",ubodt_file);
      return false;
    };
    if (binary_flag==2) {
      SPDLOG_CRITICAL("UBODT file extension not recognized");
      return false;
    }
    if (log_level<0 || log_level>LOG_LEVESLS.size()) {
      SPDLOG_CRITICAL("Invalid log_level {}, which should be 0 - 6",log_level);
      SPDLOG_INFO("0-trace,1-debug,2-info,3-warn,4-err,5-critical,6-off");
      return false;
    }
    if (UTIL::fileExists(result_file))
    {
      SPDLOG_WARN("Overwrite existing result file {}",result_file);
    };
    std::string output_folder = UTIL::get_file_directory(result_file);
    if (!UTIL::folderExists(output_folder)) {
      SPDLOG_CRITICAL("Output folder {} not exists",output_folder);
      return false;
    }
    if (gps_error <= 0 || radius <= 0 || k <= 0)
    {
      SPDLOG_CRITICAL("Invalid mm parameter k {} r {} gps error {}",
                      k,radius,gps_error);
      return false;
    }
    // Check the definition of parameters search radius and gps error
    if (radius / gps_error > 10) {
      SPDLOG_CRITICAL("Too large radius {} compared with gps error {}",
                      radius,gps_error);
      return false;
    }
    std::cout << "Validating done.\n";
    return true;
  };

  /* Input files */
  // Network file
  std::string network_file;
  std::string network_id;
  std::string network_source;
  std::string network_target;

  // UBODT configurations
  std::string ubodt_file;
  double delta;
  bool delta_defined = true;
  int binary_flag;

  // GPS file
  std::string gps_file;
  std::string gps_id;
  // Result file
  std::string result_file;
  // Parameters
  double gps_error;
  // Used by hashtable in UBODT

  // Used by Rtree search
  int k;
  double radius;

  // PF for reversed movement
  double penalty_factor;

  // Configuration of output format
  ResultConfig result_config;
  // 0-trace,1-debug,2-info,3-warn,4-err,5-critical,6-off
  int log_level;
}; // FMM_Config


/**
 * Configuration class for UBODT
 */
class UBODT_Config
{
public:
  /**
   * FILETYPE 0 for ini and 1 for XML
   */
  UBODT_Config(const std::string &file)
  {

    // Create empty property tree object
    boost::property_tree::ptree tree;
    std::cout << "Read configuration from xml file: " << file << '\n';
    boost::property_tree::read_xml(file, tree);
    // Parse the XML into the property tree.

    // UBODT configuration
    delta = tree.get("ubodt_config.parameters.delta", 5000.0);

    // Network
    network_file = tree.get<std::string>("ubodt_config.input.network.file");
    network_id = tree.get("ubodt_config.input.network.id", "id");
    network_source = tree.get("ubodt_config.input.network.source", "source");
    network_target = tree.get("ubodt_config.input.network.target", "target");

    // Output
    result_file = tree.get<std::string>("ubodt_config.output.file");
    binary_flag = UTIL::get_file_extension(result_file);

    // 0-trace,1-debug,2-info,3-warn,4-err,5-critical,6-off
    log_level = tree.get("ubodt_config.other.log_level",2);
  };
  void print()
  {
    std::cout << "------------------------------------------\n";
    std::cout << "Configuration parameters for UBODT construction: \n";
    std::cout << "Network_file: " << network_file << '\n';;
    std::cout << "Network id: " << network_id << '\n';
    std::cout << "Network source: " << network_source << '\n';
    std::cout << "Network target: " << network_target << '\n';
    std::cout << "delta: " << delta << '\n';
    std::cout << "Output file:" << result_file << '\n';
    std::cout << "Output format(1 binary, 0 csv): " << binary_flag << '\n';
    std::cout << "log_level:" << LOG_LEVESLS[log_level] << '\n';
    std::cout << "------------------------------------------\n";
  };
  bool validate()
  {
    std::cout << "Validating configuration for UBODT construction:\n";
    if (!UTIL::fileExists(network_file))
    {
      SPDLOG_CRITICAL("Network file {} not found",network_file);
      return false;
    }
    if (UTIL::fileExists(result_file))
    {
      SPDLOG_WARN("Overwrite result file {}",result_file);
    }
    std::string output_folder = UTIL::get_file_directory(result_file);
    if (!UTIL::folderExists(output_folder)) {
      SPDLOG_CRITICAL("Output folder {} not exists",output_folder);
      return false;
    }
    if (log_level<0 || log_level>LOG_LEVESLS.size()) {
      SPDLOG_CRITICAL("Invalid log_level {}, which should be 0 - 6",log_level);
      SPDLOG_INFO("0-trace,1-debug,2-info,3-warn,4-err,5-critical,6-off");
      return false;
    }
    if (binary_flag==2) {
      SPDLOG_CRITICAL("UBODT file extension not recognized");
      return false;
    }
    if (delta <= 0)
    {
      SPDLOG_CRITICAL("Delta {} should be positive");
      return false;
    }
    std::cout << "Validating done.\n";
    return true;
  };
  std::string network_file;
  std::string network_id;
  std::string network_source;
  std::string network_target;
  int binary_flag;
  double delta;
  std::string result_file;
  // 0-trace,1-debug,2-info,3-warn,4-err,5-critical,6-off
  int log_level;
}; // UBODT_Config

} // MM
#endif //MM_CONFIG_HPP
