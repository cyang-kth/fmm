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

namespace MM
{

/**
 * Configuration class for map matching
 */
class AppConfig
{
public:
  AppConfig(int argc, char **argv);
  void initialize_xml(const std::string &file);
  void initialize_arg(int argc, char **argv);
  ResultConfig get_result_config(){
    return result_config;
  };
  static void print_help();
  void print();
  bool validate_mm();
  int get_gps_format();
  static std::set<std::string> string2set(const std::string &s,
                                          char delim=',');

  std::string network_file;
  std::string network_id;
  std::string network_source;
  std::string network_target;

  // UBODT configurations
  std::string ubodt_file;
  double delta;
  int binary_flag;

  // GPS file
  std::string gps_file;
  std::string gps_id;
  std::string gps_geom;

  // Result file
  std::string result_file;

  // Used by Rtree search
  int k;
  double radius;
  double gps_error;

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
    cxxopts::Options options("ubodt_config",
                             "Configuration parser of ubodt_gen");
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
