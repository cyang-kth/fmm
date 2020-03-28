#include "mm/stmatch/stmatch_config.hpp"

namespace MM {

STMATCHConfig::STMATCHConfig(int argc, char **argv){
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

void STMATCHConfig::load_xml(const std::string &file){
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

void STMATCHConfig::load_arg(int argc, char **argv){
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

};
