namespace FMM {
namespace IO {
/**
 * Configuration class of stmatch command line program
 */
class H3MMAppConfig
{
public:
  /**
   * Constructor of the configuration from command line arguments.
   * The argument data are fetched from the main function directly.
   *
   * @param argc number of arguments
   * @param argv raw argument data
   *
   */
  H3MMAppConfig(int argc, char **argv){
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
  /**
   * Load configuration from arguments. The argument data
   * are fetched from the main function directly.
   * @param argc number of arguments
   * @param argv raw argument data
   */
  void load_arg(int argc, char **argv){
    SPDLOG_INFO("Start reading stmatch configuration from arguments");
    cxxopts::Options options("h3mm_config", "Configuration parser");
    // Register options
    GPSConfig::register_arg(options);
    H3MMConfig::register_arg(options);
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
    gps_config = GPSConfig::load_from_arg(result);
    result_config = CONFIG::ResultConfig::load_from_arg(result);
    h3mm_config = H3MMConfig::load_from_arg(result);
    log_level = result["log_level"].as<int>();
    step = result["step"].as<int>();
    use_omp = result.count("use_omp")>0;
    if (result.count("help")>0) {
      help_specified = true;
    }
    SPDLOG_INFO("Finish with reading stmatch arg configuration");
  };
  /**
   * Print help information
   */
  static void void print_help(){
    std::ostringstream oss;
    oss<<"stmatch argument lists:\n";
    NetworkConfig::register_help(oss);
    GPSConfig::register_help(oss);
    H3MMConfig::register_help(oss);
    ResultConfig::register_help(oss);
    oss<<"-l/--log_level (optional) <int>: log level (2)\n";
    oss<<"-s/--step (optional) <int>: progress report step (100)\n";
    oss<<"--use_omp: use OpenMP for multithreaded map matching\n";
    oss<<"-h/--help:print help information\n";
    std::cout<<oss.str();
  };

  void print() const {
    SPDLOG_INFO("----   Print configuration    ----")
    gps_config.print();
    result_config.print();
    h3mm_config.print();
    SPDLOG_INFO("Log level {}",UTIL::LOG_LEVESLS[log_level])
    SPDLOG_INFO("Step {}",step)
    SPDLOG_INFO("Use omp {}",(use_omp ? "true" : "false"))
    SPDLOG_INFO("---- Print configuration done ----")
  };

  bool validate() const {
    if (!gps_config.validate()) {
      return false;
    }
    if (!result_config.validate()) {
      return false;
    }
    if (!h3mm_config.validate()) {
      return false;
    }
    return true;
  };

  CONFIG::GPSConfig gps_config; /**< GPS data configuraiton */
  H3MMConfig h3mm_config; /**< Map matching configuraiton */
  bool use_omp = false; /**< If true, parallel map matching performed */
  bool help_specified = false; /**< Help is specified or not */
  int log_level = 2; /**< log level, 0-trace,1-debug,2-info,
                          3-warn,4-err,5-critical,6-off */
  int step = 100; /**< progress report step */
}; // H3MMAppConfig

class H3MMApp {
public:
  /**
   * Create stmatch command application from configuration
   * @param config configuration of the command line app
   */
  H3MMApp(const H3MMAppConfig &config) :
    config_(config){
  };
  /**
   * Run the stmatch program
   */
  void run();
private:
  const H3MMAppConfig &config_;
};

}
}
