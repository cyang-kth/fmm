#include "network/network_graph.hpp"
#include "network/heap.hpp"
#include "network/network.hpp"
#include "util/debug.hpp"

#include <cmath>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <queue>

using namespace FMM;
using namespace FMM::CORE;
using namespace FMM::NETWORK;
NetworkGraph::NetworkGraph(const Network &network_arg) : network(network_arg) {
  const std::vector<Edge> &edges = network.get_edges();
  SPDLOG_INFO("Construct graph from network edges start");
  EdgeDescriptor e;
  bool inserted;
  g = Graph_T();
  int N = edges.size();
  for (int i = 0; i < N; ++i) {
    const Edge &edge = edges[i];
    boost::tie(e, inserted) = boost::add_edge(edge.source, edge.target, g);
    // id is the FID read, id_attr is the external property in SHP
    g[e].index = edge.index;
    g[e].length = edge.length;
  }
  num_vertices = boost::num_vertices(g);
  SPDLOG_INFO("Graph nodes {} edges {}", num_vertices, boost::num_edges(g));
  SPDLOG_INFO("Construct graph from network edges end");
}

void NetworkGraph::print_graph() const {
  boost::graph_traits<Graph_T>::edge_iterator it, end;
  for (boost::tie(it, end) = boost::edges(g); it != end; ++it)
    std::cout << " index " << g[*it].index << " edge " <<
      network.get_edge_id(g[*it].index) << " "
              << network.get_node_id(boost::source(*it, g)) << " -> "
              << network.get_node_id(boost::target(*it, g)) << '\n';
}

const Graph_T &NetworkGraph::get_boost_graph() const {
  return g;
}

const Network &NetworkGraph::get_network() {
  return network;
}
unsigned int NetworkGraph::get_num_vertices() const {
  return num_vertices;
}

std::vector<EdgeIndex> NetworkGraph::shortest_path_dijkstra(
  NodeIndex source, NodeIndex target) const {
  SPDLOG_TRACE("Shortest path starts");
  if (source == target) return {};
  Heap Q;
  PredecessorMap pmap;
  DistanceMap dmap;
  // Initialization
  Q.push(source, 0);
  pmap.insert({source, source});
  dmap.insert({source, 0});
  OutEdgeIterator out_i, out_end;
  double temp_dist = 0;
  // Dijkstra search
  while (!Q.empty()) {
    HeapNode node = Q.top();
    Q.pop();
    NodeIndex u = node.index;
    if (u == target) break;
    for (boost::tie(out_i, out_end) = boost::out_edges(u, g);
         out_i != out_end; ++out_i) {
      EdgeDescriptor e = *out_i;
      NodeIndex v = boost::target(e, g);
      temp_dist = node.value + g[e].length;
      auto iter = dmap.find(v);
      if (iter != dmap.end()) {
        // dmap contains node v
        if (iter->second > temp_dist) {
          // a smaller distance is found for v
          pmap[v] = u;
          dmap[v] = temp_dist;
          Q.decrease_key(v, temp_dist);
        }
      } else {
        // dmap does not contain v
        Q.push(v, temp_dist);
        pmap.insert({v, u});
        dmap.insert({v, temp_dist});
      }
    }
  }
  // Backtrack from target to source
  return back_track(source, target, pmap, dmap);
}

double NetworkGraph::calc_heuristic_dist(
  const Point &p1, const Point &p2) const {
  return sqrt((p2.get<0>() - p1.get<0>()) * (p2.get<0>() - p1.get<0>()) +
              (p2.get<1>() - p1.get<1>()) * (p2.get<1>() - p1.get<1>()));
}

std::vector<EdgeIndex> NetworkGraph::shortest_path_astar(
  NodeIndex source, NodeIndex target) const {
  SPDLOG_TRACE("Shortest path astar starts");
  if (source == target) return {};
  const std::vector<Point> &vertex_points =
    network.get_vertex_points();
  Heap Q;
  PredecessorMap pmap;
  DistanceMap dmap;
  // Initialization
  double h = calc_heuristic_dist(vertex_points[source],
                                 vertex_points[target]);
  Q.push(source, h);
  pmap.insert({source, source});
  dmap.insert({source, 0});
  OutEdgeIterator out_i, out_end;
  double temp_dist = 0;
  // Dijkstra search
  while (!Q.empty()) {
    HeapNode node = Q.top();
    Q.pop();
    NodeIndex u = node.index;
    if (u == target) break;
    for (boost::tie(out_i, out_end) = boost::out_edges(u, g);
         out_i != out_end; ++out_i) {
      EdgeDescriptor e = *out_i;
      NodeIndex v = boost::target(e, g);
      temp_dist = dmap.at(u) + g[e].length;
      h = calc_heuristic_dist(vertex_points[v], vertex_points[target]);
      auto iter = dmap.find(v);
      if (iter != dmap.end()) {
        // dmap contains node v
        if (iter->second > temp_dist) {
          // a smaller distance is found for v
          pmap[v] = u;
          dmap[v] = temp_dist;
          // Unsure if v is still in Q or not
          if (Q.contain_node(v)) {
            Q.decrease_key(v, temp_dist + h);
          } else {
            Q.push(v, temp_dist + h);
          }
        }
      } else {
        // dmap does not contain v
        Q.push(v, temp_dist + h);
        pmap.insert({v, u});
        dmap.insert({v, temp_dist});
      }
    }
  }
  // Backtrack from target to source
  return back_track(source, target, pmap, dmap);
}

std::vector<EdgeIndex> NetworkGraph::back_track(
  NodeIndex source, NodeIndex target, const PredecessorMap &pmap,
  const DistanceMap &dmap) const {
  SPDLOG_TRACE("Backtrack starts");
  if (dmap.find(target) == dmap.end()) {
    return {};
  } else {
    std::vector<EdgeIndex> path;
    NodeIndex v = target;
    NodeIndex u = pmap.at(v);
    while (v != source) {
      double cost = dmap.at(v) - dmap.at(u);
      SPDLOG_TRACE("u {} d {} v {} d {} cost {}",
                   get_node_id(u), dmap.at(u),
                   get_node_id(v), dmap.at(v), cost);
      path.push_back(get_edge_index(u, v, cost));
      v = u;
      u = pmap.at(v);
    }
    std::reverse(path.begin(), path.end());
    return path;
  }
}

/**
 *  Find the edge ID given a pair of nodes and its cost,
 *  if not found, return -1
 */
int NetworkGraph::get_edge_index(NodeIndex source, NodeIndex target,
                                 double cost) const {
  SPDLOG_TRACE("Find edge from {} to {} cost {}", source, target, cost);
  if (source >= num_vertices || target >= num_vertices) return -1;
  EdgeDescriptor e;
  OutEdgeIterator out_i, out_end;
  for (boost::tie(out_i, out_end) = boost::out_edges(source, g);
       out_i != out_end; ++out_i) {
    e = *out_i;
    SPDLOG_TRACE("  Check Edge from {} to {} cost {}",
                 source, boost::target(e, g), g[e].length);
    if (target == boost::target(e, g) &&
        (std::abs(g[e].length - cost) <= DOUBLE_MIN)) {
      SPDLOG_TRACE("  Found edge idx {} id {}",
                   g[e].index, get_edge_id(g[e].index));
      return g[e].index;
    }
  }
  SPDLOG_ERROR("Edge not found");
  return -1;
}

int NetworkGraph::get_edge_index(
  NodeIndex source, NodeIndex target, double *cost) const {
  if (source >= num_vertices || target >= num_vertices) return -1;
  EdgeDescriptor e;
  OutEdgeIterator out_i, out_end;
  int result=-1;
  double current_cost = std::numeric_limits<double>::max();
  for (boost::tie(out_i, out_end) = boost::out_edges(source, g);
       out_i != out_end; ++out_i) {
    e = *out_i;
    SPDLOG_TRACE("  Check Edge from {} to {} cost {}",
                 source, boost::target(e, g), g[e].length);
    if (target == boost::target(e, g) &&
        g[e].length <= current_cost) {
      current_cost = g[e].length;
      *cost = current_cost;
      result = g[e].index;
    }
  }
  if (result<0)
    SPDLOG_ERROR("Edge not found from {} to {}",source,target);
  return result;
};

void NetworkGraph::single_source_upperbound_dijkstra(NodeIndex s,
                                                     double delta,
                                                     PredecessorMap *pmap,
                                                     DistanceMap *dmap) const {
  Heap Q;
  // Initialization
  Q.push(s, 0);
  pmap->insert({s, s});
  dmap->insert({s, 0});
  OutEdgeIterator out_i, out_end;
  double temp_dist = 0;
  // Dijkstra search
  while (!Q.empty()) {
    HeapNode node = Q.top();
    Q.pop();
    NodeIndex u = node.index;
    if (node.value > delta) break;
    for (boost::tie(out_i, out_end) = boost::out_edges(u, g);
         out_i != out_end; ++out_i) {
      EdgeDescriptor e = *out_i;
      NodeIndex v = boost::target(e, g);
      temp_dist = node.value + g[e].length;
      auto iter = dmap->find(v);
      if (iter != dmap->end()) {
        // dmap contains node v
        if (iter->second > temp_dist) {
          // a smaller distance is found for v
          (*pmap)[v] = u;
          (*dmap)[v] = temp_dist;
          Q.decrease_key(v, temp_dist);
        };
      } else {
        // dmap does not contain v
        if (temp_dist <= delta) {
          Q.push(v, temp_dist);
          pmap->insert({v, u});
          dmap->insert({v, temp_dist});
        }
      }
    }
  }
};

BidirectionalNetworkGraph::BidirectionalNetworkGraph(
  const Network &network_arg) : NetworkGraph(network_arg){
  SPDLOG_INFO("Create invert indices for bidirectional graph");
  const std::vector<Edge> &edges = network.get_edges();
  SPDLOG_INFO("Reserve index size {}",num_vertices);
  inverted_indices.resize(num_vertices);
  SPDLOG_INFO("Construct graph from network edges start");
  int N = edges.size();
  for (int i = 0; i < N; ++i) {
    const Edge &edge = edges[i];
    inverted_indices[edge.target].push_back(edge.source);
  }
  SPDLOG_INFO("Create invert indices done");
};

std::vector<EdgeIndex>
BidirectionalNetworkGraph::shortest_path_bidirectional_dijkstra(
  NodeIndex source, NodeIndex target) const {
  SPDLOG_TRACE("Shortest path starts");
  if (source == target) return {};
  Heap fQ,bQ;
  PredecessorMap pmap;
  SuccessorMap smap;
  DistanceMap fdmap,bdmap;
  fQ.push(source, 0);
  pmap.insert({source, source});
  fdmap.insert({source, 0});
  bQ.push(target, 0);
  smap.insert({target, target});
  bdmap.insert({target, 0});
  bool forward_search_flag = true;
  int prev_node = -1; // previous node examined
  while (!bQ.empty() && !fQ.empty()) {
    if (forward_search_flag) {
      HeapNode node = fQ.top();
      fQ.pop();
      NodeIndex u = node.index;
      SPDLOG_INFO("Forward search node {} dist {}",
                  get_node_id(u),node.value);
      if (u==prev_node) break;
      forward_search(&fQ,u,node.value,&pmap,&fdmap);
      forward_search_flag = false;
      prev_node = u;
    } else {
      // backward search
      HeapNode node = bQ.top();
      bQ.pop();
      NodeIndex u = node.index;
      SPDLOG_INFO("Backward search node {} dist {}",
                  get_node_id(u),node.value);
      if (u==prev_node) break;
      backward_search(&bQ,u,node.value,&smap,&bdmap);
      forward_search_flag = true;
      prev_node = u;
    }
  }
  // Backtrack from target to source
  std::vector<EdgeIndex> fpath =
    back_track(source, prev_node, pmap, fdmap);
  std::vector<EdgeIndex> bpath =
    forward_track(prev_node, target, smap, bdmap);
  fpath.insert(fpath.end(),bpath.begin(),bpath.end());
  return fpath;
};

void BidirectionalNetworkGraph::single_target_upperbound_dijkstra(
  NodeIndex target, double delta, SuccessorMap *smap,
  DistanceMap *dmap) const {
  Heap Q;
  // Initialization
  Q.push(target, 0);
  smap->insert({target, target});
  dmap->insert({target, 0});
  double temp_dist = 0;
  // Dijkstra search
  while (!Q.empty()) {
    HeapNode node = Q.top();
    Q.pop();
    NodeIndex u = node.index;
    if (node.value > delta) break;
    backward_search(&Q,u,node.value,smap,dmap);
  }
};

void BidirectionalNetworkGraph::forward_search(
  Heap *Q, NodeIndex u, double dist, PredecessorMap *pmap, DistanceMap *dmap)
const {
  OutEdgeIterator out_i, out_end;
  double temp_dist = 0;
  for (boost::tie(out_i, out_end) = boost::out_edges(u, g);
       out_i != out_end; ++out_i) {
    EdgeDescriptor e = *out_i;
    NodeIndex v = boost::target(e, g);
    temp_dist = dist + g[e].length;
    auto iter = dmap->find(v);
    if (iter != dmap->end()) {
      // dmap contains node v
      if (iter->second > temp_dist) {
        // a smaller distance is found for v
        (*pmap)[v] = u;
        (*dmap)[v] = temp_dist;
        Q->decrease_key(v, temp_dist);
      }
    } else {
      // dmap does not contain v
      Q->push(v, temp_dist);
      pmap->insert({v, u});
      dmap->insert({v, temp_dist});
    }
  }
};

void BidirectionalNetworkGraph::backward_search(
  Heap *Q, NodeIndex v, double dist, SuccessorMap *smap, DistanceMap *dmap)
const {
  double temp_dist = 0;
  const std::vector<NodeIndex> &incoming_nodes = inverted_indices[v];
  for (NodeIndex u:incoming_nodes) {
    double cost = 0;
    int edge_index = get_edge_index(u,v,&cost);
    temp_dist = dist + cost;
    auto iter = dmap->find(u);
    if (iter != dmap->end()) {
      if (iter->second > temp_dist) {
        (*smap)[u] = v;
        (*dmap)[u] = temp_dist;
        Q->decrease_key(u, temp_dist);
      };
    } else {
      Q->push(u, temp_dist);
      smap->insert({u, v});
      dmap->insert({u, temp_dist});
    }
  }
};

std::vector<EdgeIndex> BidirectionalNetworkGraph::forward_track(
  NodeIndex source, NodeIndex target, const SuccessorMap &smap,
  const DistanceMap &dmap) const {
  SPDLOG_TRACE("forward_track starts");
  if (dmap.find(source) == dmap.end()) {
    return {};
  } else {
    std::vector<EdgeIndex> path;
    NodeIndex u = source;
    NodeIndex v = smap.at(u);
    while (u != target) {
      double cost = dmap.at(u) - dmap.at(v);
      SPDLOG_TRACE("u {} d {} v {} d {} cost {}",
                   get_node_id(u), dmap.at(u),
                   get_node_id(v), dmap.at(v), cost);
      path.push_back(get_edge_index(u, v, cost));
      u = v;
      v = smap.at(u);
    }
    return path;
  }
};

std::unordered_set<EdgeID> BidirectionalNetworkGraph::search_edges_within_dist(
  EdgeID eid, double dist) const {
  const Edge& edge = get_edge(eid);
  NodeIndex source = edge.source;
  NodeIndex target = edge.target;
  PredecessorMap pmap;
  DistanceMap fdmap;
  SuccessorMap smap;
  DistanceMap bdmap;
  single_source_upperbound_dijkstra(target, dist, &pmap, &fdmap);
  single_target_upperbound_dijkstra(source, dist, &smap, &bdmap);
  // Find edges in the pmap
  std::unordered_set<EdgeID> result;
  for (auto &item:pmap){
    NodeIndex v = item.first;
    NodeIndex u = item.second;
    double cost = fdmap.at(v)-fdmap.at(u);
    if (u!=v){
      SPDLOG_INFO("Found edge {}",get_edge_id(get_edge_index(u, v, cost)));
      result.insert(get_edge_id(get_edge_index(u, v, cost)));
    }
  }
  for (auto &item:smap){
    NodeIndex u = item.first;
    NodeIndex v = item.second;
    double cost = bdmap.at(u)-bdmap.at(v);
    if (u!=v){
      SPDLOG_INFO("Found edge {}",get_edge_id(get_edge_index(u, v, cost)));
      result.insert(get_edge_id(get_edge_index(u, v, cost)));
    }
  }
  // Find edges in the smap
  return result;
};
