/**
 * Content
 * FMM application (by stream)
 *      
 * @author: Can Yang
 * @version: 2017.11.11
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
#include "config.hpp"
using namespace std;
using namespace MM;
using namespace MM::IO;
int main (int argc, char **argv)
{
    std::cout<<"------------ Fast map matching (FMM) ------------"<<endl;
    std::cout<<"------------     Author: Can Yang    ------------"<<endl;
    std::cout<<"------------   Version: 2018.03.09   ------------"<<endl;
    std::cout<<"------------     Applicaton: fmm     ------------"<<endl;
    if (argc<2)
    {
        std::cout<<"No configuration file supplied"<<endl;
        std::cout<<"A configuration file is given in the example folder"<<endl;
        std::cout<<"Run `fmm config.xml`"<<endl;
    } else {
        std::string configfile(argv[1]);
        FMM_Config config(configfile);
        if (!config.validate_mm())
        {
            std::cout<<"Invalid configuration file, program stop"<<endl;
            return 0;
        };
        config.print();
        clock_t begin_time = clock(); // program start time
        GPS_ERROR = config.gps_error; // Default value is 50 meter
        Network network(config.network_file,config.network_id,config.network_source,config.network_target);
        network.build_rtree_index();
        int multiplier = config.multiplier;
        int NHASH = config.nhash;
        UBODT ubodt(multiplier,NHASH);
        if (config.binary_flag==1){
            ubodt.read_binary(config.ubodt_file);
        } else {
            ubodt.read_csv(config.ubodt_file);            
        }
        TrajectoryReader tr_reader(config.gps_file,config.gps_id);
        ResultWriter rw(config.result_file,&network);
        int progress=0;
        int points_matched=0;
        int total_points=0;
        int num_trajectories = tr_reader.get_num_trajectories();
        int step_size = num_trajectories/20;
        if (step_size<10) step_size=10;
        std::cout<<"Start to map match trajectories with total number "<< num_trajectories <<'\n';
        if (config.mode == 0)
        {
            rw.write_header("id;o_path;c_path");
            while (tr_reader.has_next_feature())
            {
                Trajectory trajectory = tr_reader.read_next_trajectory();
                int points_in_tr = trajectory.geom->getNumPoints();
                if (progress%step_size==0) std::cout<<"Progress "<<progress << " / " << num_trajectories <<'\n';
                DEBUG(1) std::cout<<"\n============================="<<'\n';
                DEBUG(1) std::cout<<"Process trips with id : "<<trajectory.id<<'\n';
                Traj_Candidates traj_candidates = network.search_tr_cs_knn(trajectory,config.k,config.radius);
                TransitionGraph tg = TransitionGraph(&traj_candidates,trajectory.geom,&ubodt);
                // Optimal path inference
                O_Path *o_path_ptr = tg.viterbi(config.penalty_factor);
                // Complete path construction as an array of indices of edges vector
                C_Path *c_path_ptr = ubodt.construct_complete_path(o_path_ptr);
                // Write result
                rw.write_opath_cpath(trajectory.id,o_path_ptr,c_path_ptr);
                // update statistics
                total_points+=points_in_tr;
                if (c_path_ptr!=nullptr) points_matched+=points_in_tr;
                DEBUG(1) std::cout<<"============================="<<'\n';
                ++progress;
                delete o_path_ptr;
                delete c_path_ptr;
            }
        } else if (config.mode == 1){
            // WKB
            rw.write_header("id;o_path;c_path;m_geom");
            while (tr_reader.has_next_feature())
            {
                Trajectory trajectory = tr_reader.read_next_trajectory();
                int points_in_tr = trajectory.geom->getNumPoints();
                if (progress%step_size==0) std::cout<<"Progress "<<progress << " / " << num_trajectories <<'\n';
                DEBUG(1) std::cout<<"\n============================="<<'\n';
                DEBUG(1) std::cout<<"Process trips with id : "<<trajectory.id<<'\n';
                // Candidate search
                Traj_Candidates traj_candidates = network.search_tr_cs_knn(trajectory,config.k,config.radius);
                TransitionGraph tg = TransitionGraph(&traj_candidates,trajectory.geom,&ubodt);
                // Optimal path inference
                O_Path *o_path_ptr = tg.viterbi(config.penalty_factor);
                // Complete path construction as an array of indices of edges vector
                C_Path *c_path_ptr = ubodt.construct_complete_path(o_path_ptr);
                // Write result
                OGRLineString *m_geom = network.complete_path_to_geometry(o_path_ptr,c_path_ptr);
                rw.write_map_matched_result_wkb(trajectory.id,o_path_ptr,c_path_ptr,m_geom);
                // update statistics
                total_points+=points_in_tr;
                if (c_path_ptr!=nullptr) points_matched+=points_in_tr;
                DEBUG(1) std::cout<<"============================="<<'\n';
                ++progress;
                delete o_path_ptr;
                delete c_path_ptr;
                delete m_geom;
                //}
            }
        } else if (config.mode == 2){
            // WKT
            rw.write_header("id;o_path;c_path;m_geom");
            while (tr_reader.has_next_feature())
            {
                DEBUG(2) std::cout<<"Start of the loop"<<'\n';
                Trajectory trajectory = tr_reader.read_next_trajectory();
                int points_in_tr = trajectory.geom->getNumPoints();
                if (progress%step_size==0) std::cout<<"Progress "<<progress << " / " << num_trajectories <<'\n';
                DEBUG(1) std::cout<<"\n============================="<<'\n';
                DEBUG(1) std::cout<<"Process trips with id : "<<trajectory.id<<'\n';
                // Candidate search
                Traj_Candidates traj_candidates = network.search_tr_cs_knn(trajectory,config.k,config.radius);
                TransitionGraph tg = TransitionGraph(&traj_candidates,trajectory.geom,&ubodt);
                // Optimal path inference
                O_Path *o_path_ptr = tg.viterbi(config.penalty_factor);
                // Complete path construction as an array of indices of edges vector
                C_Path *c_path_ptr = ubodt.construct_complete_path(o_path_ptr);
                // Write result
                OGRLineString *m_geom = network.complete_path_to_geometry(o_path_ptr,c_path_ptr);
                rw.write_map_matched_result_wkt(trajectory.id,o_path_ptr,c_path_ptr,m_geom);
                // update statistics
                total_points+=points_in_tr;
                if (c_path_ptr!=nullptr) points_matched+=points_in_tr;
                DEBUG(1) std::cout<<"Free memory of o_path and c_path"<<'\n';
                ++progress;
                delete o_path_ptr;
                delete c_path_ptr;
                delete m_geom;
                DEBUG(1) std::cout<<"============================="<<'\n';
            }
        } else if (config.mode == 3){
            // Offset
            rw.write_header("id;o_path;offset;c_path");
            while (tr_reader.has_next_feature())
            {
                Trajectory trajectory = tr_reader.read_next_trajectory();
                int points_in_tr = trajectory.geom->getNumPoints();
                if (progress%step_size==0) std::cout<<"Progress "<<progress << " / " << num_trajectories <<'\n';
                DEBUG(1) std::cout<<"\n============================="<<'\n';
                DEBUG(1) std::cout<<"Process trips with id : "<<trajectory.id<<'\n';
                Traj_Candidates traj_candidates = network.search_tr_cs_knn(trajectory,config.k,config.radius);
                TransitionGraph tg = TransitionGraph(&traj_candidates,trajectory.geom,&ubodt);
                // Optimal path inference
                O_Path *o_path_ptr = tg.viterbi(config.penalty_factor);
                // Complete path construction as an array of indices of edges vector
                C_Path *c_path_ptr = ubodt.construct_complete_path(o_path_ptr);
                // Write result
                rw.write_opath_cpath_offset(trajectory.id,o_path_ptr,c_path_ptr);
                // update statistics
                total_points+=points_in_tr;
                if (c_path_ptr!=nullptr) points_matched+=points_in_tr;
                DEBUG(1) std::cout<<"============================="<<'\n';
                ++progress;
                delete o_path_ptr;
                delete c_path_ptr;
            }
        } else {
            return 0;
        };
        std::cout<<"\n============================="<<'\n';
        std::cout<<"MM process finished"<<'\n';
        clock_t end_time = clock(); // program end time
        // Unit is second
        double time_spent = (double)(end_time - begin_time) / CLOCKS_PER_SEC;
        std::cout<<"Time takes "<<time_spent<<'\n';
        std::cout<<"Finish map match total points "<<total_points
                 <<" and points matched "<<points_matched<<'\n';
        std::cout<<"Matched percentage: "<<points_matched/(double)total_points<<'\n';
        std::cout<<"Point match speed:"<<points_matched/time_spent<<"pt/s"<<'\n';
    }
    std::cout<<"------------    Program finished     ------------"<<endl;
    return 0;
};
