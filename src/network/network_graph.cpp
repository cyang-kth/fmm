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
        if (temp_dist <= delta) {
          Q.push(v, temp_dist);
          pmap->insert({v, u});
          dmap->insert({v, temp_dist});
        }
      }
    }
  }
};
