/**
 * Content
 * Precomputation of UBODT (Optimized)
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */
#include "../src/network_graph.hpp"
#include "../src/config.hpp"
#include <iostream>
#include <ctime>
using namespace std;
using namespace MM;

void run(int argc, char **argv){
  spdlog::set_pattern("[%s:%#] %v");
  if (argc<2) {
    std::cout<<"A configuration file is given in the example folder"<<endl;
    std::cout<<"Run `ubodt_gen_omp config.xml` or with arguments"<<endl;
    UBODT_Config::print_help();
    return;
  } else {
    if (argc==2){
      std::string first_arg(argv[1]);
      if (first_arg=="--help"||first_arg=="-h"){
        UBODT_Config::print_help();
        return;
      }
    }
    UBODT_Config config(argc,argv);
    if (!config.validate())
    {
      SPDLOG_CRITICAL("Validation fail, program stop");
      return;
    };
    config.print();
    std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();
    SPDLOG_INFO("Write UBODT to file {}",config.result_file);
    MM::Network network(config.network_file,
                        config.network_id,
                        config.network_source,
                        config.network_target);

    MM::NetworkGraph graph(&network);
    std::cout<<"Upperbound config (delta): "<<config.delta<<'\n';
    bool binary = (config.binary_flag==1);
    graph.precompute_ubodt_omp(config.result_file, config.delta, binary);
    std::chrono::steady_clock::time_point end =
      std::chrono::steady_clock::now();
    double time_spent =
      std::chrono::duration_cast<std::chrono::milliseconds>
        (end - begin).count() / 1000.;
    SPDLOG_INFO("Time takes {}",time_spent);
  }
};

int main(int argc, char* argv[])
{
  std::cout<<"------------ Fast map matching (FMM) ------------"<<endl;
  std::cout<<"------------     Author: Can Yang    ------------"<<endl;
  std::cout<<"------------   Version: 2020.01.31   ------------"<<endl;
  std::cout<<"------------Application: ubodt_gen_omp------------"<<endl;
  run(argc,argv);
  std::cout<<"------------    Program finished     ------------"<<endl;
  return 0;
};
