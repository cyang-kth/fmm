#include "mm/fmm/ubodt_gen_app.hpp"

using namespace MM;

int main(int argc, char **argv){
  UBODTGenAppConfig config(argc,argv);
  UBODTGenApp app(config);
  app.run();
  return 0;
};
