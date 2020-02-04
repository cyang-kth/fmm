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
#include <exception>
#include <iomanip>

// Argument parsing library
#include "cxxopts/cxxopts.hpp"

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
  // Traversed path geometry, which is a multilinestring
  bool write_tgeom = false;
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
  FMM_Config(int argc, char **argv){
    if (argc==2) {
      std::string configfile(argv[1]);
      initialize_xml(configfile);
    } else {
      initialize_arg(argc,argv);
    }
    std::cout<<"Set log level as "<<LOG_LEVESLS[log_level]<<"\n";
    spdlog::set_level((spdlog::level::level_enum) log_level);
    spdlog::set_pattern("[%l][%s:%-3#] %v");
  };

  void initialize_xml(const std::string &file)
  {
    std::cout<<"Start with reading FMM configuration "<<file<<"\n";
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
    gps_geom = tree.get("fmm_config.input.gps.geom", "geom");

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
      if (tree.get_child_optional("fmm_config.output.fields.tgeom")) {
        result_config.write_tgeom = true;
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
        result_config.write_tgeom = true;
      }
    } else {
      std::cout<<"Default output fields used.\n";
    }
    std::cout<<"Finish with reading FMM configuration.\n";
  };

  void initialize_arg(int argc, char **argv){
    std::cout<<"Start reading FMM configuration from arguments\n";
    cxxopts::Options options("fmm_config", "Configuration parser of fmm");
    options.add_options()
      ("u,ubodt","Ubodt file name", cxxopts::value<std::string>())
      ("a,network","Network file name", cxxopts::value<std::string>())
      ("b,network_id","Network id name",
      cxxopts::value<std::string>()->default_value("id"))
      ("c,source","Network source name",
      cxxopts::value<std::string>()->default_value("source"))
      ("d,target","Network target name",
      cxxopts::value<std::string>()->default_value("target"))
      ("g,gps","GPS file name", cxxopts::value<std::string>())
      ("f,gps_id","GPS file id",
      cxxopts::value<std::string>()->default_value("id"))
      ("n,gps_geom","GPS file geom column name",
      cxxopts::value<std::string>()->default_value("geom"))
      ("k,candidates","Number of candidates",
      cxxopts::value<int>()->default_value("8"))
      ("r,radius","Search radius",
      cxxopts::value<double>()->default_value("300.0"))
      ("e,error","GPS error",
      cxxopts::value<double>()->default_value("50.0"))
      ("p,pf","penalty_factor",
      cxxopts::value<double>()->default_value("0.0"))
      ("o,output","Output file name", cxxopts::value<std::string>())
      ("m,output_fields","Output fields", cxxopts::value<std::string>())
      ("l,log_level","Log level",cxxopts::value<int>()->default_value("2"));

    auto result = options.parse(argc, argv);
    ubodt_file = result["ubodt"].as<std::string>();
    binary_flag = UTIL::get_file_extension(ubodt_file);
    delta_defined = false;
    delta = 0.0;

    network_file = result["network"].as<std::string>();
    network_id = result["network_id"].as<std::string>();
    network_source = result["source"].as<std::string>();
    network_target = result["target"].as<std::string>();

    // GPS
    gps_file = result["gps"].as<std::string>();
    gps_id = result["gps_id"].as<std::string>();
    gps_geom = result["gps_geom"].as<std::string>();

    // Other parameters
    k = result["candidates"].as<int>();
    radius = result["radius"].as<double>();;

    log_level = result["log_level"].as<int>();

    // HMM
    gps_error = result["error"].as<double>();
    penalty_factor = result["pf"].as<double>();

    // Output
    result_file = result["output"].as<std::string>();

    if (result.count("output_fields")>0) {
      result_config.write_cpath = false;
      result_config.write_mgeom = false;
      std::string fields = result["output_fields"].as<std::string>();
      std::set<std::string> dict = string2set(fields);
      if (dict.find("opath")!=dict.end()) {
        result_config.write_opath = true;
      }
      if (dict.find("cpath")!=dict.end()) {
        result_config.write_cpath = true;
      }
      if (dict.find("mgeom")!=dict.end()) {
        result_config.write_mgeom = true;
      }
      if (dict.find("ogeom")!=dict.end()) {
        result_config.write_ogeom = true;
      }
      if (dict.find("tpath")!=dict.end()) {
        result_config.write_tpath = true;
      }
      if (dict.find("pgeom")!=dict.end()) {
        result_config.write_pgeom = true;
      }
      if (dict.find("offset")!=dict.end()) {
        result_config.write_offset = true;
      }
      if (dict.find("error")!=dict.end()) {
        result_config.write_error = true;
      }
      if (dict.find("spdist")!=dict.end()) {
        result_config.write_spdist = true;
      }
      if (dict.find("ep")!=dict.end()) {
        result_config.write_ep = true;
      }
      if (dict.find("tp")!=dict.end()) {
        result_config.write_tp = true;
      }
      if (dict.find("tgeom")!=dict.end()) {
        result_config.write_tgeom = true;
      }
      if (dict.find("all")!=dict.end()) {
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
        result_config.write_tgeom = true;
      }
    }
    std::cout<<"Finish with reading FMM configuration\n";
  };

  ResultConfig get_result_config(){
    return result_config;
  };

  static void print_help(){
    std::cout<<"fmm argument lists:\n";
    std::cout<<"--ubodt (required) <string>: Ubodt file name\n";
    std::cout<<"--network (required) <string>: Network file name\n";
    std::cout<<"--gps (required) <string>: GPS file name\n";
    std::cout<<"--output (required) <string>: Output file name\n";
    std::cout<<"--network_id (optional) <string>: Network id name (id)\n";
    std::cout<<"--source (optional) <string>: Network source name (source)\n";
    std::cout<<"--target (optional) <string>: Network target name (target)\n";
    std::cout<<"--gps_id (optional) <string>: GPS id name (id)\n";
    std::cout<<"--gps_geom (optional) <string>: GPS geometry name (geom)\n";
    std::cout<<"--candidates (optional) <int>: number of candidates (8)\n";
    std::cout<<"--radius (optional) <double>: search radius (300)\n";
    std::cout<<"--error (optional) <double>: GPS error (50)\n";
    std::cout<<"--pf (optional) <double>: penalty factor (0)\n";
    std::cout<<"--log_level (optional) <int>: log level (2)\n";
    std::cout<<"--output_fields (optional) <string>: Output fields\n";
    std::cout<<"  opath,cpath,tpath,ogeom,mgeom,pgeom,\n";
    std::cout<<"  offset,error,spdist,tp,ep,tgeom,all\n";
    std::cout<<"For xml configuration, check example folder\n";
  };

  void print()
  {
    std::cout << "------------------------------------------------\n";
    std::cout << "FMM Configurations\n";
    std::cout << "  Network_file: " << network_file << '\n';;
    std::cout << "  Network id: " << network_id << '\n';
    std::cout << "  Network source: " << network_source << '\n';
    std::cout << "  Network target: " << network_target << '\n';
    std::cout << "  ubodt_file: " << ubodt_file << '\n';
    if (delta_defined) {
      std::cout << "  delta: " << delta << '\n';
    } else {
      std::cout << "  delta: " << "undefined, to be inferred from ubodt file\n";
    }
    std::cout << "  ubodt format(1 binary, 0 csv): " << binary_flag << '\n';
    std::cout << "  gps_file: " << gps_file << '\n';
    std::cout << "  gps_id: " << gps_id << '\n';
    std::cout << "  k: " << k << '\n';
    std::cout << "  radius: " << radius << '\n';
    std::cout << "  gps_error: " << gps_error << '\n';
    std::cout << "  penalty_factor: " << penalty_factor << '\n';
    std::cout << "  log_level:" << LOG_LEVESLS[log_level] << '\n';
    std::cout << "  result_file:" << result_file << '\n';
    std::cout << "  Output fields:\n   ";
    if (result_config.write_ogeom)
      std::cout << " ogeom ";
    if (result_config.write_opath)
      std::cout << " opath ";
    if (result_config.write_pgeom)
      std::cout << " pgeom ";
    if (result_config.write_offset)
      std::cout << " offset ";
    if (result_config.write_error)
      std::cout << " error ";
    if (result_config.write_spdist)
      std::cout << " spdist ";
    if (result_config.write_cpath)
      std::cout << " cpath ";
    if (result_config.write_tpath)
      std::cout << " tpath ";
    if (result_config.write_mgeom)
      std::cout << " mgeom ";
    if (result_config.write_ep)
      std::cout << " ep ";
    if (result_config.write_tp)
      std::cout << " tp ";
    if (result_config.write_tgeom)
      std::cout << " tgeom ";
    std::cout << "\n";
    std::cout << "------------------------------------------\n";
  };

  bool validate_mm()
  {
    SPDLOG_INFO("Validating configuration");
    if (!UTIL::fileExists(gps_file))
    {
      SPDLOG_CRITICAL("GPS file {} not found",gps_file);
      return false;
    };
    if (get_gps_format()<0){
      SPDLOG_CRITICAL("Unknown GPS format");
      return false;
    }
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
    SPDLOG_INFO("Validating done");
    return true;
  };

  // Check gps format 0 for GDAL shapefile, 1 for trajectory CSV file
  int get_gps_format(){
    std::string fn_extension = gps_file.substr(
      gps_file.find_last_of(".") + 1);
    if (fn_extension == "csv" || fn_extension == "txt") {
      return 1;
    } else if (fn_extension == "db" || fn_extension == "shp") {
      return 0;
    } else {
      SPDLOG_CRITICAL("GPS file extension {} unknown",fn_extension);
      return -1;
    }
  };

  static std::set<std::string> string2set(const std::string &s,
                                          char delim=','){
    std::set<std::string> result;
    std::stringstream ss(s);
    std::string intermediate;
    while(getline(ss, intermediate, delim))
    {
      result.insert(intermediate);
    }
    return result;
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
  std::string gps_geom;

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
  UBODT_Config(int argc, char **argv){
    if (argc==2) {
      std::string configfile(argv[1]);
      initialize_xml(configfile);
    } else {
      initialize_arg(argc,argv);
    }
    std::cout<<"Set log level as "<<LOG_LEVESLS[log_level]<<"\n";
    spdlog::set_level((spdlog::level::level_enum) log_level);
    spdlog::set_pattern("[%l][%s:%-3#] %v");
  };

  void initialize_xml(const std::string &file)
  {
    // Create empty property tree object
    boost::property_tree::ptree tree;
    std::cout << "Read configuration from xml file: " << file << '\n';
    boost::property_tree::read_xml(file, tree);
    // Parse the XML into the property tree.

    // UBODT configuration
    delta = tree.get("ubodt_config.parameters.delta", 3000.0);

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

  void initialize_arg(int argc, char **argv){
    std::cout<<"Start reading ubodt configuration from arguments\n";
    cxxopts::Options options("ubodt_config", "Configuration parser of ubodt_gen");
    options.add_options()
      ("a,network","Network file name", cxxopts::value<std::string>())
      ("b,id","Network id name",
      cxxopts::value<std::string>()->default_value("id"))
      ("c,source","Network source name",
      cxxopts::value<std::string>()->default_value("source"))
      ("t,target","Network target name",
      cxxopts::value<std::string>()->default_value("target"))
      ("d,delta","Upperbound distance",
      cxxopts::value<double>()->default_value("3000.0"))
      ("o,output","Output file name", cxxopts::value<std::string>())
      ("l,log_level","Log level",cxxopts::value<int>()->default_value("2"));

    auto result = options.parse(argc, argv);
    // Output
    result_file = result["output"].as<std::string>();
    binary_flag = UTIL::get_file_extension(result_file);

    network_file = result["network"].as<std::string>();
    network_id = result["id"].as<std::string>();
    network_source = result["source"].as<std::string>();
    network_target = result["target"].as<std::string>();

    log_level = result["log_level"].as<int>();
    delta = result["delta"].as<double>();
  };

  void print()
  {
    std::cout << "------------------------------------------\n";
    std::cout << "UBODT Configuration: \n";
    std::cout << "  Network_file: " << network_file << '\n';;
    std::cout << "  Network id: " << network_id << '\n';
    std::cout << "  Network source: " << network_source << '\n';
    std::cout << "  Network target: " << network_target << '\n';
    std::cout << "  delta: " << delta << '\n';
    std::cout << "  Output file:" << result_file << '\n';
    std::cout << "  Output format(1 binary, 0 csv): " << binary_flag << '\n';
    std::cout << "  log_level:" << LOG_LEVESLS[log_level] << '\n';
    std::cout << "------------------------------------------\n";
  };

  static void print_help(){
    std::cout<<"ubodt_gen argument lists:\n";
    std::cout<<"--network (required) <string>: Network file name\n";
    std::cout<<"--output (required) <string>: Output file name\n";
    std::cout<<"--id (optional) <string>: Network id name (id)\n";
    std::cout<<"--source (optional) <string>: Network source name (source)\n";
    std::cout<<"--target (optional) <string>: Network target name (target)\n";
    std::cout<<"--delta (optional) <double>: upperbound (3000.0)\n";
    std::cout<<"--log_level (optional) <int>: log level (2)\n";
    std::cout<<"For xml configuration, check example folder\n";
  };

  bool validate()
  {
    SPDLOG_INFO("Validating configuration for UBODT construction");
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
    SPDLOG_INFO("Validating done.");
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
