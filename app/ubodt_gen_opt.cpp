/**
 * Content
 * Precomputation of UBODT
 *      
 * @author: Can Yang
 * @version: 2017.11.11
 */
#include "../src/network_graph_opt.hpp"
#include "config.hpp"
#include <iostream>
#include <ctime>
using namespace std;
using namespace MM;
int main(int argc, char* argv[])
{
    std::cout<<"------------ Fast map matching (FMM) ------------"<<endl;
    std::cout<<"------------     Author: Can Yang    ------------"<<endl;
    std::cout<<"------------   Version: 2018.03.09   ------------"<<endl;
    std::cout<<"------------Application: ubodt_gen_opt------------"<<endl;
    if (argc<2) {
        std::cout<<"No configuration file supplied"<<endl;
        std::cout<<"A configuration file is given in the example folder"<<endl;
        std::cout<<"Run `ubodt_gen_opt config.xml`"<<endl;
    } else {
        clock_t begin_time = clock(); // program start time
        std::string configfile(argv[1]);
        UBODT_Config config(configfile);
        if (!config.validate())
        {
            std::cout<<"Invalid configuration file, program stop"<<endl;
            return 0;
        };
        std::cout<<"Write UBODT to file "<<config.network_file<<std::endl;
        MM::Network network(config.network_file,
            config.network_id,
            config.network_source,
            config.network_target);
        // MM::NetworkGraph graph(&network);
        MM::NetworkGraphOpt graph(&network);
        graph.precompute_ubodt(config.result_file,config.delta);
        clock_t end_time = clock(); // program end time
        // Unit is second
        double time_spent = (double)(end_time - begin_time) / CLOCKS_PER_SEC;
        std::cout<<"Time takes "<<time_spent<<std::endl;        
    }
    std::cout<<"------------    Program finished     ------------"<<endl;
    return 0;
};
