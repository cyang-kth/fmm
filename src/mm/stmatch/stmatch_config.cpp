#include "mm/stmatch/stmatch_config.hpp"

namespace MM {

std::string STMATCHConfig::to_string() const {
  std::stringstream ss;
  ss<<"k: "<< k << "\n";
  ss<<"radius: "<< radius << "\n";
  ss<<"gps_error: "<< gps_error << "\n";
  ss<<"vmax: "<< vmax << "\n";
  ss<<"factor: "<< factor << "\n";
  return ss.str();
};

STMATCHConfig STMATCHConfig::load_from_xml(
  const boost::property_tree::ptree &xml_data){
  int k = tree.get("fmm_config.parameters.k", 8);
  double radius = xml_data.get("fmm_config.parameters.r", 300.0);
  double gps_error = xml_data.get("fmm_config.parameters.gps_error", 50.0);
  double vmax = xml_data.get("fmm_config.parameters.vmax", 80.0);;
  double factor = xml_data.get("fmm_config.parameters.factor", 1.5);;
  return STMATCHConfig{k,radius,gps_error,vmax,factor};
};


STMATCHConfig STMATCHConfig::load_from_arg(
  const cxxopts::ParseResult &arg_data){
  int k = arg_data["candidates"].as<int>();
  double radius = arg_data["radius"].as<double>();
  double gps_error = arg_data["error"].as<double>();
  double vmax = arg_data["vmax"].as<double>();
  double factor = arg_data["factor"].as<double>();
  return STMATCHConfig{k,radius,gps_error,vmax,factor};
};

STMATCHAppConfig::STMATCHAppConfig(int argc, char **argv){
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

void STMATCHAppConfig::load_xml(const std::string &file){
  std::cout<<"Start with reading stmatch xml configuration "<<file<<"\n";
  // Create empty property tree object
  boost::property_tree::ptree tree;
  boost::property_tree::read_xml(file, tree);
  // UBODT
  ubodt_file = tree.get<std::string>("mm_config.input.ubodt.file");

  network_config = NetworkConfig::load_from_xml();
  gps_config = GPSConfig::load_from_xml();
  result_config = ResultConfig::load_from_xml();
  stmatch_config = STMATCHAppConfig::load_from_xml();

  log_level = tree.get("mm_config.other.log_level",2);
  std::cout<<"Finish with reading stmatch xml configuration.\n";
};

void STMATCHAppConfig::load_arg(int argc, char **argv){
  std::cout<<"Start reading stmatch configuration from arguments\n";
  cxxopts::Options options("mm_config", "Configuration parser of fmm");
  options.add_options()
    ("network","Network file name", cxxopts::value<std::string>())
    ("network_id","Network id name",
    cxxopts::value<std::string>()->default_value("id"))
    ("source","Network source name",
    cxxopts::value<std::string>()->default_value("source"))
    ("target","Network target name",
    cxxopts::value<std::string>()->default_value("target"))
    ("gps","GPS file name", cxxopts::value<std::string>())
    ("gps_id","GPS file id",
    cxxopts::value<std::string>()->default_value("id"))
    ("gps_geom","GPS file geom column name",
    cxxopts::value<std::string>()->default_value("geom"))
    ("k,candidates","Number of candidates",
    cxxopts::value<int>()->default_value("8"))
    ("r,radius","Search radius",
    cxxopts::value<double>()->default_value("300.0"))
    ("e,error","GPS error",
    cxxopts::value<double>()->default_value("50.0"))
    ("vmax","Maximum speed",
    cxxopts::value<double>()->default_value("80.0"))
    ("factor","Scale factor",
    cxxopts::value<double>()->default_value("1.5"))
    ("o,output","Output file name", cxxopts::value<std::string>())
    ("m,output_fields","Output fields", cxxopts::value<std::string>())
    ("l,log_level","Log level",cxxopts::value<int>()->default_value("2"));

  auto result = options.parse(argc, argv);
  ubodt_file = result["ubodt"].as<std::string>();
  network_config = NetworkConfig::load_from_arg(result);
  gps_config = GPSConfig::load_from_arg(result);
  result_config = ResultConfig::load_from_arg(result);
  stmatch_config = STMATCHAppConfig::load_from_arg(result);

  std::cout<<"Finish with reading stmatch arg configuration\n";
};

void STMATCHAppConfig::print(){
  std::cout<<"---Network Config---\n"<< network_config.to_string() << "\n";
  std::cout<<"---GPS Config---\n"<< gps_config.to_string() << "\n";
  std::cout<<"---Result Config---\n"<< result_config.to_string() << "\n";
  std::cout<<"---STMATCH Config---\n"<< stmatch_config.to_string() << "\n";
  std::cout<<"---Others---\n"<< "log_level" << log_level << "\n";
};

void STMATCHAppConfig::print_help(){
  std::cout<<"stmatch argument lists:\n";
  std::cout<<"--network (required) <string>: Network file name\n";
  std::cout<<"--network_id (optional) <string>: Network id name (id)\n";
  std::cout<<"--source (optional) <string>: Network source name (source)\n";
  std::cout<<"--target (optional) <string>: Network target name (target)\n";
  std::cout<<"--gps (required) <string>: GPS file name\n";
  std::cout<<"--gps_id (optional) <string>: GPS id name (id)\n";
  std::cout<<"--gps_geom (optional) <string>: GPS geometry name (geom)\n";
  std::cout<<"-k/--candidates (optional) <int>: number of candidates (8)\n";
  std::cout<<"-r/--radius (optional) <double>: search radius (unit meter) (300)\n";
  std::cout<<"-e/--error (optional) <double>: GPS error (unit meter) (50)\n";
  std::cout<<"-f/--factor (optional) <double>: scale factor (1.5)\n";
  std::cout<<"-v/--vmax (optional) <double>: Maximum speed (unit km/h) (80)\n";
  std::cout<<"-o/--output (required) <string>: Output file name\n";
  std::cout<<"-m/--output_fields (optional) <string>: Output fields\n";
  std::cout<<"  opath,cpath,tpath,ogeom,mgeom,pgeom,\n";
  std::cout<<"  offset,error,spdist,tp,ep,length,all\n";
  std::cout<<"--log_level (optional) <int>: log level (2)\n";
  std::cout<<"For xml configuration, check example folder\n";
};

};
