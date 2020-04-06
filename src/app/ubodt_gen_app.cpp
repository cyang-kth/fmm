/**
 * Fast map matching.
 * 
 * ubodg_gen command line program main function
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#include "mm/fmm/ubodt_gen_app.hpp"

using namespace FMM;
using namespace FMM::MM;

int main(int argc, char **argv){
  UBODTGenAppConfig config(argc,argv);
  if (config.help_specified) {
    UBODTGenAppConfig::print_help();
    return 0;
  }
  if (!config.validate()){
    return 0;
  }
  UBODTGenApp app(config);
  app.run();
  return 0;
};
