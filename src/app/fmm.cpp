/**
 * Fast map matching.
 *
 * fmm command line program main function
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#include "mm/fmm/fmm_app.hpp"

using namespace FMM;
using namespace FMM::MM;
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
