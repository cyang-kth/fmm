//
// Created by Can Yang on 2020/3/22.
//

#include "mm/fmm/fmm_algorithm.hpp"
#include "algorithm/geom_algorithm.hpp"
#include "util/util.hpp"
#include "util/debug.hpp"

namespace FMM {
namespace MM{
FMMConfig::FMMConfig(int k_arg, double r_arg, double gps_error) :
    k(k_arg), radius(r_arg), gps_error(gps_error) {
};

void FMMConfig::print() const {
  SPDLOG_INFO("FMMAlgorithmConfig");
  SPDLOG_INFO("k {} radius {} gps_error {}", k, radius, gps_error);
};

FMMConfig FMMConfig::load_from_xml(
    const boost::property_tree::ptree &xml_data) {
  int k = xml_data.get("config.parameters.k", 8);
  double radius = xml_data.get("config.parameters.r", 300.0);
  double gps_error = xml_data.get("config.parameters.gps_error", 50.0);
  return FMMConfig{k, radius, gps_error};
};

FMMConfig FMMConfig::load_from_arg(
    const cxxopts::ParseResult &arg_data) {
  int k = arg_data["candidates"].as<int>();
  double radius = arg_data["radius"].as<double>();
  double gps_error = arg_data["error"].as<double>();
  return FMMConfig{k, radius, gps_error};
};

bool FMMConfig::validate() const {
  if (gps_error <= 0 || radius <= 0 || k <= 0) {
    SPDLOG_CRITICAL("Invalid mm parameter k {} r {} gps error {}",
                    k, radius, gps_error);
    return false;
  }
  return true;
}

MatchResult FMM::match_traj(const MM::Trajectory &traj,
                            const MM::FMMConfig &config) {
  SPDLOG_TRACE("Count of points in trajectory {}", traj.geom.get_num_points());
  SPDLOG_TRACE("Search candidates");
  Traj_Candidates tc = network_.search_tr_cs_knn(
      traj.geom, config.k, config.radius);
  SPDLOG_TRACE("Trajectory candidate {}", tc);
  if (tc.empty()) return MatchResult{};
  SPDLOG_TRACE("Generate transition graph");
  TransitionGraph tg(tc, config.gps_error);
  SPDLOG_TRACE("Update cost in transition graph");
  // The network will be used internally to update transition graph
  update_tg(&tg, traj, config);
  SPDLOG_TRACE("Optimal path inference");
  TGOpath tg_opath = tg.backtrack();
  SPDLOG_TRACE("Optimal path size {}", tg_opath.size());
  MatchedCandidatePath matched_candidate_path(tg_opath.size());
  std::transform(tg_opath.begin(), tg_opath.end(),
                 matched_candidate_path.begin(),
                 [](const TGElement *a) {
                   return MatchedCandidate{
                       *(a->c), a->ep, a->tp, a->sp_dist
                   };
                 });
  O_Path opath(tg_opath.size());
  std::transform(tg_opath.begin(), tg_opath.end(),
                 opath.begin(),
                 [](const TGElement *a) {
                   return a->c->edge->id;
                 });
  std::vector<int> indices;
  const std::vector<Edge> &edges = network_.get_edges();
  C_Path cpath = ubodt_->construct_complete_path(tg_opath, edges,
                                                 &indices);
  SPDLOG_TRACE("Cpath {}", cpath);
  SPDLOG_TRACE("Complete path inference");
  LineString mgeom = network_.complete_path_to_geometry(
      traj.geom, cpath);
  SPDLOG_TRACE("Complete path inference done");
  return MatchResult{
      traj.id, matched_candidate_path, opath, cpath, indices, mgeom};
}

PyMatchResult FMM::match_wkt(
    const std::string &wkt, const FMMConfig &config) {
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

double FMM::get_sp_dist(const MM::Candidate *ca, const MM::Candidate *cb) {
  double sp_dist = 0;
  if (ca->edge->id == cb->edge->id && ca->offset <= cb->offset) {
    sp_dist = cb->offset - ca->offset;
  } else if (ca->edge->target == cb->edge->source) {
    // Transition on the same OD nodes
    sp_dist = ca->edge->length - ca->offset + cb->offset;
  } else {
    Record *r = ubodt_->look_up(ca->edge->target, cb->edge->source);
    // No sp path exist from O to D.
    if (r == nullptr) return ubodt_->get_delta();
    // calculate original SP distance
    sp_dist = r->cost + ca->edge->length - ca->offset + cb->offset;
  }
  return sp_dist;
}

void FMM::update_tg(
    MM::TransitionGraph *tg,
    const MM::Trajectory &traj, const MM::FMMConfig &config) {
  SPDLOG_TRACE("Update transition graph");
  std::vector<TGLayer> &layers = tg->get_layers();
  std::vector<double> eu_dists = ALGORITHM::cal_eu_dist(traj.geom);
  int N = layers.size();
  for (int i = 0; i < N - 1; ++i) {
    update_layer(i, &(layers[i]), &(layers[i + 1]),
                 eu_dists[i]);
  }
  SPDLOG_TRACE("Update transition graph done");
}

void FMM::update_layer(int level,
                       MM::TGLayer *la_ptr,
                       MM::TGLayer *lb_ptr,
                       double eu_dist) {
  SPDLOG_TRACE("Update layer");
  TGLayer &lb = *lb_ptr;
  for (auto iter_a = la_ptr->begin(); iter_a != la_ptr->end(); ++iter_a) {
    NodeIndex source = iter_a->c->index;
    for (auto iter_b = lb_ptr->begin(); iter_b != lb_ptr->end(); ++iter_b) {
      double sp_dist = get_sp_dist(iter_a->c, iter_b->c);
      double tp = TransitionGraph::calc_tp(sp_dist, eu_dist);
      if (iter_a->cumu_prob + tp * iter_b->ep >= iter_b->cumu_prob) {
        iter_b->cumu_prob = iter_a->cumu_prob + tp * iter_b->ep;
        iter_b->prev = &(*iter_a);
        iter_b->tp = tp;
        iter_b->sp_dist = sp_dist;
      }
    }
  }
  SPDLOG_TRACE("Update layer done");
}
}
}
