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
#include "../src/multilevel_debug.h"
#include "../src/config.hpp"

using namespace std;
using namespace MM;
using namespace MM::IO;
int main (int argc, char **argv)
{
    std::cout << "------------ Fast map matching (FMM) ------------" << endl;
    std::cout << "------------     Author: Can Yang    ------------" << endl;
    std::cout << "------------   Version: 2018.03.09   ------------" << endl;
    std::cout << "------------   Applicaton: fmm_omp   ------------" << endl;
    if (argc < 2)
    {
        std::cout << "No configuration file supplied" << endl;
        std::cout << "A configuration file is given in the example folder" << endl;
        std::cout << "Run `fmm config.xml`" << endl;
    } else {
        std::string configfile(argv[1]);
        FMM_Config config(configfile);
        if (!config.validate_mm())
        {
            std::cout << "Invalid configuration file, program stop" << endl;
            return 0;
        };
        config.print();

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        // std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() <<std::endl;
        // clock_t begin_time = clock(); // program start time
        Network network(config.network_file, config.network_id, config.network_source, config.network_target);
        network.build_rtree_index();
        int multiplier = network.get_node_count();
        if (multiplier==0) multiplier = 50000;
        UBODT *ubodt=nullptr;
        if (config.binary_flag==1){
            ubodt = read_ubodt_binary(config.ubodt_file,multiplier);
        } else {
            ubodt = read_ubodt_csv(config.ubodt_file,multiplier);
        }
        if (!config.delta_defined){
            config.delta = ubodt->get_delta();
            std::cout<<"    Delta inferred from ubodt as "<< config.delta <<'\n';
        }
        TrajectoryReader tr_reader(config.gps_file, config.gps_id);
        ResultConfig result_config = config.get_result_config();
        ResultWriter rw(config.result_file,&network,result_config);
        int progress = 0;
        int points_matched = 0;
        int total_points = 0;
        int num_trajectories = tr_reader.get_num_trajectories();
        int buffer_trajectories_size = 100000;
        int step_size = num_trajectories / 10;
        if (step_size < 10) step_size = 10;
        std::cout << "Start to map match trajectories with total number " << num_trajectories << '\n';
        // No geometry output
        while (tr_reader.has_next_feature()) {
            std::vector<Trajectory> trajectories =  tr_reader.read_next_N_trajectories(buffer_trajectories_size);
            int trajectories_fetched = trajectories.size();
            #pragma omp parallel for
            for (int i = 0; i < trajectories_fetched; ++i) {
                Trajectory &trajectory = trajectories[i];
                int points_in_tr = trajectory.geom->getNumPoints();
                DEBUG(1) std::cout << "\n=============================" << '\n';
                DEBUG(1) std::cout << "Process trips with id : " << trajectory.id << '\n';
                // Candidate search
                Traj_Candidates traj_candidates = network.search_tr_cs_knn(trajectory, config.k, config.radius,config.gps_error);
                TransitionGraph tg = TransitionGraph(&traj_candidates,trajectory.geom, ubodt,config.delta);
                // Optimal path inference
                O_Path *o_path_ptr = tg.viterbi(config.penalty_factor);
                T_Path *t_path_ptr = ubodt->construct_traversed_path(o_path_ptr);
                if (result_config.write_mgeom) {
                    LineString *m_geom = network.complete_path_to_geometry(o_path_ptr,&(t_path_ptr->cpath));
                    rw.write_result(trajectory.id,trajectory.geom,o_path_ptr,t_path_ptr,m_geom);
                    delete m_geom;
                } else {
                    rw.write_result(trajectory.id,trajectory.geom,o_path_ptr,t_path_ptr,nullptr);
                }
                // update statistics
                total_points+=points_in_tr;
                if (t_path_ptr!=nullptr) points_matched+=points_in_tr;
                DEBUG(1) std::cout<<"Free memory of o_path and c_path"<<'\n';
                ++progress;
                if (progress % step_size == 0) {
                    std::stringstream buf;
                    buf << "Progress " << progress << " / " << num_trajectories << '\n';
                    std::cout << buf.rdbuf();
                }
                delete o_path_ptr;
                delete t_path_ptr;
            }
        }
        std::cout << "\n=============================" << '\n';
        std::cout << "MM process finished" << '\n';
        // clock_t end_time = clock(); // program end time
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        // Unit is second
        // std::cout << "Time takes" <<  <<std::endl;
        double time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / 1000.;
        //double time_spent = (double)(end_time - begin_time) / CLOCKS_PER_SEC;
        std::cout << "Time takes " << time_spent << '\n';
        std::cout << "Finish map match total points " << total_points
                  << " and points matched " << points_matched << '\n';
        std::cout << "Matched percentage: " << points_matched / (double)total_points << '\n';
        std::cout << "Point match speed:" << points_matched / time_spent << "pt/s" << '\n';
        delete ubodt;
    }
    std::cout << "------------    Program finished     ------------" << endl;
    return 0;
};
