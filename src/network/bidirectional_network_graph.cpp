#include "network/bidirectional_network_graph.hpp"
#include "network/heap.hpp"
#include "network/network.hpp"
#include "network/network_graph.hpp"
#include "util/debug.hpp"

#include <cmath>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <queue>

using namespace FMM;
using namespace FMM::CORE;
using namespace FMM::NETWORK;

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
      if (u==prev_node) break;
      forward_search(&fQ,u,node.value,&pmap,&fdmap);
      forward_search_flag = false;
      prev_node = u;
    } else {
      // backward search
      HeapNode node = bQ.top();
      bQ.pop();
      NodeIndex u = node.index;
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
      path.push_back(get_edge_index(u, v, cost));
      u = v;
      v = smap.at(u);
    }
    return path;
  }
};

std::unordered_set<EdgeID>
BidirectionalNetworkGraph::search_edges_within_dist_from_node(
  NodeIndex source, double delta) const {
  std::unordered_set<EdgeID> result;
  Heap Q;
  DistanceMap dmap;
  Q.push(source, 0);
  dmap.insert({source, 0});
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
      EdgeID eid = get_edge_id(g[e].index);
      auto iter = dmap.find(v);
      if (iter != dmap.end()) {
        // dmap contains node v
        if (iter->second > temp_dist) {
          // a smaller distance is found for v
          dmap[v] = temp_dist;
          Q.decrease_key(v, temp_dist);
        };
        if (temp_dist<=delta){
          result.insert(eid);
        }
      } else {
        if (temp_dist <= delta) {
          Q.push(v, temp_dist);
          dmap.insert({v, temp_dist});
          result.insert(eid);
        }
      }
    }
  }
  return result;
};

std::unordered_set<EdgeID>
BidirectionalNetworkGraph::search_edges_within_dist_to_node(
  NodeIndex target, double delta) const {
  std::unordered_set<EdgeID> result;
  Heap Q;
  DistanceMap dmap;
  Q.push(target, 0);
  dmap.insert({target, 0});
  OutEdgeIterator out_i, out_end;
  double temp_dist = 0;
  // Dijkstra search
  while (!Q.empty()) {
    HeapNode node = Q.top();
    Q.pop();
    NodeIndex v = node.index;
    if (node.value > delta) break;
    const std::vector<NodeIndex> &incoming_nodes = inverted_indices[v];
    for (NodeIndex u:incoming_nodes) {
      double cost = 0;
      // Cost is updated in the command below
      int edge_index = get_edge_index(u,v,&cost);
      EdgeID eid = get_edge_id(edge_index);
      double temp_dist = node.value + cost;
      auto iter = dmap.find(u);
      if (iter != dmap.end()) {
        if (iter->second > temp_dist) {
          dmap[u] = temp_dist;
          Q.decrease_key(u, temp_dist);
        };
        if (temp_dist<=delta){
          result.insert(eid);
        }
      } else {
        if (temp_dist <= delta) {
          Q.push(u, temp_dist);
          dmap.insert({u, temp_dist});
          result.insert(eid);
        }
      }
    }
  }
  return result;
};

std::unordered_set<EdgeID>
BidirectionalNetworkGraph::search_edges_within_dist_ft_edge(
  EdgeID eid, double dist) const {
  const Edge& edge = get_edge(eid);
  NodeIndex source = edge.source;
  NodeIndex target = edge.target;
  std::unordered_set<EdgeID> result =
    search_edges_within_dist_from_node(target,dist);
  std::unordered_set<EdgeID> temp =
    search_edges_within_dist_to_node(source,dist);
  result.insert (temp.begin(), temp.end());
  return result;
};
