//
// Created by Can Yang on 2020/4/1.
//

#include "mm/stmatch/stmatch_app_config.hpp"
#include "util/debug.hpp"
#include "util/util.hpp"
using namespace FMM;
using namespace FMM::CORE;
using namespace FMM::NETWORK;
using namespace FMM::MM;
using namespace FMM::CONFIG;

STMATCHAppConfig::STMATCHAppConfig(int argc, char **argv){
  spdlog::set_pattern("[%^%l%$][%s:%-3#] %v");
  if (argc==2) {
    std::string configfile(argv[1]);
    if (UTIL::check_file_extension(configfile,"xml,XML"))
      load_xml(configfile);
    else {
      load_arg(argc,argv);
    }
  } else {
    load_arg(argc,argv);
  }
  spdlog::set_level((spdlog::level::level_enum) log_level);
  if (!help_specified)
    print();
};

void STMATCHAppConfig::load_xml(const std::string &file){
  SPDLOG_INFO("Start with reading stmatch xml configuration {}",
    file);
  // Create empty property tree object
  boost::property_tree::ptree tree;
  boost::property_tree::read_xml(file, tree);
  network_config = NetworkConfig::load_from_xml(tree);
  gps_config = GPSConfig::load_from_xml(tree);
  result_config = CONFIG::ResultConfig::load_from_xml(tree);
  stmatch_config = STMATCHConfig::load_from_xml(tree);
  log_level = tree.get("config.other.log_level",2);
  step =  tree.get("config.other.step",100);
  use_omp = !(!tree.get_child_optional("config.other.use_omp"));
  SPDLOG_INFO("Finish with reading stmatch xml configuration");
};

void STMATCHAppConfig::load_arg(int argc, char **argv){
  SPDLOG_INFO("Start reading stmatch configuration from arguments");
  cxxopts::Options options("stmatch_config", "Configuration parser");
  // Register options
  NetworkConfig::register_arg(options);
  GPSConfig::register_arg(options);
  ResultConfig::register_arg(options);
  STMATCHConfig::register_arg(options);
  options.add_options()
    ("l,log_level","Log level",cxxopts::value<int>()->default_value("2"))
    ("s,step","Step report",cxxopts::value<int>()->default_value("100"))
    ("h,help","Help information")
    ("use_omp","Use omp or not");
  if (argc==1) {
    help_specified = true;
    return;
  }
  // Parse options
  auto result = options.parse(argc, argv);
  // Read options
  network_config = NetworkConfig::load_from_arg(result);
  gps_config = GPSConfig::load_from_arg(result);
  result_config = CONFIG::ResultConfig::load_from_arg(result);
  stmatch_config = STMATCHConfig::load_from_arg(result);
  log_level = result["log_level"].as<int>();
  step = result["step"].as<int>();
  use_omp = result.count("use_omp")>0;
  if (result.count("help")>0){
    help_specified = true;
  }
  SPDLOG_INFO("Finish with reading stmatch arg configuration");
};

void STMATCHAppConfig::print() const {
  SPDLOG_INFO("----   Print configuration    ----")
  network_config.print();
  gps_config.print();
  result_config.print();
  stmatch_config.print();
  SPDLOG_INFO("Log level {}",UTIL::LOG_LEVESLS[log_level])
  SPDLOG_INFO("Step {}",step)
  SPDLOG_INFO("Use omp {}",(use_omp ? "true" : "false"))
  SPDLOG_INFO("---- Print configuration done ----")
};

void STMATCHAppConfig::print_help(){
  std::ostringstream oss;
  oss<<"stmatch argument lists:\n";
  NetworkConfig::register_help(oss);
  GPSConfig::register_help(oss);
  STMATCHConfig::register_help(oss);
  ResultConfig::register_help(oss);
  oss<<"-l/--log_level (optional) <int>: log level (2)\n";
  oss<<"-s/--step (optional) <int>: progress report step (100)\n";
  oss<<"--use_omp: use OpenMP for multithreaded map matching\n";
  oss<<"-h/--help:print help information\n";
  oss<<"For xml configuration, check example folder\n";
  std::cout<<oss.str();
}
bool STMATCHAppConfig::validate() const {
  if (!gps_config.validate()) {
    return false;
  }
  if (!result_config.validate()) {
    return false;
  }
  if (!network_config.validate()) {
    return false;
  }
  if (!stmatch_config.validate()) {
    return false;
  }
  return true;
};
