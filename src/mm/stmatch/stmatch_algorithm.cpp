#include "mm/stmatch/stmatch_algorithm.hpp"
#include "algorithm/geom_algorithm.hpp"
#include "util/debug.hpp"
#include "util/util.hpp"
#include "io/gps_reader.hpp"
#include "io/mm_writer.hpp"

#include <limits>

using namespace FMM;
using namespace FMM::CORE;
using namespace FMM::NETWORK;
using namespace FMM::MM;
using namespace FMM::PYTHON;

STMATCHConfig::STMATCHConfig(
  int k_arg, double r_arg, double gps_error_arg,
  double vmax_arg, double factor_arg, double reverse_tolerance_arg):
  k(k_arg), radius(r_arg), gps_error(gps_error_arg),
  vmax(vmax_arg), factor(factor_arg),
  reverse_tolerance(reverse_tolerance_arg) {
};

void STMATCHConfig::print() const {
  SPDLOG_INFO("STMATCHAlgorithmConfig");
  SPDLOG_INFO("k {} radius {} gps_error {} vmax {} factor {}",
              k, radius, gps_error, vmax, factor);
};

STMATCHConfig STMATCHConfig::load_from_xml(
  const boost::property_tree::ptree &xml_data) {
  int k = xml_data.get("config.parameters.k", 8);
  double radius = xml_data.get("config.parameters.r", 300.0);
  double gps_error = xml_data.get("config.parameters.gps_error", 50.0);
  double vmax = xml_data.get("config.parameters.vmax", 80.0);
  double factor = xml_data.get("config.parameters.factor", 1.5);
  double reverse_tolerance =
    xml_data.get("config.parameters.reverse_tolerance", 0.0);
  return STMATCHConfig{k, radius, gps_error, vmax, factor,reverse_tolerance};
};

STMATCHConfig STMATCHConfig::load_from_arg(
  const cxxopts::ParseResult &arg_data) {
  int k = arg_data["candidates"].as<int>();
  double radius = arg_data["radius"].as<double>();
  double gps_error = arg_data["error"].as<double>();
  double vmax = arg_data["vmax"].as<double>();
  double factor = arg_data["factor"].as<double>();
  double reverse_tolerance = arg_data["reverse_tolerance"].as<double>();
  return STMATCHConfig{k, radius, gps_error, vmax, factor, reverse_tolerance};
};

void STMATCHConfig::register_arg(cxxopts::Options &options){
  options.add_options()
    ("k,candidates","Number of candidates",
    cxxopts::value<int>()->default_value("8"))
    ("r,radius","Search radius",
    cxxopts::value<double>()->default_value("300.0"))
    ("e,error","GPS error",
    cxxopts::value<double>()->default_value("50.0"))
    ("vmax","Maximum speed",
    cxxopts::value<double>()->default_value("30.0"))
    ("factor","Scale factor",
    cxxopts::value<double>()->default_value("1.5"))
    ("reverse_tolerance","Ratio of reverse movement allowed",
      cxxopts::value<double>()->default_value("0.0"));
}

void STMATCHConfig::register_help(std::ostringstream &oss){
  oss<<"-k/--candidates (optional) <int>: number of candidates (8)\n";
  oss<<"-r/--radius (optional) <double>: search "
    "radius (network data unit) (300)\n";
  oss<<"-e/--error (optional) <double>: GPS error "
    "(network data unit) (50)\n";
  oss<<"-f/--factor (optional) <double>: scale factor (1.5)\n";
  oss<<"-v/--vmax (optional) <double>: "
    " Maximum speed (unit: network_data_unit/s) (30)\n";
  oss<<"--reverse_tolerance (optional) <double>: proportion "
      "of reverse movement allowed on an edge\n";
};

bool STMATCHConfig::validate() const {
  if (gps_error <= 0 || radius <= 0 || k <= 0 || vmax <= 0 || factor <= 0
      || reverse_tolerance<0) {
    SPDLOG_CRITICAL("Invalid mm parameter k {} r {} gps error {} "
        "vmax {} f {} reverse_tolerance {}",
                    k, radius, gps_error, vmax, factor, reverse_tolerance);
    return false;
  }
  return true;
}

PyMatchResult STMATCH::match_wkt(
  const std::string &wkt, const STMATCHConfig &config) {
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

// Procedure of HMM based map matching algorithm.
MatchResult STMATCH::match_traj(const Trajectory &traj,
                                const STMATCHConfig &config) {
  SPDLOG_DEBUG("Count of points in trajectory {}", traj.geom.get_num_points());
  SPDLOG_DEBUG("Search candidates");
  Traj_Candidates tc = network_.search_tr_cs_knn(
    traj.geom, config.k, config.radius);
  SPDLOG_DEBUG("Trajectory candidate {}", tc);
  if (tc.empty()) return MatchResult{};
  SPDLOG_DEBUG("Generate dummy graph");
  DummyGraph dg(tc, config.reverse_tolerance);
  SPDLOG_DEBUG("Generate composite_graph");
  CompositeGraph cg(graph_, dg);
  SPDLOG_DEBUG("Generate composite_graph");
  TransitionGraph tg(tc, config.gps_error);
  SPDLOG_DEBUG("Update cost in transition graph");
  // The network will be used internally to update transition graph
  update_tg(&tg, cg, traj, config);
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
  C_Path cpath = build_cpath(tg_opath, &indices, config.reverse_tolerance);
  SPDLOG_DEBUG("Opath is {}", opath);
  SPDLOG_DEBUG("Indices is {}", indices);
  SPDLOG_DEBUG("Complete path is {}", cpath);
  LineString mgeom = network_.complete_path_to_geometry(
    traj.geom, cpath);
  return MatchResult{
    traj.id, matched_candidate_path, opath, cpath, indices, mgeom};
}

std::string STMATCH::match_gps_file(
  const FMM::CONFIG::GPSConfig &gps_config,
  const FMM::CONFIG::ResultConfig &result_config,
  const STMATCHConfig &stmatch_config,
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
  if (!stmatch_config.validate()) {
    oss<<"stmatch_config invalid\n";
    validate = false;
  }
  if (!validate) {
    oss<<"match_gps_file canceled\n";
    return oss.str();
  }
  // Start map matching
  int progress = 0;
  int points_matched = 0;
  int total_points = 0;
  int step_size = 1000;
  UTIL::TimePoint begin_time = std::chrono::steady_clock::now();
  FMM::IO::GPSReader reader(gps_config);
  FMM::IO::CSVMatchResultWriter writer(result_config.file,
                                       result_config.output_config);
  if (use_omp) {
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
          trajectory, stmatch_config);
        writer.write_result(trajectory,result);
        #pragma omp critical
        if (!result.cpath.empty()) {
          points_matched += points_in_tr;
        }
        total_points += points_in_tr;
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
        trajectory, stmatch_config);
      writer.write_result(trajectory,result);
      if (!result.cpath.empty()) {
        points_matched += points_in_tr;
      }
      total_points += points_in_tr;
      ++progress;
    }
  }
  UTIL::TimePoint end_time = std::chrono::steady_clock::now();
  double duration = std::chrono::duration_cast<
    std::chrono::milliseconds>(end_time - begin_time).count() / 1000.;
  oss<<"Status: success\n";
  oss<<"Time takes " << duration << " seconds\n";
  oss<<"Total points " << total_points << " matched "<< points_matched <<"\n";
  oss<<"Map match speed " << points_matched / duration << " points/s \n";
  return oss.str();
};

void STMATCH::update_tg(TransitionGraph *tg,
                        const CompositeGraph &cg,
                        const Trajectory &traj,
                        const STMATCHConfig &config) {
  SPDLOG_DEBUG("Update transition graph");
  std::vector<TGLayer> &layers = tg->get_layers();
  std::vector<double> eu_dists = ALGORITHM::cal_eu_dist(traj.geom);
  int N = layers.size();
  for (int i = 0; i < N - 1; ++i) {
    // Routing from current_layer to next_layer
    SPDLOG_DEBUG("Update layer {} ", i);
    double delta = 0;
    if (traj.timestamps.size() != N) {
      delta = eu_dists[i] * config.factor * 4;
    } else {
      double duration = traj.timestamps[i + 1] - traj.timestamps[i];
      delta = config.factor * config.vmax * duration;
    }
    update_layer(i, &(layers[i]), &(layers[i + 1]),
                 cg, eu_dists[i], delta);
  }
  SPDLOG_DEBUG("Update transition graph done");
}

void STMATCH::update_layer(int level, TGLayer *la_ptr, TGLayer *lb_ptr,
                           const CompositeGraph &cg,
                           double eu_dist,
                           double delta) {
  // SPDLOG_TRACE("Update layer");
  TGLayer &lb = *lb_ptr;
  for (auto iter = la_ptr->begin(); iter != la_ptr->end(); ++iter) {
    NodeIndex source = iter->c->index;
    SPDLOG_TRACE("  Calculate distance from source {}", source);
    // single source upper bound routing
    std::vector<NodeIndex> targets(lb.size());
    std::transform(lb.begin(), lb.end(), targets.begin(),
                   [](TGNode &a) {
      return a.c->index;
    });
    SPDLOG_TRACE("  Upperbound shortest path {} ", delta);
    std::vector<double> distances = shortest_path_upperbound(
      level, cg, source, targets, delta);
    SPDLOG_TRACE("  Update property of transition graph ");
    for (int i = 0; i < distances.size(); ++i) {
      double tp = TransitionGraph::calc_tp(distances[i], eu_dist);
      double temp = iter->cumu_prob + log(tp) + log(lb[i].ep);
      if (lb[i].cumu_prob<temp) {
        lb[i].cumu_prob = temp;
        lb[i].prev = &(*iter);
        lb[i].sp_dist = distances[i];
        lb[i].tp = tp;
      }
    }
  }
  SPDLOG_TRACE("Update layer done");
}

std::vector<double> STMATCH::shortest_path_upperbound(
  int level, const CompositeGraph &cg, NodeIndex source,
  const std::vector<NodeIndex> &targets, double delta) {
  SPDLOG_TRACE("Upperbound shortest path source {}", source);
  SPDLOG_TRACE("Upperbound shortest path targets {}", targets);
  std::unordered_set<NodeIndex> unreached_targets;
  for (auto &node:targets) {
    unreached_targets.insert(node);
  }
  DistanceMap dmap;
  PredecessorMap pmap;
  Heap Q;
  Q.push(source, 0);
  pmap.insert({source, source});
  dmap.insert({source, 0});
  double temp_dist = 0;
  // Dijkstra search
  while (!Q.empty() && !unreached_targets.empty()) {
    HeapNode node = Q.top();
    Q.pop();
    SPDLOG_TRACE("  Node u {} dist {}", node.index, node.value);
    NodeIndex u = node.index;
    auto iter = unreached_targets.find(u);
    if (iter != unreached_targets.end()) {
      // Remove u
      SPDLOG_TRACE("  Remove target {}", u);
      unreached_targets.erase(iter);
    }
    if (node.value > delta) break;
    std::vector<CompEdgeProperty> out_edges = cg.out_edges(u);
    for (auto node_iter = out_edges.begin(); node_iter != out_edges.end();
         ++node_iter) {
      NodeIndex v = node_iter->v;
      temp_dist = node.value + node_iter->cost;
      SPDLOG_TRACE("  Examine node v {} temp dist {}", v, temp_dist);
      auto v_iter = dmap.find(v);
      if (v_iter != dmap.end()) {
        // dmap contains node v
        if (v_iter->second - temp_dist > 1e-6) {
          // a smaller distance is found for v
          SPDLOG_TRACE("    Update key {} {} in pdmap prev dist {}",
                       v, temp_dist, v_iter->second);
          pmap[v] = u;
          dmap[v] = temp_dist;
          Q.decrease_key(v, temp_dist);
        }
      } else {
        // dmap does not contain v
        if (temp_dist <= delta) {
          SPDLOG_TRACE("    Insert key {} {} into pmap and dmap",
                       v, temp_dist);
          Q.push(v, temp_dist);
          pmap.insert({v, u});
          dmap.insert({v, temp_dist});
        }
      }
    }
  }
  // Update distances
  SPDLOG_TRACE("  Update distances");
  std::vector<double> distances;
  for (int i = 0; i < targets.size(); ++i) {
    if (dmap.find(targets[i]) != dmap.end()) {
      distances.push_back(dmap[targets[i]]);
    } else {
      distances.push_back(std::numeric_limits<double>::max());
    }
  }
  SPDLOG_TRACE("  Distance value {}", distances);
  return distances;
}

C_Path STMATCH::build_cpath(const TGOpath &opath, std::vector<int> *indices,
  double reverse_tolerance) {
  SPDLOG_DEBUG("Build cpath from optimal candidate path");
  C_Path cpath;
  if (!indices->empty()) indices->clear();
  if (opath.empty()) return cpath;
  const std::vector<Edge> &edges = network_.get_edges();
  int N = opath.size();
  cpath.push_back(opath[0]->c->edge->id);
  int current_idx = 0;
  SPDLOG_TRACE("Insert index {}", current_idx);
  indices->push_back(current_idx);
  for (int i = 0; i < N - 1; ++i) {
    const Candidate *a = opath[i]->c;
    const Candidate *b = opath[i + 1]->c;
    SPDLOG_DEBUG("Check a {} b {}", a->edge->id, b->edge->id);
    if ((a->edge->id != b->edge->id) ||
        (a->offset-b->offset>a->edge->length * reverse_tolerance)) {
      auto segs = graph_.shortest_path_dijkstra(a->edge->target,
                                                b->edge->source);
      // No transition found
      if (segs.empty() && a->edge->target != b->edge->source) {
        SPDLOG_DEBUG("Edges not found connecting a b");
        indices->clear();
        return {};
      }
      if (segs.empty()) {
        SPDLOG_DEBUG("Edges ab are adjacent");
      } else {
        SPDLOG_DEBUG("Edges connecting ab are {}", segs);
      }

      for (int e:segs) {
        cpath.push_back(edges[e].id);
        ++current_idx;
      }
      cpath.push_back(b->edge->id);
      ++current_idx;
      SPDLOG_TRACE("Insert index {}", current_idx);
      indices->push_back(current_idx);
    } else {
      SPDLOG_TRACE("Insert index {}", current_idx);
      indices->push_back(current_idx);
    }
  }
  SPDLOG_DEBUG("Build cpath from optimal candidate path done");
  return cpath;
}
