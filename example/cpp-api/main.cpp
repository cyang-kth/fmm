#include <fmm/fmm-api.hpp>
#include <iostream>
using namespace FMM;
using namespace FMM::NETWORK;
using namespace FMM::CORE;
using namespace FMM::MM;

int main(int argc, char **argv){
  SPDLOG_INFO("Use FMM in an external project");
  if (argc!=2){
    SPDLOG_ERROR("Incorrect number of arguments");
  } else {
    std::string filename(argv[1]);
    SPDLOG_INFO("Network file {}",filename);
    Network network(filename,"id","source","target");
    SPDLOG_INFO("Network node count {}",network.get_node_count());
    SPDLOG_INFO("Network edge count {}",network.get_edge_count());
  }
};
