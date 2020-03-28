#include "mm/fmm/fmm_config.hpp"

namespace MM {

FMMAppConfig::FMMAppConfig(int argc, char **argv){
  if (argc==2) {
    std::string configfile(argv[1]);
    load_xml(configfile);
  } else {
    load_arg(argc,argv);
  }
  std::cout<<"Set log level as "<<LOG_LEVESLS[log_level]<<"\n";
  spdlog::set_level((spdlog::level::level_enum) log_level);
  spdlog::set_pattern("[%l][%s:%-3#] %v");
};

void FMMAppConfig::load_xml(const std::string &file){
  std::cout<<"Start with reading FMM configuration "<<file<<"\n";
  // Create empty property tree object
  boost::property_tree::ptree tree;
  boost::property_tree::read_xml(file, tree);
  // UBODT
  ubodt_file = tree.get<std::string>("mm_config.input.ubodt.file");

  network_config = NetworkConfig::load_from_xml();
  gps_config = GPSConfig::load_from_xml();
  result_config = ResultConfig::load_from_xml();
  fmm_config = FMMConfig::load_from_xml();

  log_level = tree.get("mm_config.other.log_level",2);
  std::cout<<"Finish with reading FMM xml configuration.\n";
};

void FMMAppConfig::load_arg(int argc, char **argv){
  std::cout<<"Start reading FMM configuration from arguments\n";
  cxxopts::Options options("mm_config", "Configuration parser of fmm");
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
  network_config = NetworkConfig::load_from_arg();
  gps_config = GPSConfig::load_from_arg();
  result_config = ResultConfig::load_from_arg();
  fmm_config = FMMConfig::load_from_arg();

  log_level = tree.get("mm_config.other.log_level",2);
  std::cout<<"Finish with reading FMM arg configuration\n";
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
  std::cout<<"  offset,error,spdist,tp,ep,length,all\n";
  std::cout<<"For xml configuration, check example folder\n";
};

void print()
{
};

bool validate()
{
  SPDLOG_INFO("Validating configuration");
  if (!UTIL::fileExists(gps_file))
  {
    SPDLOG_CRITICAL("GPS file {} not found",gps_file);
    return false;
  };
  if (get_gps_format()<0) {
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

}
