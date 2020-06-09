#ifndef FMM_BIDIRECTIONAL_NETWORK_GRAPH_HPP
#define FMM_BIDIRECTIONAL_NETWORK_GRAPH_HPP

#include "network/heap.hpp"
#include "network/graph.hpp"
#include "network/network.hpp"
#include "network/network_graph.hpp"

namespace FMM {
namespace NETWORK {

class BidirectionalNetworkGraph : public NetworkGraph {
public:
  explicit BidirectionalNetworkGraph(const Network &network_arg);
  std::vector<EdgeIndex> shortest_path_bidirectional_dijkstra(
    NodeIndex source, NodeIndex target) const;
  void single_target_upperbound_dijkstra(NodeIndex target,
                                         double delta,
                                         SuccessorMap *smap,
                                         DistanceMap *dmap) const;
  std::vector<EdgeIndex> forward_track(NodeIndex source,
                                       NodeIndex target,
                                       const SuccessorMap &smap,
                                       const DistanceMap &dmap) const;
  void backward_search(
    Heap *Q, NodeIndex v, double dist,
    SuccessorMap *smap, DistanceMap *dmap) const;
  void forward_search(
    Heap *Q, NodeIndex v, double dist,
    PredecessorMap *pmap, DistanceMap *dmap) const;
  std::unordered_set<EdgeID> search_edges_within_dist_from_node(
    NodeIndex source, double dist) const;
  std::unordered_set<EdgeID> search_edges_within_dist_to_node(
    NodeIndex source, double dist) const;
  std::unordered_set<EdgeID> search_edges_within_dist_ft_edge(
    EdgeID eid, double dist) const;
protected:
  std::vector<std::vector<NodeIndex> > inverted_indices;
};

}; // NETWORK
}; // FMM

#endif
