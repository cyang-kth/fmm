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
#include "../src/python_types.hpp"

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
        // multiplier = tree.get("fmm_config.input.ubodt.multiplier", 37); // multiplier=30000
        // nhash = tree.get("fmm_config.input.ubodt.nhash", 127); // 5178049
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
        // std::cout << "multiplier: " << multiplier << '\n';
        // std::cout << "nhash: " << nhash << '\n';
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
    // int multiplier;
    // int nhash;
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
        int multipler = network->get_node_count();
        if (config.binary_flag==1){
            ubodt = MM::read_ubodt_binary(config.ubodt_file,multipler);
        } else {
            ubodt = MM::read_ubodt_csv(config.ubodt_file,multipler);
        }
        if (!config.delta_defined){
            config.delta = ubodt->get_delta();
            std::cout<<"    Delta inferred from ubodt as "<< config.delta <<'\n';
        }
        std::cout << "Loading model finished" << '\n';
    };
    MatchResult match_wkt(const std::string &wkt){
        std::cout << "Perform map matching" << '\n';
        LineString line;
        bg::read_wkt(wkt,*(line.get_geometry()));
        int points_in_tr = line.getNumPoints();
        // Candidate search
        MM::Traj_Candidates traj_candidates = network->search_tr_cs_knn(&line,config.k,config.radius,config.gps_error);
        MM::TransitionGraph tg = MM::TransitionGraph(&traj_candidates,&line,ubodt,config.delta);
        // Optimal path inference
        MM::O_Path *o_path_ptr = tg.viterbi();
        // Complete path construction as an array of indices of edges vector
        MM::T_Path *t_path_ptr = ubodt->construct_traversed_path(o_path_ptr);
        MM::LineString *m_geom = network->complete_path_to_geometry(o_path_ptr,&(t_path_ptr->cpath));
        MatchResult result = generate_result(network,o_path_ptr,t_path_ptr,m_geom);
        delete o_path_ptr;
        delete t_path_ptr;
        std::cout << "Perform map matching success" << '\n';
        return result;
    };
    /**
     *  Search the network for candidates matched to a trajectory
     */
    CandidateSet search_candidate(const std::string &wkt){
        LineString line;
        bg::read_wkt(wkt,*(line.get_geometry()));
        int points_in_tr = line.getNumPoints();
        Traj_Candidates traj_candidates = network->search_tr_cs_knn(&line,config.k,config.radius,config.gps_error);
        CandidateSet result;
        for (int i = 0;i < traj_candidates.size();++i){
            Point_Candidates & point_candidates = traj_candidates[i];
            for (int j = 0;j < point_candidates.size();++j){
                Candidate c = point_candidates[j];
                result.push_back({i,
                    std::stoi(c.edge->id_attr.c_str()),c.edge->source,c.edge->target,c.dist,
                    c.edge->length,c.offset,c.obs_prob});
            };
        };
        return result;
    };
    /**
     *  Build a transition lattice for the trajectory containing
     *  index,from,to,tp,ep,cp
     */
    TransitionLattice build_transition_lattice(const std::string &wkt){
        std::cout << "Perform map matching" << '\n';
        LineString line;
        bg::read_wkt(wkt,*(line.get_geometry()));
        int points_in_tr = line.getNumPoints();
        // Candidate search
        MM::Traj_Candidates traj_candidates = network->search_tr_cs_knn(&line,config.k,config.radius,config.gps_error);
        MM::TransitionGraph tg = MM::TransitionGraph(&traj_candidates,&line,ubodt,config.delta);
        return tg.generate_transition_lattice();
    };
    // Getter and setter to change the configuration in Python interactively. 
    void set_gps_error(double error){
        config.gps_error = error;
    };
    double get_gps_error(){
        return config.gps_error;
    };
    void set_k(int k){
        config.k = k;
    };
    int get_k(){
        return config.k;
    };
    double get_radius(){
        return config.radius;
    };
    void set_radius(double r){
        config.radius = r;
    };   
    ~MapMatcher(){
        delete network;
        delete ubodt;
    };
private:
    static MatchResult generate_result(Network *network_ptr,O_Path *o_path_ptr, T_Path *t_path_ptr, LineString *mgeom){
        MatchResult result;
        // Opath
        if (o_path_ptr != nullptr) {
            int N = o_path_ptr->size();
            for (int i = 0; i < N; ++i)
            {
                result.opath.push_back(std::stoi((*o_path_ptr)[i]->edge->id_attr));
            }
        };
        // Cpath
        if (t_path_ptr != nullptr) {
            C_Path *c_path_ptr = &(t_path_ptr->cpath);
            int N = c_path_ptr->size();
            for (int i = 0; i < N; ++i)
            {
                result.cpath.push_back(std::stoi(network_ptr->get_edge_id_attr((*c_path_ptr)[i])));
            }
        };
        if (mgeom!=nullptr){
            std::stringstream buf;
            MM::IO::ResultWriter::write_geometry(buf,mgeom);
            result.mgeom = buf.str();
        }
        std::stringstream pgeom_buf;
        MM::IO::ResultWriter::write_pgeom(pgeom_buf,o_path_ptr);
        result.pgeom = pgeom_buf.str();
        return result;
    };
    MM::UBODT *ubodt;
    MM::Network *network;
    MapMatcherConfig config;
};

} // MM
#endif
