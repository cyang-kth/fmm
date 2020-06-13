//
// Created by Can Yang on 2020/4/1.
//

#include "mm/fmm/fmm_app_config.hpp"
#include "util/debug.hpp"
#include "util/util.hpp"

using namespace FMM::CORE;
using namespace FMM::NETWORK;
using namespace FMM::CONFIG;
using namespace FMM::MM;

FMMAppConfig::FMMAppConfig(int argc, char **argv){
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

void FMMAppConfig::load_xml(const std::string &file){
  SPDLOG_INFO("Start with reading FMM configuration {}",file);
  // Create empty property tree object
  boost::property_tree::ptree tree;
  boost::property_tree::read_xml(file, tree);
  network_config = NetworkConfig::load_from_xml(tree);
  gps_config = GPSConfig::load_from_xml(tree);
  result_config = CONFIG::ResultConfig::load_from_xml(tree);
  fmm_config = FastMapMatchConfig::load_from_xml(tree);
  // UBODT
  ubodt_file = tree.get<std::string>("config.input.ubodt.file");
  log_level = tree.get("config.other.log_level",2);
  step =  tree.get("config.other.step",100);
  use_omp = !(!tree.get_child_optional("config.other.use_omp"));
  SPDLOG_INFO("Finish with reading FMM xml configuration");
};

void FMMAppConfig::load_arg(int argc, char **argv){
  SPDLOG_INFO("Start reading FMM configuration from arguments");
  cxxopts::Options options("fmm_config", "Configuration parser of fmm");
  NetworkConfig::register_arg(options);
  GPSConfig::register_arg(options);
  ResultConfig::register_arg(options);
  FastMapMatchConfig::register_arg(options);
  options.add_options()
    ("ubodt","Ubodt file name",
    cxxopts::value<std::string>()->default_value(""))
    ("l,log_level","Log level",cxxopts::value<int>()->default_value("2"))
    ("s,step","Step report",cxxopts::value<int>()->default_value("100"))
    ("h,help","Help information")
    ("use_omp","Use parallel computing if specified");
  if (argc==1) {
    help_specified = true;
    return;
  }
  auto result = options.parse(argc, argv);
  network_config = NetworkConfig::load_from_arg(result);
  gps_config = GPSConfig::load_from_arg(result);
  result_config = CONFIG::ResultConfig::load_from_arg(result);
  fmm_config = FastMapMatchConfig::load_from_arg(result);
  ubodt_file = result["ubodt"].as<std::string>();
  log_level = result["log_level"].as<int>();
  step = result["step"].as<int>();
  use_omp = result.count("use_omp")>0;
  if (result.count("help")>0) {
    help_specified = true;
  }
  SPDLOG_INFO("Finish with reading FMM arg configuration");
};

void FMMAppConfig::print_help(){
  std::ostringstream oss;
  oss<<"fmm argument lists:\n";
  oss<<"--ubodt (required) <string>: Ubodt file name\n";
  NetworkConfig::register_help(oss);
  GPSConfig::register_help(oss);
  ResultConfig::register_help(oss);
  FastMapMatchConfig::register_help(oss);
  oss<<"-l/--log_level (optional) <int>: log level (2)\n";
  oss<<"-s/--step (optional) <int>: progress report step (100)\n";
  oss<<"--use_omp: use OpenMP for multithreaded map matching\n";
  oss<<"-h/--help:print help information\n";
  oss<<"For xml configuration, check example folder\n";
  std::cout<<oss.str();
};

void FMMAppConfig::print() const {
  SPDLOG_INFO("----   Print configuration    ----");
  network_config.print();
  gps_config.print();
  result_config.print();
  fmm_config.print();
  SPDLOG_INFO("Log level {}",UTIL::LOG_LEVESLS[log_level]);
  SPDLOG_INFO("Step {}",step);
  SPDLOG_INFO("Use omp {}",(use_omp ? "true" : "false"));
  SPDLOG_INFO("---- Print configuration done ----");
};

bool FMMAppConfig::validate() const
{
  SPDLOG_DEBUG("Validating configuration");
  if (log_level<0 || log_level>UTIL::LOG_LEVESLS.size()) {
    SPDLOG_CRITICAL("Invalid log_level {}, which should be 0 - 6",log_level);
    SPDLOG_CRITICAL("0-trace,1-debug,2-info,3-warn,4-err,5-critical,6-off");
    return false;
  }
  if (!gps_config.validate()) {
    return false;
  }
  if (!result_config.validate()) {
    return false;
  }
  if (!network_config.validate()) {
    return false;
  }
  if (!fmm_config.validate()) {
    return false;
  }
  if (!UTIL::file_exists(ubodt_file)) {
    SPDLOG_CRITICAL("UBODT file not exists {}", ubodt_file);
    return false;
  }
  SPDLOG_DEBUG("Validating done");
  return true;
};
