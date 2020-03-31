#include "mm/fmm/fmm_app.hpp"

using namespace MM;

int main(int argc, char **argv){
  FMMAppConfig config(argc,argv);
  FMMApp app(config);
  app.run();
  return 0;
};
