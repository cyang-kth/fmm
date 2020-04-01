//
// Created by Can Yang on 2020/3/22.
//

#include "mm/fmm/fmm_algorithm.hpp"
#include "algorithm/geom_algorithm.hpp"
#include "util/util.hpp"
#include "util/debug.hpp"

namespace MM {

MatchResult FMM::match_traj(const MM::Trajectory &traj,
                                     const MM::FMMConfig &config) {
  SPDLOG_TRACE("Count of points in trajectory {}", traj.geom.get_num_points())
  SPDLOG_TRACE("Search candidates")
  Traj_Candidates tc = network_.search_tr_cs_knn(
      traj.geom, config.k, config.radius);
  SPDLOG_TRACE("Trajectory candidate {}", tc);
  if (tc.empty()) return MatchResult{};
  SPDLOG_TRACE("Generate transition graph")
  TransitionGraph tg(tc, config.gps_error);
  SPDLOG_TRACE("Update cost in transition graph")
  // The network will be used internally to update transition graph
  update_tg(&tg, traj, config);
  SPDLOG_TRACE("Optimal path inference")
  TGOpath oc_path = tg.backtrack();
  SPDLOG_TRACE("Optimal path size {}", oc_path.size())
  O_Path opath(oc_path.size());
  std::transform(oc_path.begin(), oc_path.end(), opath.begin(),
                 [](const Candidate *a) {
                   return a->edge->id;
                 });
  const std::vector<Edge> &edges = network_.get_edges();
  C_Path cpath = ubodt_.construct_complete_path(oc_path,edges);
  SPDLOG_TRACE("Complete path inference")
  LineString mgeom = network_.complete_path_to_geometry(
      traj.geom, cpath);
  return MatchResult{traj.id, opath, cpath, mgeom};
}

double FMM::get_sp_dist(const MM::Candidate *ca, const MM::Candidate *cb) {
  double sp_dist = 0;
  if (ca->edge->id == cb->edge->id && ca->offset <= cb->offset) {
    sp_dist = cb->offset - ca->offset;
  } else if (ca->edge->target == cb->edge->source) {
    // Transition on the same OD nodes
    sp_dist = ca->edge->length - ca->offset + cb->offset;
  } else {
    Record *r = ubodt_.look_up(ca->edge->target, cb->edge->source);
    // No sp path exist from O to D.
    if (r == NULL) return ubodt_.get_delta();
    // calculate original SP distance
    sp_dist = r->cost + ca->edge->length - ca->offset + cb->offset;
  }
  return sp_dist;
}

void FMM::update_tg(
    MM::TransitionGraph *tg,
    const MM::Trajectory &traj, const MM::FMMConfig &config) {
  SPDLOG_TRACE("Update transition graph")
  std::vector<TGLayer> &layers = tg->get_layers();
  std::vector<double> eu_dists = ALGORITHM::cal_eu_dist(traj.geom);
  int N = layers.size();
  for (int i = 0; i < N - 1; ++i) {
    update_layer(i, &(layers[i]), &(layers[i + 1]),
                 eu_dists[i]);
  }
  SPDLOG_TRACE("Update transition graph done")
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
      if (iter_a->cumu_prob + tp * iter_b->ep >= iter_b->cumu_prob)
      {
        iter_b->cumu_prob = iter_a->cumu_prob + tp * iter_b->ep;
        iter_b->prev = &(*iter_a);
      }
    }
  }
  SPDLOG_TRACE("Update layer done")
}

}
