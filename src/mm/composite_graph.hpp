#ifndef MM_COMPOSITEGRAPH_HPP
#define MM_COMPOSITEGRAPH_HPP

#include "network/network_graph.hpp"

namespace MM {

// DummyIndex is used in the DummyGraph and CompositeGraph
typedef unsigned int DummyIndex;

class DummyGraph {
public:
  DummyGraph();
  DummyGraph(const Traj_Candidates &traj_candidates);
  Graph_T *get_graph_ptr();

  const Graph_T &get_boost_graph() const;

  int get_num_vertices() const;

  bool containNodeIndex(NodeIndex external_index) const;

  NodeIndex get_external_index(NodeIndex inner_index) const;

  DummyIndex get_internal_index(NodeIndex external_index) const;

  int get_edge_index(NodeIndex source,NodeIndex target,double cost) const;

  void print_node_index_map() const;
protected:
  void add_edge(NodeIndex source, NodeIndex target,
                EdgeIndex edge_index, double cost);
private:
  static constexpr double DOUBLE_MIN = 1e-6;
  Graph_T g;
  std::vector<NodeIndex> external_index_vec;
  std::unordered_map<NodeIndex,DummyIndex> internal_index_map;
};

struct CompEdgeProperty {
  NodeIndex v;
  double cost;
};

class CompositeGraph {
public:
  CompositeGraph(const NetworkGraph &g,const DummyGraph &dg);
  unsigned int get_dummy_node_start_index() const;
  int get_edge_index(NodeIndex u, NodeIndex v, double cost) const;
  EdgeID get_edge_id(NodeIndex u, NodeIndex v, double cost) const;
  std::vector<CompEdgeProperty> out_edges(NodeIndex u) const;
  bool check_dummy_node(NodeIndex u) const;
private:
  const NetworkGraph &g_;
  const DummyGraph &dg_;
  unsigned int num_vertices;
};
}
#endif
