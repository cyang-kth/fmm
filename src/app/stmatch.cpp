/**
 * Fast map matching.
 *
 * stmatch command line program main function
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#include "mm/stmatch/stmatch_app.hpp"

using namespace FMM;
using namespace FMM::MM;

int main(int argc, char **argv){
  STMATCHAppConfig config(argc,argv);
  if (config.help_specified) {
    STMATCHAppConfig::print_help();
    return 0;
  }
  if (!config.validate()){
    return 0;
  }
  STMATCHApp app(config);
  app.run();
  return 0;
};
