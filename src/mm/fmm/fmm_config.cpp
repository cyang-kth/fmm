/**
 * Content
 * Configuration Class defined for the two application
 *
 * @author: Can Yang
 * @version: 2019.03.27
 */
#include "mm/mm_app_config.hpp"
#include "util/debug.hpp"
#include "util/util.hpp"

#include "cxxopts/cxxopts.hpp"

#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <string>
#include <set>
#include <exception>
#include <iomanip>
#include <mm/stmatch/stmatch.hpp>

namespace MM
{

const std::vector<std::string> MMAppConfig::LOG_LEVESLS({
    "0-trace","1-debug","2-info","3-warn","4-err","5-critical","6-off"});
const std::vector<std::string> MMAppConfig::ALGORITHM_IDS({
    "0-CWRMM","1-FCDMM","2-STMATCH"});

MMAppConfig::MMAppConfig(int argc, char **argv)
{
  if (argc==2) {
    std::string configfile(argv[1]);
    if (UTIL::check_file_extension(configfile,"xml,XML"))
      initialize_xml(configfile);
    else {
      initialize_arg(argc,argv);
    }
  } else {
    initialize_arg(argc,argv);
  }
  spdlog::set_level((spdlog::level::level_enum) log_level);
  spdlog::set_pattern("[%^%l%$][%s:%-3#] %v");
  if (!help_specified)
    print();
}

void MMAppConfig::initialize_xml(const std::string &file)
{
  SPDLOG_DEBUG("Start reading FMM configuration from xml file")
  // Create empty property tree object
  boost::property_tree::ptree tree;
  boost::property_tree::read_xml(file, tree);

  // Network
  network_file = tree.get<std::string>("mm_config.input.network.file");
  network_id = tree.get("mm_config.input.network.id", "id");
  network_source = tree.get("mm_config.input.network.source", "source");
  network_target = tree.get("mm_config.input.network.target", "target");

  // GPS
  gps_file = tree.get<std::string>("mm_config.input.gps.file");
  gps_id = tree.get("mm_config.input.gps.id", "id");
  gps_geom = tree.get("mm_config.input.gps.geom", "geom");
  gps_timestamp = tree.get("mm_config.input.gps.timestamp", "timestamp");

  // Other parameters
  k = tree.get("mm_config.parameters.k", 8);
  radius = tree.get("mm_config.parameters.r", 300.0);

  // HMM
  gps_error = tree.get("mm_config.parameters.gps_error", 50.0);
  weight = tree.get("mm_config.parameters.weight", 1.0);
  vmax = tree.get("mm_config.parameters.vmax",18);
  factor = tree.get("mm_config.parameters.factor",1.5);
  algorithm_id = static_cast<MM_ALGOR_TAG>(
    tree.get("mm_config.parameters.algorithm",0));
  data_projected = UTIL::string2bool(
    tree.get("mm_config.parameters.projected","false"));

  // Output
  result_file = tree.get<std::string>("mm_config.output.file");

  if (tree.get_child_optional("mm_config.output.fields")) {
    // Fields specified
    // close the default output fields (cpath,mgeom are true by default)
    result_config.write_cpath = false;
    result_config.write_mgeom = false;
    if (tree.get_child_optional("mm_config.output.fields.opath")) {
      result_config.write_opath = true;
    }
    if (tree.get_child_optional("mm_config.output.fields.cpath")) {
      result_config.write_cpath = true;
    }
    if (tree.get_child_optional("mm_config.output.fields.mgeom")) {
      result_config.write_mgeom = true;
    }
    if (tree.get_child_optional("mm_config.output.fields.all")) {
      result_config.write_opath = true;
      result_config.write_cpath = true;
      result_config.write_mgeom = true;
    }
  } else {
    std::cout << "    Default output fields used.\n";
  }

  // Others
  step = tree.get("mm_config.others.step", 1);
  log_level = tree.get("mm_config.others.log_level", 5);
  int verbose_flag = tree.get("mm_config.others.verbose", 0);
  verbose = (verbose_flag==1);
  SPDLOG_DEBUG("Finish with reading FMM configuration")
}

void MMAppConfig::initialize_arg(int argc, char **argv)
{
  SPDLOG_DEBUG("Start reading FMM configuration from arguments")
  cxxopts::Options options("mm_config", "Configuration parser of mm");
  options.add_options()
    ("n,network","Network file name",
        cxxopts::value<std::string>()->default_value(""))
    ("network_id","Network id name",
    cxxopts::value<std::string>()->default_value("id"))
    ("source","Network source name",
    cxxopts::value<std::string>()->default_value("source"))
    ("target","Network target name",
    cxxopts::value<std::string>()->default_value("target"))
    ("g,gps",   "GPS file name",
        cxxopts::value<std::string>()->default_value(""))
    ("gps_id",   "GPS file id",
    cxxopts::value<std::string>()->default_value("id"))
    ("gps_geom",   "GPS file geom column name",
    cxxopts::value<std::string>()->default_value("geom"))
    ("gps_x",   "GPS file x column name",
     cxxopts::value<std::string>()->default_value("x"))
    ("gps_y",   "GPS file y column name",
     cxxopts::value<std::string>()->default_value("y"))
    ("gps_point",   "GPS file in point form")
    ("gps_timestamp",   "GPS file timestamp column name",
    cxxopts::value<std::string>()->default_value("timestamp"))
    ("k,candidates",   "Number of candidates",
    cxxopts::value<int>()->default_value("8"))
    ("r,radius",   "Search radius",
    cxxopts::value<double>()->default_value("300"))
    ("e,error",   "GPS error",
    cxxopts::value<double>()->default_value("50"))
    ("w,weight",   "Weight",
    cxxopts::value<double>()->default_value("1.0"))
    ("vmax",   "Vmax",
    cxxopts::value<double>()->default_value("80.0"))
    ("projected",   "Data projected or not",
    cxxopts::value<std::string>()->default_value("false"))
    ("factor",   "Factor",
    cxxopts::value<double>()->default_value("1.5"))
    ("a,algorithm",   "Algorithm id",
    cxxopts::value<std::string>()->default_value(""))
    ("o,output", "Output file name",
        cxxopts::value<std::string>()->default_value(""))
    ("output_fields",   "Output fields",
        cxxopts::value<std::string>()->default_value(""))
    ("s,step",   "Log steps", cxxopts::value<int>()->default_value("1"))
    ("l,log_level",   "Log level", cxxopts::value<int>()->default_value("2"))
    ("log_file",   "Log file for mm_app",
       cxxopts::value<std::string>()->default_value(""))
    ("ubodt_file",   "Ubodt file for fmm_shell",
        cxxopts::value<std::string>()->default_value(""))
    ("script",   "Script file",
     cxxopts::value<std::string>()->default_value(""))
    ("mm_log_level",   "Log level for MM",
        cxxopts::value<int>()->default_value("5"))
    ("mm_log_file",   "Log file for map matching algorithm",
       cxxopts::value<std::string>()->default_value(""))
    ("node_limit",   "Node limit",
    cxxopts::value<int>()->default_value("10000"))
    ("v,verbose",   "Verbose of node visited")
    ("h,help",   "Help information");

  if (argc==1)
    help_specified = true;
  auto result = options.parse(argc, argv);
  network_file = result["network"].as<std::string>();
  network_id = result["network_id"].as<std::string>();
  network_source = result["source"].as<std::string>();
  network_target = result["target"].as<std::string>();

  // GPS
  gps_file = result["gps"].as<std::string>();
  gps_id = result["gps_id"].as<std::string>();
  gps_geom = result["gps_geom"].as<std::string>();
  gps_x = result["gps_x"].as<std::string>();
  gps_y = result["gps_y"].as<std::string>();
  if (result.count("gps_point")>0)
    gps_point = true;
  gps_timestamp = result["gps_timestamp"].as<std::string>();

  // Other parameters
  k = result["k"].as<int>();
  radius = result["radius"].as<double>();

  // HMM
  gps_error = result["error"].as<double>();
  weight = result["weight"].as<double>();
  vmax = result["vmax"].as<double>();
  factor = result["factor"].as<double>();
  algorithm_id = string2algor(
    result["algorithm"].as<std::string>());
  data_projected = UTIL::string2bool(result["projected"].as<std::string>());
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

    if (dict.find("all")!=dict.end()) {
      result_config.write_opath = true;
      result_config.write_cpath = true;
      result_config.write_mgeom = true;
    }
  }

  // Others
  step = result["step"].as<int>();
  log_level = result["log_level"].as<int>();
  log_file = result["log_file"].as<std::string>();
  ubodt_file = result["ubodt_file"].as<std::string>();
  script_file = result["script"].as<std::string>();
  mm_log_level = result["mm_log_level"].as<int>();
  mm_log_file = result["mm_log_file"].as<std::string>();
  verbose = (result.count("verbose")>0);
  node_limit = result["node_limit"].as<int>();
  if (result.count("help")>0)
    help_specified = true;
  SPDLOG_DEBUG("Finish with reading FMM configuration")
}

std::set<std::string> MMAppConfig::string2set(
  const std::string &s,char delim) {
  std::set<std::string> result;
  // stringstream class check1
  std::stringstream check1(s);

  std::string intermediate;

  // Tokenizing w.r.t. space ' '
  while(getline(check1, intermediate, delim))
  {
    result.insert(intermediate);
  }

  return result;
}

OutputConfig MMAppConfig::get_result_config() const {
  return result_config;
}

MMConfig MMAppConfig::get_fmm_config() const {
  double convert_factor = 1.0;
  if (!data_projected) {
    // Transform meter into degrees
    convert_factor = 8.98303e-6;
  }
  return MMConfig{k,
                  radius*convert_factor,
                  gps_error*convert_factor,weight,
                  vmax/3.6*convert_factor,factor};
}

MMConfig MMAppConfig::get_mm_config() const {
  double convert_factor = 1.0;
  if (!data_projected) {
    // Transform meter into degrees
    convert_factor = 8.98303e-6;
  }
  switch (algorithm_id) {
  case (MM_ALGOR_TAG::CWRMM):
    SPDLOG_INFO("Create CWRMM configuration")
    return MMConfigFactory::createMMConfigCWRMM(
      k, radius*convert_factor, weight
      );
    break;
  case (MM_ALGOR_TAG::FCDMM):
    SPDLOG_INFO("Create FCDMM configuration")
    return MMConfigFactory::createMMConfigFCDMM(
      k, radius*convert_factor, weight, vmax/3.6*convert_factor, factor
      );
    break;
  case (MM_ALGOR_TAG::STMATCH):
    SPDLOG_INFO("Create STMATCH configuration")
    return MMConfigFactory::createMMConfigSTMATCH(
      k, radius*convert_factor, gps_error*convert_factor,
      vmax/3.6*convert_factor, factor
      );
    break;
  default:
    break;
  }
  SPDLOG_CRITICAL("MM Algorithm not found")
  return MMConfig{k,
                  radius*convert_factor,
                  gps_error*convert_factor,weight,
                  vmax/3.6*convert_factor,factor};
}

// 0 for CSV and 1 for GeoPackage or shp
int MMAppConfig::GetOutputFormat(){
  std::string fn_extension = result_file.substr(
    result_file.find_last_of('.') + 1);
  if (fn_extension == "csv" || fn_extension == "txt") {
    return 0;
  } else if (fn_extension == "db" || fn_extension == "sqlite") {
    return 1;
  }
  SPDLOG_CRITICAL("Program stops as the output format is unknown")
  std::exit(EXIT_FAILURE);
}

int MMAppConfig::GetInputFormat(){
  std::string fn_extension = gps_file.substr(
    gps_file.find_last_of('.') + 1);
  if (fn_extension == "csv" || fn_extension == "txt") {
    return 0;
  } else if (fn_extension == "db" || fn_extension == "shp") {
    return 1;
  }
  SPDLOG_CRITICAL("Program stops as the output format is unknown")
  std::exit(EXIT_FAILURE);
}

void MMAppConfig::print_help(){
  std::cout<<"mm_app argument lists:\n";
  std::cout<<"--network (required) <string>: Network file name\n";
  std::cout<<"--gps (required) <string>: GPS file name\n";
  std::cout<<"--output (required) <string>: Output file name\n";
  std::cout<<"--network_id (optional) <string>: Network id name (id)\n";
  std::cout<<"--source (optional) <string>: Network source name (source)\n";
  std::cout<<"--target (optional) <string>: Network target name (target)\n";
  std::cout<<"--gps_id (optional) <string>: GPS id name (id)\n";
  std::cout<<"--gps_geom (optional) <string>: GPS geometry name (geom)\n";
  std::cout<<"--gps_timestamp (optional) <string>: "
    "GPS timestamp name (timestamp)\n";
  std::cout<<"--weight (optional) <double>: weight of GPS error (1.0)\n";
  std::cout<<"--vmax (optional) <double>: maximum speed km/h (80)\n";
  std::cout<<"--factor (optional) <double>: factor of search (1.5)\n";
  std::cout<<"--candidates (optional) <int>: number of candidates (8)\n";
  std::cout<<"--algorithm_id (optional) <int>: algorithm id (0)\n";
  std::cout<<"--radius (optional) <double>: search radius in meter (300) \n";
  std::cout<<"--error (optional) <double>: GPS error in meter (50)\n";
  std::cout<<"--projected (optional) <bool>: Data projected or not (false) \n";
  std::cout<<"--output_fields (optional)<string>: "
           <<"Output fields separated by comma (id,cpath,mgeom)\n";
  std::cout<<"--step (optional) <int>: step in progress report (0)\n";
  std::cout<<"--log_level (optional) <int>: loglevel (5)\n";
  std::cout<<"--verbose (optional) : verbose of node visited\n";
  std::cout<<"--node_limit (optional) <int> : node limit per point (10000)\n";
  std::cout<<"For xml configuration, check example folder\n";
}

void MMAppConfig::print() const
{
  std::cout << "------------------------------------------\n";
  std::cout << "Configuration parameters for map matching application:\n";
  std::cout << "Network_file: " << network_file << "\n";
  std::cout << "Network id: " << network_id << "\n";
  std::cout << "Network source: " << network_source << "\n";
  std::cout << "Network target: " << network_target << "\n";
  std::cout << "gps_file: " << gps_file << "\n";
  std::cout << "gps_id: " << gps_id << "\n";
  std::cout << "gps_geom: " << gps_geom << "\n";
  std::cout << "gps_timestamp: " << gps_timestamp << "\n";
  std::cout << "k: " << k << "\n";
  std::cout << "radius: " << radius << " meters\n";
  std::cout << "gps_error: " << gps_error << " meters\n";
  std::cout << "weight: "<< weight <<"\n";
  std::cout << "vmax: "<< vmax <<" km/h \n";
  std::cout << "projected: "<< (data_projected ? "true" : "false") <<"\n";
  std::cout << "factor: "<< factor <<"\n";
  std::cout << "algorithm_id: "<< ALGORITHM_IDS[
    static_cast<int>(algorithm_id)] <<"\n";
  std::cout << "result_file:" << result_file << "\n";
  std::cout << "Output fields:"<<"\n";

  if (result_config.write_opath)
    std::cout <<"opath"<<"\n";

  if (result_config.write_cpath)
    std::cout << "cpath"<<"\n";

  if (result_config.write_mgeom)
    std::cout << "mgeom"<<"\n";
  std::cout << "step:" << step << "\n";
  std::cout << "log_level:"<< log_level <<"\n";
  std::cout << "mm_log_level:"<< mm_log_level <<"\n";
  std::cout << "mm_log_file:"<< mm_log_file <<"\n";
  std::cout << "Verbose:"<< verbose <<"\n";
  std::cout << "Node limit:"<< node_limit <<"\n";
  std::cout << "------------------------------------------" << "\n";
}

bool MMAppConfig::validate() const
{
  SPDLOG_INFO("Validating configuration for map match application:")
  if (UTIL::file_exists(result_file))
  {
    SPDLOG_WARN("Overwrite existing result file {}.",result_file)
  }
  if (!UTIL::file_exists(gps_file))
  {
    SPDLOG_INFO("Error, GPS_file not found. Program stop.")
    return false;
  }
  if (gps_error <= 0 || radius <= 0 || k <= 0)
  {
    SPDLOG_INFO("Error, Algorithm parameters invalid.")
    return false;
  }
  SPDLOG_INFO("Validating success.")
  return true;
}

bool MMAppConfig::validate_network() const {
  if (!UTIL::file_exists(network_file))
  {
    SPDLOG_INFO("Error, Network file not found. Program stop.")
    return false;
  }
  return true;
}

bool MMAppConfig::validate_script() const {
  if (!UTIL::file_exists(script_file))
  {
    SPDLOG_INFO("Error, script file not found. Program stop.")
    return false;
  }
  return true;
}


bool MMAppConfig::validate_gps() const {
  if (!UTIL::file_exists(gps_file))
  {
    SPDLOG_INFO("Error, GPS_file not found. Program stop.")
    return false;
  }
  if (gps_error <= 0 || radius <= 0 || k <= 0)
  {
    SPDLOG_INFO("Error, Algorithm parameters invalid.")
    return false;
  }
  return true;
}

bool MMAppConfig::validate_ubodt() const {
  if (!UTIL::file_exists(ubodt_file))
  {
    SPDLOG_INFO("Error, UBODT not found. Program stop.")
    return false;
  }
  return true;
}

} // MM
