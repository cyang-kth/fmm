class UBODT_Config
{
public:
  UBODT_Config(int argc, char **argv);
  void load_xml(const std::string &file);
  void load_arg(int argc, char **argv);
  void print() const;
  bool validate() const;
  static void print_help();
  NetworkConfig network_config;
  double delta;
  std::string result_file;
  int log_level;
}; // UBODT_Config
