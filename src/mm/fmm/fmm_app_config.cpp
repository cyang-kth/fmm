//
// Created by Can Yang on 2020/4/1.
//

#include "mm/fmm/fmm_app_config.hpp"
#include "util/debug.hpp"
#include "util/util.hpp"
#include "result_config.hpp"
#include "network_config.hpp"
#include "gps_config.hpp"

namespace MM {

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
  network_config = MM::CONFIG::NetworkConfig::load_from_xml(tree);
  gps_config = MM::CONFIG::GPSConfig::load_from_xml(tree);
  result_config = CONFIG::ResultConfig::load_from_xml(tree);
  fmm_config = FMMConfig::load_from_xml(tree);
  // UBODT
  ubodt_file = tree.get<std::string>("config.input.ubodt.file");
  log_level = tree.get("config.other.log_level",2);
  step =  tree.get("config.other.step",100);
  use_omp = !(!tree.get_child_optional("config.other.use_omp"));
  projected = !(!tree.get_child_optional("config.other.projected"));
  SPDLOG_INFO("Finish with reading FMM xml configuration");
};

void FMMAppConfig::load_arg(int argc, char **argv){
  SPDLOG_INFO("Start reading FMM configuration from arguments");
  cxxopts::Options options("fmm_config", "Configuration parser of fmm");
  options.add_options()
    ("ubodt","Ubodt file name",
    cxxopts::value<std::string>()->default_value(""))
    ("network","Network file name",
    cxxopts::value<std::string>()->default_value(""))
    ("network_id","Network id name",
    cxxopts::value<std::string>()->default_value("id"))
    ("source","Network source name",
    cxxopts::value<std::string>()->default_value("source"))
    ("target","Network target name",
    cxxopts::value<std::string>()->default_value("target"))
    ("gps","GPS file name",
    cxxopts::value<std::string>()->default_value(""))
    ("gps_id","GPS file id",
    cxxopts::value<std::string>()->default_value("id"))
    ("gps_x","GPS x name",
    cxxopts::value<std::string>()->default_value("x"))
    ("gps_y","GPS y name",
    cxxopts::value<std::string>()->default_value("y"))
    ("gps_geom","GPS file geom column name",
    cxxopts::value<std::string>()->default_value("geom"))
    ("gps_timestamp",   "GPS file timestamp column name",
    cxxopts::value<std::string>()->default_value("timestamp"))
    ("k,candidates","Number of candidates",
    cxxopts::value<int>()->default_value("8"))
    ("r,radius","Search radius",
    cxxopts::value<double>()->default_value("300.0"))
    ("e,error","GPS error",
    cxxopts::value<double>()->default_value("50.0"))
    ("o,output","Output file name",
    cxxopts::value<std::string>()->default_value(""))
    ("output_fields","Output fields",
    cxxopts::value<std::string>()->default_value(""))
    ("l,log_level","Log level",cxxopts::value<int>()->default_value("2"))
    ("step","Step report",cxxopts::value<int>()->default_value("100"))
    ("h,help",   "Help information")
    ("gps_point","GPS point or not")
    ("use_omp","Use parallel computing if specified")
    ("projected","Data is projected or not");
  if (argc==1) {
    help_specified = true;
    return;
  }
  auto result = options.parse(argc, argv);
  ubodt_file = result["ubodt"].as<std::string>();
  network_config = MM::CONFIG::NetworkConfig::load_from_arg(result);
  gps_config = MM::CONFIG::GPSConfig::load_from_arg(result);
  result_config = CONFIG::ResultConfig::load_from_arg(result);
  fmm_config = FMMConfig::load_from_arg(result);
  log_level = result["log_level"].as<int>();
  step = result["step"].as<int>();
  use_omp = result.count("use_omp")>0;
  projected = result.count("projected")>0;
  if (result.count("help")>0) {
    help_specified = true;
  }
  SPDLOG_INFO("Finish with reading FMM arg configuration");
};

void FMMAppConfig::print_help(){
  std::cout<<"fmm argument lists:\n";
  std::cout<<"--ubodt (required) <string>: Ubodt file name\n";
  std::cout<<"--network (required) <string>: Network file name\n";
  std::cout<<"--network_id (optional) <string>: Network id name (id)\n";
  std::cout<<"--source (optional) <string>: Network source name (source)\n";
  std::cout<<"--target (optional) <string>: Network target name (target)\n";
  std::cout<<"--gps (required) <string>: GPS file name\n";
  std::cout<<"--gps_id (optional) <string>: GPS id name (id)\n";
  std::cout<<"--gps_x (optional) <string>: GPS x name (x)\n";
  std::cout<<"--gps_y (optional) <string>: GPS y name (y)\n";
  std::cout<<"--gps_timestamp (optional) <string>: "
    "GPS timestamp name (timestamp)\n";
  std::cout<<"--gps_geom (optional) <string>: GPS geometry name (geom)\n";
  std::cout<<"-r/--radius (optional) <double>: search "
             "radius (network data unit) (300)\n";
  std::cout<<"-e/--error (optional) <double>: GPS error "
             "(network data unit) (50)\n";
  std::cout<<"--output (required) <string>: Output file name\n";
  std::cout<<"--output_fields (optional) <string>: Output fields\n";
  std::cout<<"  opath,cpath,tpath,ogeom,mgeom,pgeom,\n";
  std::cout<<"  offset,error,spdist,tp,ep,length,all\n";
  std::cout<<"--log_level (optional) <int>: log level (2)\n";
  std::cout<<"--step (optional) <int>: progress report step (100)\n";
  std::cout<<"--use_omp: use OpenMP for multithreaded map matching\n";
  std::cout<<"-h/--help:print help information\n";
  std::cout<<"For xml configuration, check example folder\n";
};

void FMMAppConfig::print() const {
  SPDLOG_INFO("----   Print configuration    ----");
  network_config.print();
  gps_config.print();
  result_config.print();
  fmm_config.print();
  SPDLOG_INFO("Log level {}",LOG_LEVESLS[log_level]);
  SPDLOG_INFO("Step {}",step);
  SPDLOG_INFO("Use omp {}",(use_omp ? "true" : "false"));
  SPDLOG_INFO("---- Print configuration done ----");
};

bool FMMAppConfig::validate() const
{
  SPDLOG_DEBUG("Validating configuration");
  if (log_level<0 || log_level>LOG_LEVESLS.size()) {
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

}
