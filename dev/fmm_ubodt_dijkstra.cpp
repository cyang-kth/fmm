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
#include "ubodt_routing.hpp"
#include "transition_graph_routing.hpp"
#include "network_graph.hpp"
#include "../src/gps.hpp"
#include "../src/reader.hpp"
#include "../src/writer.hpp"
#include "../src/multilevel_debug.h"
#include "../app/config.hpp"
using namespace std;
using namespace MM;
using namespace MM::IO;

/**
 *  This application integrates a graph into the map matching process
 *  when the sp_dist is not found in the UBODT, the graph is used to
 *  compute the SP distance so that memory can also be used.
 */

int main (int argc, char **argv)
{
    if (argc<2)
    {
        cout<<"No configuration file supplied"<<endl;
        return 0;
    }
    std::string configfile(argv[1]);
    FMM_Config config(configfile);
    if (!config.validate_mm())
    {
        std::cout<<"Invalid configuration file, program stop"<<endl;
        return 0;
    };
    config.print_config_mm();
    clock_t begin_time = clock(); // program start time
    GPS_ERROR = config.gps_error; // Default value is 50 meter
    Network network(config.network_file,config.network_id,config.network_source,config.network_target);
    network.build_rtree_index();
    NetworkGraph graph(&network); // construct a graph
    int multiplier = config.multiplier;
    int NHASH = config.nhash;
    UBODT_Routing ubodt(multiplier,NHASH,&graph,config.delta);
    ubodt.read_csv(config.ubodt_file);
    TrajectoryReader tr_reader(config.gps_file,config.gps_id);
    ResultWriter rw(config.result_file,&network);
    int progress=0;
    int points_matched=0;
    int total_points=0;
    // Check the mode for geometry output
    // write_result_funct_type result_write;
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
            // Candidate search
            Traj_Candidates traj_candidates = network.search_tr_cs_knn(trajectory,config.k,config.radius);
            TransitionGraphRouting tg(&traj_candidates,trajectory.geom,&ubodt);
            // Optimal path inference
            O_Path *o_path_ptr = tg.viterbi(config.penalty_factor); // Default is ubodt
            // Complete path construction as an array of indices of edges vector
            C_Path *c_path_ptr = ubodt.construct_complete_path_norouting(o_path_ptr);
            // If it is a true large gap
            if (o_path_ptr!=nullptr && c_path_ptr==nullptr){
                o_path_ptr = tg.viterbi(config.penalty_factor,true); // Run viterbi with ubodt+dijkstra
                c_path_ptr = ubodt.construct_complete_path_routing(o_path_ptr);
            }
            rw.write_opath_cpath(trajectory.id,o_path_ptr,c_path_ptr);
            // update statistics
            total_points+=points_in_tr;
            if (c_path_ptr!=nullptr) points_matched+=points_in_tr;
            DEBUG(1) std::cout<<"============================="<<'\n';
            ++progress;
            delete o_path_ptr;
            delete c_path_ptr;
            //}
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
            TransitionGraphRouting tg(&traj_candidates,trajectory.geom,&ubodt);
            // Optimal path inference
            O_Path *o_path_ptr = tg.viterbi(config.penalty_factor); // No routing query
            // Complete path construction as an array of indices of edges vector
            C_Path *c_path_ptr = ubodt.construct_complete_path_norouting(o_path_ptr);
            // Write result
            if (o_path_ptr!=nullptr && c_path_ptr==nullptr){
                o_path_ptr = tg.viterbi(config.penalty_factor,true);
                c_path_ptr = ubodt.construct_complete_path_routing(o_path_ptr);
            }
            // Write result
            OGRLineString *m_geom = network.complete_path_to_geometry(c_path_ptr);
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
    } else {
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
            TransitionGraphRouting tg(&traj_candidates,trajectory.geom,&ubodt);
            // Optimal path inference
            O_Path *o_path_ptr = tg.viterbi(config.penalty_factor);
            // Complete path construction as an array of indices of edges vector
            C_Path *c_path_ptr = ubodt.construct_complete_path_norouting(o_path_ptr);
            // Write result
            if (o_path_ptr!=nullptr && c_path_ptr==nullptr){
                o_path_ptr = tg.viterbi(config.penalty_factor,true);
                c_path_ptr = ubodt.construct_complete_path_routing(o_path_ptr);
            }
            OGRLineString *m_geom = network.complete_path_to_geometry(c_path_ptr);
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
    std::cout<<"Routing distance called :"<< ubodt.get_routing_distance_times() <<'\n';
    std::cout<<"Routing path called :"<< ubodt.get_routing_path_times() <<'\n';
#ifdef ROUTING_RECORD_DEBUG
    ubodt.save_routing_statistics_to_file("routing_stat.txt");
    ubodt.save_routing_path_to_file("routing_stat_path.txt");
#endif // ROUTING_RECORD_DEBUG
    std::cout<<"Point match speed:"<<points_matched/time_spent<<"pt/s"<<'\n';
    return 0;
};
