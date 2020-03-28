#include "mm/stmatch/stmatch_app.hpp"

int main(int argc, char **argv){
  STMATCHAppConfig config(argc,argv);
  STMATCHApp app(config);
  app.run();
  return 0;
};
