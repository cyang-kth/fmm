#include <fmm-api.hpp>
#include <iostream>
using namespace FMM;
using namespace FMM::NETWORK;
using namespace FMM::CORE;
using namespace FMM::MM;

int main(int argc, char **argv){
  std::cout<<"Use FMM in an external project"<<std::endl;
  if (argc!=2){
    std::cout<<"Incorrect number of arguments"<<std::endl;
  } else {
    std::string filename(argv[1]);
    std::cout<<"Network file"<<filename<<std::endl;
    Network network(filename,"id","source","target");
    std::cout<<"Network nodes "<<network.get_node_count()<<std::endl;
    std::cout<<"Network edges "<<network.get_edge_count()<<std::endl;
  }
};
