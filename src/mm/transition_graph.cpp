/**
 * Content
 * Definition of a TransitionGraph, which is a wrapper of trajectory
 * candidates, raw trajectory and UBODT.
 * This class is designed for optimal path inference where
 * Viterbi algorithm is implemented.
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */

#include "mm/transition_graph.hpp"
#include "network/type.hpp"
#include "util/debug.hpp"

namespace MM
{

TransitionGraph::TransitionGraph(const Traj_Candidates &tc, double gps_error){
  for (auto cs = tc.begin(); cs!=tc.end(); ++cs) {
    TGLayer layer;
    for (auto iter = cs->begin(); iter!=cs->end(); ++iter) {
      double ep = calc_ep(iter->dist,gps_error);
      layer.push_back(TGElement{&(*iter),nullptr,ep,0});
    }
    layers.push_back(layer);
  }
  if (!tc.empty()) {
    reset_layer(&(layers[0]));
  }
}

double TransitionGraph::calc_tp(double sp_dist,double eu_dist){
  return eu_dist>=sp_dist ? (sp_dist+1e-6)/(eu_dist+1e-6) : eu_dist/sp_dist;
}

double TransitionGraph::calc_ep(double dist,double error){
  double a = dist / error;
  return exp(-0.5 * a * a);
}

// Reset the properties of a candidate set
void TransitionGraph::reset_layer(TGLayer *layer){
  for (auto iter=layer->begin(); iter!=layer->end(); ++iter) {
    iter->cumu_prob = iter->ep;
    iter->prev = nullptr;
  }
}

const TGElement *TransitionGraph::find_optimal_candidate(const TGLayer &layer){
  const TGElement *opt_c=nullptr;
  double final_prob = -0.001;
  for (auto c = layer.begin(); c!=layer.end(); ++c) {
    if(final_prob < c->cumu_prob) {
      final_prob = c->cumu_prob;
      opt_c = &(*c);
    }
  }
  return opt_c;
}

TGOpath TransitionGraph::backtrack(){
  SPDLOG_TRACE("Backtrack on transition graph");
  TGElement* track_cand=nullptr;
  double final_prob = -0.001;
  std::vector<TGElement>& last_layer = layers.back();
  for (auto c = last_layer.begin(); c!=last_layer.end(); ++c) {
    if(final_prob < c->cumu_prob) {
      final_prob = c->cumu_prob;
      track_cand = &(*c);
    }
  }
  TGOpath opath;
  if (final_prob>0) {
    opath.push_back(track_cand);
    // Iterate from tail to head to assign path
    while ((track_cand=track_cand->prev)!=nullptr) {
      opath.push_back(track_cand);
    }
    std::reverse(opath.begin(), opath.end());
  }
  SPDLOG_TRACE("Backtrack on transition graph done");
  return opath;
}

std::vector<TGLayer> &TransitionGraph::get_layers(){
  return layers;
}

} // MM
