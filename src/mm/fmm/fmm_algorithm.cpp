//
// Created by Can Yang on 2020/3/22.
//

#include "mm/fmm/fmm_algorithm.hpp"
#include "algorithm/geom_algorithm.hpp"
#include "util/util.hpp"
#include "util/debug.hpp"
#include "io/gps_reader.hpp"
#include "io/mm_writer.hpp"


using namespace FMM;
using namespace FMM::CORE;
using namespace FMM::NETWORK;
using namespace FMM::PYTHON;
using namespace FMM::MM;

//
class MatchException : public std::runtime_error {
public:
  int traj_id;
  int index1, index2;

  MatchException(int traj_id, int i, int j)
    : std::runtime_error("Unmatched trajectory ID=" + std::to_string(traj_id) +
                         " at points " + std::to_string(i) + " and " + std::to_string(j)),
      traj_id(traj_id), index1(i), index2(j) {}
};

FastMapMatchConfig::FastMapMatchConfig(int k_arg, double r_arg,
                                       double gps_error,
                                       double reverse_tolerance) :
  k(k_arg), radius(r_arg), gps_error(gps_error),
  reverse_tolerance(reverse_tolerance) {
};

void FastMapMatchConfig::print() const {
  SPDLOG_INFO("FMMAlgorithmConfig");
  SPDLOG_INFO("k {} radius {} gps_error {} reverse_tolerance {}",
    k, radius, gps_error, reverse_tolerance);
};

FastMapMatchConfig FastMapMatchConfig::load_from_xml(
  const boost::property_tree::ptree &xml_data) {
  int k = xml_data.get("config.parameters.k", 8);
  double radius = xml_data.get("config.parameters.r", 300.0);
  double gps_error = xml_data.get("config.parameters.gps_error", 50.0);
  double reverse_tolerance =
    xml_data.get("config.parameters.reverse_tolerance", 0.0);
  return FastMapMatchConfig{k, radius, gps_error, reverse_tolerance};
};

FastMapMatchConfig FastMapMatchConfig::load_from_arg(
  const cxxopts::ParseResult &arg_data) {
  int k = arg_data["candidates"].as<int>();
  double radius = arg_data["radius"].as<double>();
  double gps_error = arg_data["error"].as<double>();
  double reverse_tolerance = arg_data["reverse_tolerance"].as<double>();
  return FastMapMatchConfig{k, radius, gps_error, reverse_tolerance};
};

void FastMapMatchConfig::register_arg(cxxopts::Options &options){
  options.add_options()
    ("k,candidates","Number of candidates",
    cxxopts::value<int>()->default_value("8"))
    ("r,radius","Search radius",
    cxxopts::value<double>()->default_value("300.0"))
    ("reverse_tolerance","Ratio of reverse movement allowed",
      cxxopts::value<double>()->default_value("0.0"))
    ("e,error","GPS error",
    cxxopts::value<double>()->default_value("50.0"));
}

void FastMapMatchConfig::register_help(std::ostringstream &oss){
  oss<<"-k/--candidates (optional) <int>: Number of candidates (8)\n";
  oss<<"-r/--radius (optional) <double>: search "
    "radius (network data unit) (300)\n";
  oss<<"-e/--error (optional) <double>: GPS error "
    "(network data unit) (50)\n";
  oss<<"--reverse_tolerance (optional) <double>: proportion "
      "of reverse movement allowed on an edge\n";
};

bool FastMapMatchConfig::validate() const {
  if (gps_error <= 0 || radius <= 0 || k <= 0 || reverse_tolerance <0
    || reverse_tolerance>1 ) {
    SPDLOG_CRITICAL(
      "Invalid mm parameter k {} r {} gps error {} reverse_tolerance {}",
                    k, radius, gps_error,reverse_tolerance);
    return false;
  }
  return true;
}

MatchResult FastMapMatch::match_traj(const Trajectory &traj,
                                     const FastMapMatchConfig &config) {
  SPDLOG_DEBUG("Count of points in trajectory {}", traj.geom.get_num_points());
  SPDLOG_DEBUG("Search candidates");
  Traj_Candidates tc = network_.search_tr_cs_knn(
    traj.geom, config.k, config.radius);
  SPDLOG_DEBUG("Trajectory candidate {}", tc);
  if (tc.empty()) return MatchResult{};
  SPDLOG_DEBUG("Generate transition graph");
  TransitionGraph tg(tc, config.gps_error);
  SPDLOG_DEBUG("Update cost in transition graph");
  // The network will be used internally to update transition graph
  update_tg(&tg, traj, config.reverse_tolerance);
  SPDLOG_DEBUG("Optimal path inference");
  TGOpath tg_opath = tg.backtrack();
  SPDLOG_DEBUG("Optimal path size {}", tg_opath.size());
  MatchedCandidatePath matched_candidate_path(tg_opath.size());
  std::transform(tg_opath.begin(), tg_opath.end(),
                 matched_candidate_path.begin(),
                 [](const TGNode *a) {
    return MatchedCandidate{
      *(a->c), a->ep, a->tp, a->sp_dist
    };
  });
  O_Path opath(tg_opath.size());
  std::transform(tg_opath.begin(), tg_opath.end(),
                 opath.begin(),
                 [](const TGNode *a) {
    return a->c->edge->id;
  });
  std::vector<int> indices;
  const std::vector<Edge> &edges = network_.get_edges();
  C_Path cpath = ubodt_->construct_complete_path(traj.id, tg_opath, edges,
                                                 &indices,
                                                 config.reverse_tolerance);
  SPDLOG_DEBUG("Opath is {}", opath);
  SPDLOG_DEBUG("Indices is {}", indices);
  SPDLOG_DEBUG("Complete path is {}", cpath);
  LineString mgeom = network_.complete_path_to_geometry(
    traj.geom, cpath);
  return MatchResult{
    traj.id, matched_candidate_path, opath, cpath, indices, mgeom};
}

PyMatchResult FastMapMatch::match_wkt(
  const std::string &wkt, const FastMapMatchConfig &config) {
  LineString line = wkt2linestring(wkt);
  std::vector<double> timestamps;
  Trajectory traj{0, line, timestamps};
  MatchResult result = match_traj(traj, config);
  PyMatchResult output;
  output.id = result.id;
  output.opath = result.opath;
  output.cpath = result.cpath;
  output.mgeom = result.mgeom;
  output.indices = result.indices;
  for (int i = 0; i < result.opt_candidate_path.size(); ++i) {
    const MatchedCandidate &mc = result.opt_candidate_path[i];
    output.candidates.push_back(
      {i,
       mc.c.edge->id,
       graph_.get_node_id(mc.c.edge->source),
       graph_.get_node_id(mc.c.edge->target),
       mc.c.dist,
       mc.c.offset,
       mc.c.edge->length,
       mc.ep,
       mc.tp,
       mc.sp_dist}
      );
    output.pgeom.add_point(mc.c.point);
  }
  return output;
};

std::string FastMapMatch::match_gps_file(
  const FMM::CONFIG::GPSConfig &gps_config,
  const FMM::CONFIG::ResultConfig &result_config,
  const FastMapMatchConfig &fmm_config,
  bool use_omp
  ){
  std::ostringstream oss;
  std::string status;
  bool validate = true;
  if (!gps_config.validate()) {
    oss<<"gps_config invalid\n";
    validate = false;
  }
  if (!result_config.validate()) {
    oss<<"result_config invalid\n";
    validate = false;
  }
  if (!fmm_config.validate()) {
    oss<<"fmm_config invalid\n";
    validate = false;
  }
  if (!validate){
    oss<<"match_gps_file canceled\n";
    return oss.str();
  }
  // Start map matching
  int progress = 0;
  int points_matched = 0;
  int total_points = 0;
  int traj_matched = 0;
  int total_trajs = 0;
  int step_size = 1000;
  auto begin_time = UTIL::get_current_time();
  FMM::IO::GPSReader reader(gps_config);
  FMM::IO::CSVMatchResultWriter writer(result_config.file,
                                       result_config.output_config);
  if (use_omp){
    int buffer_trajectories_size = 100000;
    while (reader.has_next_trajectory()) {
      std::vector<Trajectory> trajectories =
        reader.read_next_N_trajectories(buffer_trajectories_size);
      int trajectories_fetched = trajectories.size();
      #pragma omp parallel for
      for (int i = 0; i < trajectories_fetched; ++i) {
        Trajectory &trajectory = trajectories[i];
        int points_in_tr = trajectory.geom.get_num_points();
        MM::MatchResult result = match_traj(
            trajectory, fmm_config);
        writer.write_result(trajectory,result);
        #pragma omp critical
        if (!result.cpath.empty()) {
          points_matched += points_in_tr;
          traj_matched+=1;
        }
        total_points += points_in_tr;
        total_trajs += 1;
        ++progress;
        if (progress % step_size == 0) {
          std::stringstream buf;
          buf << "Progress " << progress << '\n';
          std::cout << buf.rdbuf();
        }
      }
    }
  } else {
    while (reader.has_next_trajectory()) {
      if (progress % step_size == 0) {
        SPDLOG_INFO("Progress {}", progress);
      }
      Trajectory trajectory = reader.read_next_trajectory();
      int points_in_tr = trajectory.geom.get_num_points();
      MM::MatchResult result = match_traj(
        trajectory, fmm_config);
      writer.write_result(trajectory,result);
      if (!result.cpath.empty()) {
        points_matched += points_in_tr;
        traj_matched+=1;
      }
      total_points += points_in_tr;
      total_trajs += 1;
      ++progress;
    }
  }
  auto end_time = UTIL::get_current_time();
  double duration = UTIL::get_duration(begin_time,end_time);
  oss<<"Status: success\n";
  oss<<"Time takes " << duration << " seconds\n";
  oss<<"Total points " << total_points << " matched "<< points_matched <<"\n";
  oss<<"Total trajectories " << total_trajs << " matched "
     << traj_matched <<"\n";
  oss<<"Map match percentage " << points_matched / (double) total_points <<"\n";
  oss<<"Map match speed " << points_matched / duration << " points/s \n";
  return oss.str();
};

double FastMapMatch::get_sp_dist(
  const Candidate *ca, const Candidate *cb, double reverse_tolerance) {
  double sp_dist = 0;
  if (ca->edge->id == cb->edge->id && ca->offset <= cb->offset) {
    sp_dist = cb->offset - ca->offset;
  } else if (ca->edge->id == cb->edge->id &&
    ca->offset - cb->offset < ca->edge->length * reverse_tolerance) {
    sp_dist = 0;
  }
  else if (ca->edge->target == cb->edge->source) {
    // Transition on the same OD nodes
    sp_dist = ca->edge->length - ca->offset + cb->offset;
  } else {
    Record *r = ubodt_->look_up(ca->edge->target, cb->edge->source);
    // No sp path exist from O to D.
    if (r == nullptr) return std::numeric_limits<double>::infinity();
    // calculate original SP distance
    sp_dist = r->cost + ca->edge->length - ca->offset + cb->offset;
  }
  return sp_dist;
}

void FastMapMatch::update_tg(
  TransitionGraph *tg,
  const Trajectory &traj, double reverse_tolerance) {
  SPDLOG_DEBUG("Update transition graph");
  std::vector<TGLayer> &layers = tg->get_layers();
  std::vector<double> eu_dists = ALGORITHM::cal_eu_dist(traj.geom);
  int N = layers.size();
  for (int i = 0; i < N - 1; ++i) {
    SPDLOG_DEBUG("Update layer {} ", i);
    bool connected = false;
    update_layer(i, &(layers[i]), &(layers[i + 1]),
                 eu_dists[i], reverse_tolerance, &connected);
    if (!connected) {
      tg->print_optimal_info();
      std::stringstream ss;
      ss << "Unmatched trajectory " << traj.id << " at point " << i << " and " << (i + 1);
      throw std::runtime_error(ss.str());
    }
  }
  SPDLOG_DEBUG("Update transition graph done");
}


void FastMapMatch::update_layer(int level,
                                TGLayer *la_ptr,
                                TGLayer *lb_ptr,
                                double eu_dist,
                                double reverse_tolerance,
                                bool *connected) {
  // SPDLOG_TRACE("Update layer");
  TGLayer &lb = *lb_ptr;
  bool layer_connected = false;
  for (auto iter_a = la_ptr->begin(); iter_a != la_ptr->end(); ++iter_a) {
    NodeIndex source = iter_a->c->index;
    for (auto iter_b = lb_ptr->begin(); iter_b != lb_ptr->end(); ++iter_b) {
      double sp_dist = get_sp_dist(iter_a->c, iter_b->c,
        reverse_tolerance);
      double tp = TransitionGraph::calc_tp(sp_dist, eu_dist);
      double temp = iter_a->cumu_prob + log(tp) + log(iter_b->ep);
      SPDLOG_TRACE("L {} f {} t {} sp {} dist {} tp {} ep {} fcp {} tcp {}",
        level, iter_a->c->edge->id,iter_b->c->edge->id,
        sp_dist, eu_dist, tp, iter_b->ep, iter_a->cumu_prob,
        temp);
      if (temp >= iter_b->cumu_prob) {
        if (temp>-std::numeric_limits<double>::infinity()){
          layer_connected = true;
        }
        iter_b->cumu_prob = temp;
        iter_b->prev = &(*iter_a);
        iter_b->tp = tp;
        iter_b->sp_dist = sp_dist;
      }
    }
  }
  if (connected!=nullptr){
    *connected = layer_connected;
  }
  // SPDLOG_TRACE("Update layer done");
}
