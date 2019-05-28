/**
 *
 * Interface of FMM, to be used for swig
 *
 * @author: Can Yang, cyang@kth.se
 * @version: 2019.05.23
 */

#ifndef MM_MAPMATCHER_HPP
#define MM_MAPMATCHER_HPP

#include "../src/geometry_type.hpp"
#include "../src/network.hpp"
#include "../src/ubodt.hpp"
#include "../src/transition_graph.hpp"
#include "../src/config.hpp"
#include "../src/writer.hpp"

namespace MM{

class MapMatcherConfig{
public:
    MapMatcherConfig(const std::string &config_file){
        std::cout << "Start reading FMM configuration \n";
        // Create empty property tree object
        boost::property_tree::ptree tree;
        boost::property_tree::read_xml(config_file, tree);
        // Parse the XML into the property tree.
        // Without default value, the throwing version of get to find attribute.
        // If the path cannot be resolved, an exception is thrown.
        // UBODT
        ubodt_file = tree.get<std::string>("fmm_config.input.ubodt.file");
        multiplier = tree.get("fmm_config.input.ubodt.multiplier", 37); // multiplier=30000
        nhash = tree.get("fmm_config.input.ubodt.nhash", 127); // 5178049
        // Check if delta is specified or not
        if (!tree.get_optional<bool>("fmm_config.input.ubodt.delta").is_initialized()){
            delta_defined = false;
            delta = 0.0;
        } else {
            delta = tree.get("fmm_config.input.ubodt.delta",5000.0); //
            delta_defined = true;
        }
        binary_flag = MM::get_file_extension(ubodt_file);
        // Network
        network_file = tree.get<std::string>("fmm_config.input.network.file");
        network_id = tree.get("fmm_config.input.network.id", "id");
        network_source = tree.get("fmm_config.input.network.source", "source");
        network_target = tree.get("fmm_config.input.network.target", "target");

        // Other parameters
        k = tree.get("fmm_config.parameters.k", 8);
        radius = tree.get("fmm_config.parameters.r", 300.0);
        gps_error = tree.get("fmm_config.parameters.gps_error", 50.0);
        std::cout << "Finish with reading FMM configuration \n";

        std::cout << "------------------------------------------" << '\n';
        std::cout << "Configuration parameters for map matching application: " << '\n';
        std::cout << "Network_file: " << network_file << '\n';;
        std::cout << "Network id: " << network_id << '\n';
        std::cout << "Network source: " << network_source << '\n';
        std::cout << "Network target: " << network_target << '\n';
        std::cout << "ubodt_file: " << ubodt_file << '\n';
        std::cout << "multiplier: " << multiplier << '\n';
        std::cout << "nhash: " << nhash << '\n';
        if (delta_defined) {
            std::cout << "delta: " << delta << '\n';
        } else {
            std::cout << "delta: " << "undefined, to be inferred from ubodt file\n";
        }
        std::cout << "ubodt format(1 binary, 0 csv): " << binary_flag << '\n';
        std::cout << "k: " << k << '\n';
        std::cout << "radius: " << radius << '\n';
        std::cout << "gps_error: " << gps_error << '\n';
        std::cout << "------------------------------------------" << '\n';

    };
    std::string network_file;
    std::string network_id;
    std::string network_source;
    std::string network_target;

    // UBODT configurations
    std::string ubodt_file;
    int multiplier;
    int nhash;
    double delta;
    bool delta_defined = true;
    int binary_flag;

    // Parameters
    double gps_error;
    // Used by hashtable in UBODT

    // Used by Rtree search
    int k;
    double radius;
};

class MapMatcher {
public:
    MapMatcher(const std::string &config_file):config(MapMatcherConfig(config_file)){
        std::cout << "Loading model from file" << config_file <<'\n';
        network = new MM::Network(config.network_file,config.network_id,
                                  config.network_source,config.network_target);
        network->build_rtree_index();
        int multiplier = config.multiplier;
        int NHASH = config.nhash;
        ubodt = new MM::UBODT(multiplier,NHASH);
        if (config.binary_flag==1){
            ubodt->read_binary(config.ubodt_file);
        } else {
            ubodt->read_csv(config.ubodt_file);
        }
        if (!config.delta_defined){
            config.delta = ubodt->get_delta();
            std::cout<<"    Delta inferred from ubodt as "<< config.delta <<'\n';
        }
        std::cout << "Loading model finished" << '\n';
    };
    std::string match_geometry(const std::string &wkt){
        std::cout << "Perform map matching" << '\n';
        LineString line;
        bg::read_wkt(wkt,*(line.get_geometry()));
        int points_in_tr = line.getNumPoints();
        // Candidate search
        MM::Traj_Candidates traj_candidates = network->search_tr_cs_knn(&line,config.k,config.radius);
        MM::TransitionGraph tg = MM::TransitionGraph(&traj_candidates,&line,ubodt,config.delta);
        // Optimal path inference
        MM::O_Path *o_path_ptr = tg.viterbi();
        // Complete path construction as an array of indices of edges vector
        MM::T_Path *t_path_ptr = ubodt->construct_traversed_path(o_path_ptr);
        MM::LineString *m_geom = network->complete_path_to_geometry(o_path_ptr,&(t_path_ptr->cpath));
        std::string result = MM::IO::ResultWriter::mkString(
            network,o_path_ptr,t_path_ptr,m_geom
        );
        delete o_path_ptr;
        delete t_path_ptr;
        std::cout << "Perform map matching success" << '\n';
        return result;
    };
    ~MapMatcher(){
        delete network;
        delete ubodt;
    };
private:
    MM::UBODT *ubodt;
    MM::Network *network;
    MapMatcherConfig config;
    std::string match_trajectory(MM::Trajectory &trajectory){
        int points_in_tr = trajectory.geom->getNumPoints();
        // Candidate search
        MM::Traj_Candidates traj_candidates = network->search_tr_cs_knn(trajectory,config.k,config.radius);
        MM::TransitionGraph tg = MM::TransitionGraph(&traj_candidates,trajectory.geom,ubodt,config.delta);
        // Optimal path inference
        MM::O_Path *o_path_ptr = tg.viterbi();
        // Complete path construction as an array of indices of edges vector
        MM::T_Path *t_path_ptr = ubodt->construct_traversed_path(o_path_ptr);
        MM::LineString *m_geom = network->complete_path_to_geometry(o_path_ptr,&(t_path_ptr->cpath));
        std::string result = MM::IO::ResultWriter::mkString(
            network,o_path_ptr,t_path_ptr,m_geom
        );
        delete o_path_ptr;
        delete t_path_ptr;
        return result;
    };
};

} // MM
#endif
