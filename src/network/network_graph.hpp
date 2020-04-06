/**
 * Fast map matching.
 *
 * Network graph class
 *
 * The optimization is achieved by recording the output
 * of routing in two variables predecessor map and distance map
 * and regularly updating and cleaning them.
 *
 * It avoids intialization of distances and predecessors vector
 * in each iteration of the driving distance calculation. With a
 * large road network, a small proportion of their nodes are visited
 * in the precomputation. 2018.03.09
 *
 * Add a property map for vertices in the graph to store discontinuous ID
 * for nodes.
 *
 * @author: Can Yang
 * @version: 2018.03.09
 */
#ifndef FMM_NETWORK_GRAPH_HPP
#define FMM_NETWORK_GRAPH_HPP

#include "network/heap.hpp"
#include "network/graph.hpp"
#include "network/network.hpp"

namespace FMM {
namespace NETWORK {
/**
 * Graph class of the network
 */
class NetworkGraph {
 public:
  /**
   *  Construct a network graph from a network
   *  @param network_arg network data
   */
  explicit NetworkGraph(const Network &network_arg);
  /**
   * Dijkstra Shortest path query from source to target
   * @param source
   * @param target
   * @return a vector of edge index representing the path from source to target
   */
  std::vector<EdgeIndex> shortest_path_dijkstra(
      NodeIndex source, NodeIndex target) const;
  /**
   * Calculate heuristic distance from p1 to p2,which is used in Astar routing.
   * @param p1
   * @param p2
   * @return the Euclidean distance from p1 to p2
   */
  double calc_heuristic_dist(
      const CORE::Point &p1, const CORE::Point &p2) const;
  /**
   * AStar Shortest path query from source to target
   * @param source
   * @param target
   * @return a vector of edge index representing the path from source to target
   */
  std::vector<EdgeIndex> shortest_path_astar(NodeIndex source,
                                             NodeIndex target) const;
  /**
   * Backtrack the routing result to find a path from source to target
   * @param source
   * @param target
   * @param pmap predecessor map
   * @param dmap distance map
   * @return a vector of edge index representing the path from source to target
   */
  std::vector<EdgeIndex> back_track(NodeIndex source,
                                    NodeIndex target,
                                    const PredecessorMap &pmap,
                                    const DistanceMap &dmap) const;
  /**
   * Single source shortest path query with an uppper bound
   * @param source source node queried
   * @param delta upper bound to stop early
   * @param pmap predecessor map updated to store the routing result
   * @param dmap distance map updated to store the routing result
   */
  void single_source_upperbound_dijkstra(NodeIndex source,
                                         double delta,
                                         PredecessorMap *pmap,
                                         DistanceMap *dmap) const;
  /**
   *  Find the edge ID given a pair of nodes and its cost,
   *  if not found, return -1
   */
  int get_edge_index(NodeIndex source, NodeIndex target,
                     double cost) const;
  /**
   * Get the edge ID from edge index
   * @param idx edge index
   * @return edge ID
   */
  inline int get_edge_id(EdgeIndex idx) const {
    return network.get_edge_id(idx);
  };
  /**
   * Get edge ID from source node, target node and cost
   * @param source
   * @param target
   * @param cost
   * @return edge ID
   */
  inline int get_edge_id(NodeIndex source, NodeIndex target,
                         double cost) const {
    return network.get_edge_id(get_edge_index(source, target, cost));
  };
  /**
   * Get node ID from a node index
   * @param idx
   * @return node ID
   */
  inline int get_node_id(NodeIndex idx) const {
    return network.get_node_id(idx);
  };
  /**
   * Get node index from node ID
   * @param id node ID
   * @return node index
   */
  inline NodeIndex get_node_index(NodeID id) const {
    return network.get_node_index(id);
  };
  /**
   * Get node point from node index
   * @param u
   * @return node point
   */
  inline const FMM::CORE::Point &get_vertex_point(NodeIndex u) const {
    return network.get_vertex_point(u);
  };
  /**
   * Print graph information
   */
  void print_graph() const;
  /**
   * Get inner boost graph
   * @return graph reference
   */
  const Graph_T &get_boost_graph() const;
  /**
   * Get inner network reference
   * @return reference to the road network
   */
  const Network &get_network();
  /**
   * Get number of vertices in the graph
   * @return number of vertices
   */
  unsigned int get_num_vertices() const;
 protected:
  Graph_T g; /**< The member storing a boost graph */
  /**
   * A value used in checking edge from source,target and cost
   */
  static constexpr double DOUBLE_MIN = 1.e-6;
  const Network &network; /**< Road network */
  unsigned int num_vertices = 0; /**< number of vertices  */
}; // NetworkGraph
}; // NETWORK
} // FMM
#endif /* FMM_NETWORK_GRAPH_HPP */
