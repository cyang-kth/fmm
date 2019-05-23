/**
 * 
 * Interface of FMM, to be used for swig 
 * 
 * @author: Can Yang, cyang@kth.se
 * @version: 2019.05.23
 */

#include "network.hpp"
#include "ubodt.hpp"
#include "transition_graph.hpp"
#include "config.hpp"
#include "writer.hpp"

class FMM {
public: 
    FMM(std::string &config_file):{
        config=MM::FMM_Config(config_file);
        config.print();
        network = new MM::Network(config.network_file,config.network_id,
                                  config.network_source,config.network_target);
        network->build_rtree_index();
        int multiplier = config.multiplier;
        int NHASH = config.nhash;
        ubodt = new UBODT(multiplier,NHASH);
    };
    ~FMM(){
        delete network;
        delete ubodt;
    };
private:
    UBODT *ubodt;
    Network *network;
    FMM_Config config;
    std::string match_trajectory(MM::Trajectory *trajectory){
        int points_in_tr = trajectory.geom->getNumPoints();
        // Candidate search
        Traj_Candidates traj_candidates = network->search_tr_cs_knn(trajectory,config.k,config.radius);
        TransitionGraph tg = MM::TransitionGraph(&traj_candidates,trajectory.geom,ubodt,config.delta);
        // Optimal path inference
        O_Path *o_path_ptr = tg.viterbi(config.penalty_factor);
        // Complete path construction as an array of indices of edges vector
        T_Path *t_path_ptr = ubodt->construct_traversed_path(o_path_ptr); 
        OGRLineString *m_geom = network.complete_path_to_geometry(o_path_ptr,&(t_path_ptr->cpath));
        std::string result = MM::IO::ResultWriter::mkString(
            o_path_ptr,t_path_ptr,m_geom
        );
        delete o_path_ptr;
        delete t_path_ptr;
        return result;
    };
};
