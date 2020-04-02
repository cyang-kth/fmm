#include "mm/stmatch/stmatch_app.hpp"

using namespace MM;

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
