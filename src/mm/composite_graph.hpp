/**
 * Fast map matching.
 *
 * Composition graph
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */


#ifndef FMM_COMPOSITEGRAPH_HPP
#define FMM_COMPOSITEGRAPH_HPP

#include "network/network_graph.hpp"

namespace FMM {

namespace MM {

/**
 * This is an index used in the dummy graph.
 */
typedef unsigned int DummyIndex;

/**
 * A graph containing dummy nodes and edges used in map matching.
 * It connects candidate node (dummy node) matched to GPS observations
 * with the nodes in the original road network. The connected edges
 * are dummy edges.
 */
class DummyGraph {
 public:
  /**
   * Default constructor of dummy graph
   */
  DummyGraph();
  /**
   * Constructor of dummy graph from trajectory candidates.
   *
   * Each candidate will be connected with two end nodes of the matched
   * edge.
   *
   * @param traj_candidates input information
   */
  DummyGraph(const Traj_Candidates &traj_candidates);

  /**
   * Get the inner graph data
   * @return A pointer to the inner boost graph.
   */
  NETWORK::Graph_T *get_graph_ptr();

  /**
   * Get a const reference to the inner graph data
   * @return A pointer to the inner boost graph.
   */
  const NETWORK::Graph_T &get_boost_graph() const;

  /**
   * Get the number of vertices in the dummy graph
   */
  int get_num_vertices() const;
  /**
   * Check if a node is contained in the dummy graph
   *
   * A node is contained in the dummy graph if (a) it is a
   * dummy node representing a candidate or (b) it is the end
   * node of a matched candidate edge.
   *
   * @param  external_index The NodeIndex of a node
   * @return true if a node is contained
   */
  bool containNodeIndex(NETWORK::NodeIndex external_index) const;

  /**
   * Get the NodeIndex of a node according to the inner index of the
   * dummy graph
   * @param  inner_index an inner index of the dummy graph
   * @return a node index of the node in the original network graph
   */
  NETWORK::NodeIndex get_external_index(DummyIndex inner_index) const;

  /**
   * Get the internal index of a node in dummy graph
   * If the node is not contained in the dummy graph, an exception will be
   * thrown. Call the containNodeIndex before invoking this function.
   *
   * @param  external_index The node index in original network graph
   * @return  a internal index
   *
   */
  DummyIndex get_internal_index(NETWORK::NodeIndex external_index) const;
  /**
   * Get the edge index in the original network graph.
   * @param  source source NodeIndex in the original network graph
   * @param  target target NodeIndex in the original network graph
   * @param  cost   Cost value of the edge
   * @return  the corresponding edge index in the origin network graph. If
   * edge is not found, -1 is returned.
   */
  int get_edge_index(NETWORK::NodeIndex source,
                     NETWORK::NodeIndex target, double cost) const;
  /**
   * Print the mapping from dummy index to node index
   */
  void print_node_index_map() const;
 protected:
  /**
   * Add an edge to the dummy graph
   * @param source source node index
   * @param target target node index
   * @param edge_index Edge index. It will be the same edge index where
   * the dummy edge is located.
   * @param cost cost of the edge
   */
  void add_edge(NETWORK::NodeIndex source, NETWORK::NodeIndex target,
                NETWORK::EdgeIndex edge_index, double cost);
 private:
  static constexpr double DOUBLE_MIN = 1e-6;
  NETWORK::Graph_T g;
  std::vector<NETWORK::NodeIndex> external_index_vec;
  std::unordered_map<NETWORK::NodeIndex, DummyIndex> internal_index_map;
};

/**
 * Property of an edge in the composite graph
 */
struct CompEdgeProperty {
  NETWORK::NodeIndex v; /**< Target node index */
  double cost; /**< Cost of an edge */
};

/**
 * Composite Graph as a wrapper of network graph and dummy graph.
 */
class CompositeGraph {
 public:
  /**
   * Constructor
   * @param g  A network graph
   * @param dg A dummy graph
   */
  CompositeGraph(const NETWORK::NetworkGraph &g,
                 const DummyGraph &dg);
  /**
   * Get the starting node index corresponding to the
   * the first dummy node in the dummy graph.
   */
  unsigned int get_dummy_node_start_index() const;
  /**
   * Get edge index from node index u,v and cost. If
   * edge is not found, -1 is returned.
   */
  int get_edge_index(NETWORK::NodeIndex u,
                     NETWORK::NodeIndex v, double cost) const;
  /**
   * Get edge id from node index u,v and cost
   */
  NETWORK::EdgeID get_edge_id(NETWORK::NodeIndex u,
                     NETWORK::NodeIndex v, double cost) const;
  /**
   * Get out edges leaving a node u in the composite graph
   */
  std::vector<CompEdgeProperty> out_edges(NETWORK::NodeIndex u) const;
  /**
   * Check if a node u is dummy node, namely representing
   * a candidate point
   */
  bool check_dummy_node(NETWORK::NodeIndex u) const;
 private:
  const NETWORK::NetworkGraph &g_;
  const DummyGraph &dg_;
  unsigned int num_vertices;
};
}
}
#endif
