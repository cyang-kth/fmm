/**
 * Content
 * An optimizated network graph class
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
#ifndef MM_NETWORK_GRAPH_HPP
#define MM_NETWORK_GRAPH_HPP

#include "network/heap.hpp"
#include "network/graph.hpp"
#include "network/network.hpp"

namespace MM {

class NetworkGraph
{
public:
  /**
   *  Construct a network graph from a network object
   */
  explicit NetworkGraph(const Network &network_arg);
  std::vector<EdgeIndex> shortest_path_dijkstra(NodeIndex source,
                                                NodeIndex target) const;
  // this function is used in AStar routing
  double calc_heuristic_dist(
    const Point &p1,const Point &p2) const;
  std::vector<EdgeIndex> shortest_path_astar(NodeIndex source,
                                             NodeIndex target) const;
  std::vector<EdgeIndex> back_track(NodeIndex source, NodeIndex target,
                                    const PredecessorMap &pmap,
                                    const DistanceMap&dmap) const;
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
  inline int get_edge_id(EdgeIndex idx) const {
    return network.get_edge_id(idx);
  };
  inline int get_edge_id(NodeIndex source, NodeIndex target,
                         double cost) const {
    return network.get_edge_id(get_edge_index(source,target,cost));
  };
  inline int get_node_id(NodeIndex idx) const{
    return network.get_node_id(idx);
  };
  inline NodeIndex get_node_index(NodeID id) const{
    return network.get_node_index(id);
  };
  inline const Point &get_vertex_point(NodeIndex u) const {
    return network.get_vertex_point(u);
  };
  void print_graph() const;
  const Graph_T &get_boost_graph() const;
  const Network &get_network();
  unsigned int get_num_vertices() const;
protected:
  // Boost graph type definition
  Graph_T g;   // The member storing a boost graph
  static constexpr double DOUBLE_MIN = 1.e-6;
  const Network &network;
  unsigned int num_vertices=0;
}; // NetworkGraph

} // MM
#endif /* MM_NETWORK_GRAPH_HPP */
