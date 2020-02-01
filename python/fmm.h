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

#include "python_types.hpp"

namespace MM {

class MapMatcherConfig {
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
    // nhash = tree.get("fmm_config.input.ubodt.nhash", 127); // 5178049
    // Check if delta is specified or not
    if (!tree.get_optional<bool>
          ("fmm_config.input.ubodt.delta").is_initialized()) {
      delta_defined = false;
      delta = 0.0;
    } else {
      delta = tree.get("fmm_config.input.ubodt.delta",5000.0);       //
      delta_defined = true;
    }
    binary_flag = MM::UTIL::get_file_extension(ubodt_file);
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
    std::cout << "Configuration parameters for map matching application:\n ";
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
  MapMatcher(const std::string &config_file) :
    config(MapMatcherConfig(config_file)){
    std::cout << "Loading model from file" << config_file <<'\n';
    network = new Network(config.network_file,config.network_id,
            config.network_source,config.network_target);
    network->build_rtree_index();
    int multipler = network->get_node_count();
    if (config.binary_flag==1) {
      ubodt = MM::read_ubodt_binary(config.ubodt_file,multipler);
    } else {
      ubodt = MM::read_ubodt_csv(config.ubodt_file,multipler);
    }
    if (!config.delta_defined) {
      config.delta = ubodt->get_delta();
      std::cout<<"    Delta inferred from ubodt as "<< config.delta <<'\n';
    }
    std::cout << "Loading model finished" << '\n';
  };
  ~MapMatcher(){
    delete network;
    delete ubodt;
  };
  MatchResult match_wkt(const std::string &wkt){
    SPDLOG_INFO("Perform map matching");
    LineString line;
    bg::read_wkt(wkt,line.get_geometry());
    SPDLOG_INFO("Linestring WKT {}",line.exportToWkt());
    int points_in_tr = line.getNumPoints();
    SPDLOG_INFO("Search candidates");
    // Candidate search
    MM::Traj_Candidates traj_candidates = network->search_tr_cs_knn(
      line,config.k,config.radius,config.gps_error);
    SPDLOG_INFO("Build transition graph");
    MM::TransitionGraph tg = MM::TransitionGraph(
      &traj_candidates,line,*ubodt,config.delta);
    SPDLOG_INFO("Viterbi Optimal inference");
    // Optimal path inference
    MM::O_Path o_path = tg.viterbi();
    SPDLOG_INFO("Construct complete path");
    // Complete path construction as an array of indices of edges vector
    MM::T_Path t_path = ubodt->construct_traversed_path(o_path,*network);
    MM::LineString m_geom = network->complete_path_to_geometry(
      o_path,t_path.cpath);
    MatchResult result = generate_result(*network,o_path,t_path,m_geom);
    SPDLOG_INFO("Perform map matching success");
    return result;
  };
  /**
   *  Search the network for candidates matched to a trajectory
   */
  CandidateSet search_candidate(const std::string &wkt){
    LineString line;
    bg::read_wkt(wkt,line.get_geometry());
    int points_in_tr = line.getNumPoints();
    Traj_Candidates traj_candidates = network->search_tr_cs_knn(
      line,config.k,config.radius,config.gps_error);
    CandidateSet result;
    for (int i = 0; i < traj_candidates.size(); ++i) {
      Point_Candidates & point_candidates = traj_candidates[i];
      for (int j = 0; j < point_candidates.size(); ++j) {
        Candidate c = point_candidates[j];
        result.push_back({i,
                          c.edge->id,
                          network->get_node_id(c.edge->source),
                          network->get_node_id(c.edge->target),
                          c.dist,c.edge->length,c.offset,c.obs_prob});
      };
    };
    return result;
  };
  /**
   *  Build a transition lattice for the trajectory containing
   *  index,from,to,tp,ep,cp
   */
  TransitionLattice build_transition_lattice(const std::string &wkt){
    std::cout << "Building transition lattice" << '\n';
    LineString line;
    bg::read_wkt(wkt,line.get_geometry());
    int points_in_tr = line.getNumPoints();
    std::cout << "Number of points " <<points_in_tr<< '\n';
    // Candidate search
    MM::Traj_Candidates traj_candidates = network->search_tr_cs_knn(
      line,config.k,config.radius,config.gps_error);
    MM::TransitionGraph tg = MM::TransitionGraph(
      &traj_candidates,line,*ubodt,config.delta);
    return generate_transition_lattice(tg);
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
private:
  /**
   *  Generate transition lattice for the transition graph, used in
   *  Python extension for verification of the result
   */
  TransitionLattice generate_transition_lattice(MM::TransitionGraph &tg){
    TransitionLattice tl;
    Traj_Candidates *m_traj_candidates = tg.get_traj_candidates();
    if (m_traj_candidates->empty()) return tl;
    std::vector<double> &eu_distances = tg.get_eu_distances();
    int N = m_traj_candidates->size();
    /* Update transition probabilities */
    Traj_Candidates::iterator csa = m_traj_candidates->begin();
    /* Initialize the cumu probabilities of the first layer */
    Point_Candidates::iterator ca = csa->begin();
    while (ca != csa->end())
    {
      ca->cumu_prob = ca->obs_prob;
      ++ca;
    }
    /* Updating the cumu probabilities of subsequent layers */
    Traj_Candidates::iterator csb = m_traj_candidates->begin();
    ++csb;
    while (csb != m_traj_candidates->end())
    {
      Point_Candidates::iterator ca = csa->begin();
      double eu_dist=eu_distances[std::distance(
                                    m_traj_candidates->begin(),csa)];
      while (ca != csa->end())
      {
        Point_Candidates::iterator cb = csb->begin();
        while (cb != csb->end())
        {

          int step =std::distance(m_traj_candidates->begin(),csa);
          // Calculate transition probability
          double sp_dist = tg.get_sp_dist_penalized(ca,cb,0);
          /*
             A degenerate case is that the *same point
             is reported multiple times where both eu_dist and sp_dist = 0
           */
          double tran_prob = 1.0;
          if (eu_dist<0.00001) {
            tran_prob =sp_dist>0.00001 ? 0 : 1.0;
          } else {
            tran_prob =eu_dist>sp_dist ? sp_dist/eu_dist : eu_dist/sp_dist;
          }

          if (ca->cumu_prob + tran_prob * cb->obs_prob >= cb->cumu_prob)
          {
            cb->cumu_prob = ca->cumu_prob + tran_prob * cb->obs_prob;
            cb->prev = &(*ca);
            cb->sp_dist = sp_dist;
          }
          tl.push_back(
            {step,
             ca->edge->id,
             cb->edge->id,
             sp_dist,
             eu_dist,
             tran_prob,
             cb->obs_prob,
             ca->cumu_prob + tran_prob * cb->obs_prob});
          ++cb;
        }
        ++ca;
      }
      ++csa;
      ++csb;
    }
    return tl;
  };

  static MatchResult generate_result(
    const Network &network,const O_Path &o_path,
    const T_Path &t_path, const LineString &mgeom){
    MatchResult result;
    // Opath
    if (!o_path.empty()) {
      int N = o_path.size();
      for (int i = 0; i < N; ++i)
      {
        result.opath.push_back(o_path[i]->edge->id);
      }
    };
    // Cpath
    if (!t_path.cpath.empty()) {
      const C_Path &c_path = t_path.cpath;
      int N = c_path.size();
      for (int i = 0; i < N; ++i)
      {
        result.cpath.push_back(c_path[i]);
      }
    };
    if (!mgeom.isEmpty()) {
      std::stringstream buf;
      MM::IO::ResultWriter::write_geometry(buf,mgeom);
      result.mgeom = buf.str();
    }
    std::stringstream pgeom_buf;
    MM::IO::ResultWriter::write_pgeom(pgeom_buf,o_path);
    result.pgeom = pgeom_buf.str();
    return result;
  };
  MM::UBODT *ubodt;
  MM::Network *network;
  MapMatcherConfig config;
};

} // MM
#endif
