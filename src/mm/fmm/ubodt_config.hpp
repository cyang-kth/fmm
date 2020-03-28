class UBODT_Config
{
public:
  UBODT_Config(int argc, char **argv);
  void load_xml(const std::string &file);
  void load_arg(int argc, char **argv);
  void print();
  static void print_help();
  bool validate();
  std::string network_file;
  std::string network_id;
  std::string network_source;
  std::string network_target;
  int binary_flag;
  double delta;
  std::string result_file;
  int log_level;
}; // UBODT_Config
