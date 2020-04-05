#include "mm/fmm/fmm_app.hpp"

using namespace FMM;

int main(int argc, char **argv){
  FMMAppConfig config(argc,argv);
  if (config.help_specified) {
    FMMAppConfig::print_help();
    return 0;
  }
  if (!config.validate()){
    return 0;
  }
  FMMApp app(config);
  app.run();
  return 0;
};
