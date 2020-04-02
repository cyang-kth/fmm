#include "mm/fmm/ubodt_gen_app_config.hpp"
#include "util/util.hpp"
#include "util/debug.hpp"


namespace MM {
/**
 * Configuration class for UBODT
 */

UBODTGenAppConfig::UBODTGenAppConfig(int argc, char **argv) {
  if (argc == 2) {
    std::string configfile(argv[1]);
    load_xml(configfile);
  } else {
    load_arg(argc, argv);
  }
  std::cout << "Set log level as " << LOG_LEVESLS[log_level] << "\n";
  spdlog::set_level((spdlog::level::level_enum) log_level);
  spdlog::set_pattern("[%l][%s:%-3#] %v");
};

void UBODTGenAppConfig::load_xml(const std::string &file) {
  // Create empty property tree object
  boost::property_tree::ptree tree;
  std::cout << "Read configuration from xml file: " << file << '\n';
  boost::property_tree::read_xml(file, tree);
  network_config = NetworkConfig::load_from_xml(tree);
  delta = tree.get("config.parameters.delta", 3000.0);
  result_file = tree.get<std::string>("config.output.file");
  // 0-trace,1-debug,2-info,3-warn,4-err,5-critical,6-off
  log_level = tree.get("config.other.log_level", 2);
  use_omp = !(!tree.get_child_optional("config.other.use_omp"));
};

void UBODTGenAppConfig::load_arg(int argc, char **argv) {
  std::cout << "Start reading ubodt configuration from arguments\n";
  cxxopts::Options options("config",
                           "Configuration parser of ubodt_gen");
  options.add_options()
      ("network", "Network file name", cxxopts::value<std::string>())
      ("network_id", "Network id name",
       cxxopts::value<std::string>()->default_value("id"))
      ("source", "Network source name",
       cxxopts::value<std::string>()->default_value("source"))
      ("target", "Network target name",
       cxxopts::value<std::string>()->default_value("target"))
      ("delta", "Upperbound distance",
       cxxopts::value<double>()->default_value("3000.0"))
      ("o,output", "Output file name", cxxopts::value<std::string>())
      ("l,log_level", "Log level", cxxopts::value<int>()->default_value("2"));

  auto result = options.parse(argc, argv);
  // Output
  result_file = result["output"].as<std::string>();
  network_config =  NetworkConfig::load_from_arg(result);
  log_level = result["log_level"].as<int>();
  delta = result["delta"].as<double>();
  use_omp = result.count("use_omp")>0;
  std::cout<<"Finish with reading ubodt arg configuration\n";
};

void UBODTGenAppConfig::print() const {
  std::cout << "UBODT Configuration: \n";
  std::cout <<"---Network Config---\n"<< network_config.to_string() << "\n";
  std::cout <<"---UBODT Config---\n";
  std::cout << "  delta: " << delta << '\n';
  std::cout << "  Output file:" << result_file << '\n';
  std::cout << "  log_level:" << LOG_LEVESLS[log_level] << '\n';
  std::cout << "use_omp" << (use_omp?"true":"false") << "\n";
};

void UBODTGenAppConfig::print_help() {
  std::cout << "ubodt_gen argument lists:\n";
  std::cout << "--network (required) <string>: Network file name\n";
  std::cout << "--output (required) <string>: Output file name\n";
  std::cout << "--id (optional) <string>: Network id name (id)\n";
  std::cout << "--source (optional) <string>: Network source name (source)\n";
  std::cout << "--target (optional) <string>: Network target name (target)\n";
  std::cout << "--delta (optional) <double>: upperbound (3000.0)\n";
  std::cout << "--log_level (optional) <int>: log level (2)\n";
  std::cout << "--use_omp: use OpenMP or not\n";
  std::cout << "For xml configuration, check example folder\n";
};

bool UBODTGenAppConfig::validate() const {
  SPDLOG_INFO("Validating configuration for UBODT construction");
  if (!network_config.validate()) {
    return false;
  }
  if (UTIL::file_exists(result_file)) {
    SPDLOG_WARN("Overwrite result file {}", result_file);
  }
  std::string output_folder = UTIL::get_file_directory(result_file);
  if (!UTIL::folder_exist(output_folder)) {
    SPDLOG_CRITICAL("Output folder {} not exists", output_folder);
    return false;
  }
  if (log_level < 0 || log_level > LOG_LEVESLS.size()) {
    SPDLOG_CRITICAL("Invalid log_level {}, which should be 0 - 6", log_level);
    SPDLOG_INFO("0-trace,1-debug,2-info,3-warn,4-err,5-critical,6-off");
    return false;
  }
  if (delta <= 0) {
    SPDLOG_CRITICAL("Delta {} should be positive");
    return false;
  }
  SPDLOG_INFO("Validating done.");
  return true;
};

bool UBODTGenAppConfig::is_binary_output() const {
  if (UTIL::check_file_extension(result_file,"bin")){
    return true;
  }
  return false;
}

}