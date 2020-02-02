/**
 * Content
 * Parallel map matching implemented by OpenMP
 *
 *
 *
 * @author: Can Yang
 * @version: 2018.04.24
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <string>
#include <ctime>
#include "../src/network.hpp"
#include "../src/ubodt.hpp"
#include "../src/transition_graph.hpp"
#include "../src/gps.hpp"
#include "../src/reader.hpp"
#include "../src/writer.hpp"
#include "../src/debug.h"
#include "../src/config.hpp"

using namespace std;
using namespace MM;
using namespace MM::IO;

void run(int argc, char **argv)
{
  spdlog::set_pattern("[%s:%#] %v");
  if (argc < 2)
  {
    std::cout << "No configuration file supplied" << endl;
    std::cout << "A configuration file is given in the example folder" << endl;
    FMM_Config::print_help();
    return;
  } else {
    if (argc==2){
      std::string first_arg(argv[1]);
      if (first_arg=="--help"||first_arg=="-h"){
        FMM_Config::print_help();
        return;
      }
    }
    FMM_Config config(argc,argv);
    if (!config.validate_mm())
    {
      std::cout << "Invalid configuration file, program stop" << endl;
      return;
    };
    config.print();
    spdlog::set_level((spdlog::level::level_enum) config.log_level);
    std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();
    // clock_t begin_time = clock(); // program start time
    Network network(config.network_file, config.network_id,
                    config.network_source, config.network_target);
    network.build_rtree_index();
    int multiplier = network.get_node_count();
    if (multiplier==0) multiplier = 50000;
    UBODT *ubodt=nullptr;
    if (config.binary_flag==1) {
      ubodt = read_ubodt_binary(config.ubodt_file,multiplier);
    } else {
      ubodt = read_ubodt_csv(config.ubodt_file,multiplier);
    }
    if (!config.delta_defined) {
      config.delta = ubodt->get_delta();
      SPDLOG_INFO("Delta inferred from ubodt as {}",config.delta);
    }
    GPSReader gps_reader(config);
    ResultConfig result_config = config.get_result_config();
    ResultWriter rw(config.result_file,network,result_config);
    int progress = 0;
    int points_matched = 0;
    int total_points = 0;
    int buffer_trajectories_size = 100000;
    int num_trajectories = gps_reader.get_trajectory_number();
    int step_size = 1;
    if (num_trajectories<0) {
      step_size = 1000;
      SPDLOG_INFO("Progress report step {}",step_size);
    } else {
      step_size = num_trajectories /10;
      if (step_size<10) step_size=10;
      SPDLOG_INFO("Total trajectory number {}",num_trajectories);
      SPDLOG_INFO("Progress report step {}",step_size);
    }
    // No geometry output
    while (gps_reader.has_next_trajectory()) {
      std::vector<Trajectory> trajectories =
        gps_reader.read_next_N_trajectories(buffer_trajectories_size);
      int trajectories_fetched = trajectories.size();
            #pragma omp parallel for
      for (int i = 0; i < trajectories_fetched; ++i) {
        Trajectory &trajectory = trajectories[i];
        int points_in_tr = trajectory.geom.getNumPoints();
        // Candidate search
        Traj_Candidates traj_candidates = network.search_tr_cs_knn(
          trajectory, config.k, config.radius,config.gps_error);
        TransitionGraph tg = TransitionGraph(
          &traj_candidates,trajectory.geom,*ubodt,config.delta);
        // Optimal path inference
        O_Path o_path = tg.viterbi(config.penalty_factor);
        T_Path t_path = ubodt->construct_traversed_path(o_path,network);
        LineString m_geom;
        if (result_config.write_mgeom) {
          m_geom = network.complete_path_to_geometry(o_path,t_path.cpath);
        }
        rw.write_result(trajectory.id,trajectory.geom,o_path,t_path,m_geom);
        // update statistics
        #pragma omp critical
        total_points+=points_in_tr;
        if (!t_path.cpath.empty()) points_matched+=points_in_tr;
        ++progress;
        if (progress % step_size == 0) {
          std::stringstream buf;
          buf << "Progress " << progress << '\n';
          std::cout << buf.rdbuf();
        }
      }
    }
    SPDLOG_INFO("Progress {}",progress);
    std::chrono::steady_clock::time_point end =
      std::chrono::steady_clock::now();
    double time_spent =
      std::chrono::duration_cast<std::chrono::milliseconds>
        (end - begin).count() / 1000.;
    //double time_spent = (double)(end_time - begin_time) / CLOCKS_PER_SEC;
    SPDLOG_INFO("Time takes {}",time_spent);
    SPDLOG_INFO("Total points {} matched {}",total_points,points_matched);
    SPDLOG_INFO("Matched percentage: ",points_matched/(double)total_points);
    SPDLOG_INFO("Point match speed: {} pts",points_matched/time_spent);
    delete ubodt;
  }
};

int main(int argc, char **argv){
  std::cout<<"------------ Fast map matching (FMM) ------------"<<endl;
  std::cout<<"------------     Author: Can Yang    ------------"<<endl;
  std::cout<<"------------   Version: 2020.01.31   ------------"<<endl;
  std::cout<<"------------   Applicaton: fmm_omp   ------------"<<endl;
  run(argc,argv);
  std::cout<<"------------    Program finished     ------------"<<endl;
  return 0;
};
