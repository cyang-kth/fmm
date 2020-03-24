#include "mm/stmatch/stmatch.hpp"
#include "algorithm/geom_algorithm.hpp"
#include "util/debug.hpp"
#include "util/util.hpp"

#include <limits>

namespace MM {

// Procedure of HMM based map matching algorithm.
MatchResult STMATCH::match_temporal_traj(const TemporalTrajectory &traj,
                                         const MMConfig &config){
  SPDLOG_TRACE("Count of points in trajectory {}", traj.geom.get_num_points())
  SPDLOG_TRACE("Search candidates")
  Traj_Candidates tc = network.search_tr_cs_knn(
    traj.geom, config.k, config.radius);
  SPDLOG_TRACE("Trajectory candidate {}", tc);
  if (tc.empty()) return MatchResult{};
  SPDLOG_TRACE("Generate dummy graph")
  DummyGraph dg(tc);
  SPDLOG_TRACE("Generate composite_graph")
  CompositeGraph cg(ng,dg);
  SPDLOG_TRACE("Generate composite_graph")
  TransitionGraph tg(tc,config.gps_error);
  SPDLOG_TRACE("Update cost in transition graph")
  // The network will be used internally to update transition graph
  update_tg(&tg,cg,traj,config);
  SPDLOG_TRACE("Optimal path inference")
  OptCandidatePath oc_path = tg.backtrack();
  SPDLOG_TRACE("Optimal path size {}",oc_path.size())
  O_Path opath(oc_path.size());
  std::transform(oc_path.begin(),oc_path.end(),opath.begin(),
                 [](const Candidate *a){
      return a->edge->id;
    });
  C_Path cpath = build_cpath(oc_path);
  SPDLOG_TRACE("Complete path inference")
  LineString mgeom = network.complete_path_to_geometry(
    traj.geom,cpath);
  return MatchResult{traj.id, opath, cpath, mgeom};
}

void STMATCH::update_tg(TransitionGraph *tg,
                        const CompositeGraph &cg,
                        const TemporalTrajectory &traj,
                        const MMConfig &config){
  SPDLOG_TRACE("Update transition graph")
  if (logger!=nullptr) {
    SPDLOG_LOGGER_TRACE(logger, "level;id;dist;geom")
  }
  std::vector<TGLayer> &layers = tg->get_layers();
  std::vector<double> eu_dists = ALGORITHM::cal_eu_dist(traj.geom);
  int N = layers.size();
  for(int i=0; i<N-1; ++i) {
    // Routing from current_layer to next_layer
    SPDLOG_TRACE("Update layer {} and timestamp {}",i,
        traj.timestamps[i])
    double duration = traj.timestamps[i+1] - traj.timestamps[i];
    double delta = config.factor * config.vmax * duration;
    update_layer(i, &(layers[i]), &(layers[i+1]),
                 cg,eu_dists[i],delta);
  }
  SPDLOG_TRACE("Update transition graph done")
}

void STMATCH::update_layer(int level, TGLayer *la_ptr, TGLayer *lb_ptr,
                           const CompositeGraph &cg,
                           double eu_dist,
                           double delta){
  // SPDLOG_TRACE("Update layer");
  TGLayer &lb = *lb_ptr;
  for (auto iter=la_ptr->begin(); iter!=la_ptr->end(); ++iter) {
    NodeIndex source = iter->c->index;
    SPDLOG_TRACE("  Calculate distance from source {}",source)
    // single source upper bound routing
    std::vector<NodeIndex> targets(lb.size());
    std::transform(lb.begin(),lb.end(),targets.begin(),
                   [](TGElement &a){
        return a.c->index;
      });
    SPDLOG_TRACE("  Upperbound shortest path {} ",delta)
    std::vector<double> distances = shortest_path_upperbound(
      level, cg, source, targets, delta);
    SPDLOG_TRACE("  Update property of transition graph ")
    for (int i=0; i<distances.size(); ++i) {
      double tp = TransitionGraph::calc_tp(distances[i],eu_dist);
      if (lb[i].cumu_prob<iter->cumu_prob+tp*lb[i].ep) {
        lb[i].cumu_prob = iter->cumu_prob+tp*lb[i].ep;
        lb[i].prev = &(*iter);
      }
    }
  }
  SPDLOG_TRACE("Update layer done")
}

/**
 * Return distances from source to all targets and with an upper bound of
 * delta to stop the search
 * @param  source  source node index
 * @param  targets a vector of N target node indices
 * @param  delta   upperbound of search
 * @return         a vector of N indices
 */
std::vector<double> STMATCH::shortest_path_upperbound(
  int level, const CompositeGraph &cg, NodeIndex source,
  const std::vector<NodeIndex> &targets, double delta){
  SPDLOG_TRACE("Upperbound shortest path source {}", source)
  SPDLOG_TRACE("Upperbound shortest path targets {}", targets)
  std::unordered_set<NodeIndex> unreached_targets;
  for (auto &node:targets) {
    unreached_targets.insert(node);
  }
  DistanceMap dmap;
  PredecessorMap pmap;
  Heap Q;
  Q.push(source,0);
  pmap.insert({source,source});
  dmap.insert({source,0});
  double temp_dist = 0;
  // Dijkstra search
  while (!Q.empty() && !unreached_targets.empty()) {
    HeapNode node = Q.top();
    Q.pop();
    SPDLOG_TRACE("  Node u {} dist {}", node.index,node.dist)
    NodeIndex u = node.index;
    if (logger!=nullptr){
      // level;index;id;dist;geom
      if (!cg.check_dummy_node(u)) {
        SPDLOG_LOGGER_TRACE(logger, "{};{};{};{}", level,
                            network.get_node_id(u), node.dist,
                            network.get_node_geom_from_idx(u))
      }
    }
    auto iter = unreached_targets.find(u);
    if (iter!=unreached_targets.end()) {
      // Remove u
      SPDLOG_TRACE("  Remove target {}",u)
      unreached_targets.erase(iter);
    }
    if (node.dist>delta) break;
    std::vector<CompEdgeProperty> out_edges = cg.out_edges(u);
    for (auto node_iter = out_edges.begin(); node_iter!=out_edges.end();
         ++node_iter) {
      NodeIndex v = node_iter->v;
      temp_dist = node.dist + node_iter->cost;
      SPDLOG_TRACE("  Examine node v {} temp dist {}",v,temp_dist)
      auto v_iter = dmap.find(v);
      if (v_iter!=dmap.end()) {
        // dmap contains node v
        if (v_iter->second-temp_dist>1e-6) {
          // a smaller distance is found for v
          SPDLOG_TRACE("    Update key {} {} in pdmap prev dist {}",
              v, temp_dist, v_iter->second)
          pmap[v] = u;
          dmap[v] = temp_dist;
          Q.decrease_key(v,temp_dist);
        }
      } else {
        // dmap does not contain v
        if (temp_dist<=delta) {
          SPDLOG_TRACE("    Insert key {} {} into pmap and dmap",
              v, temp_dist)
          Q.push(v,temp_dist);
          pmap.insert({v,u});
          dmap.insert({v,temp_dist});
        }
      }
    }
  }
  // Update distances
  SPDLOG_TRACE("  Update distances")
  std::vector<double> distances;
  for (int i=0; i<targets.size(); ++i) {
    if (dmap.find(targets[i])!=dmap.end()) {
      distances.push_back(dmap[targets[i]]);
    } else {
      distances.push_back(std::numeric_limits<double>::max());
    }
  }
  SPDLOG_TRACE("  Distance value {}",distances)
  return distances;
}

C_Path STMATCH::build_cpath(const OptCandidatePath &opath){
  SPDLOG_TRACE("Build cpath from optimal candidate path")
  C_Path cpath;
  if (opath.empty()) return cpath;
  const std::vector<Edge> &edges = network.get_edges();
  int N = opath.size();
  cpath.push_back(opath[0]->edge->id);
  for(int i=0; i<N-1; ++i) {
    const Candidate* a = opath[i];
    const Candidate* b = opath[i+1];
    // SPDLOG_TRACE("Check a {} b {}",a->edge->id,b->edge->id);
    if ((a->edge->id!=b->edge->id) || (a->offset>b->offset)) {
      auto segs = ng.shortest_path_dijkstra(a->edge->target,b->edge->source);
      // No transition exist in UBODT
      if (segs.empty() &&  a->edge->target!=b->edge->source) {
        return {};
      }
      // SPDLOG_TRACE("Edges found {}",segs);
      for (int e:segs) {
        cpath.push_back(edges[e].id);
      }
      cpath.push_back(b->edge->id);
    }
  }
  // SPDLOG_INFO("Build cpath from optimal candidate path done");
  return cpath;
}

} // MM
