#include "mm/composite_graph.hpp"
#include "util/debug.hpp"

using namespace FMM;
using namespace FMM::CORE;
using namespace FMM::NETWORK;
using namespace FMM::MM;

DummyGraph::DummyGraph(){}

DummyGraph::DummyGraph(const Traj_Candidates &traj_candidates){
  if (traj_candidates.empty()) return;
  int N = traj_candidates.size();
  std::unordered_map<EdgeIndex,const Candidate*> ca;
  std::unordered_map<EdgeIndex,const Candidate*> cb;
  std::unordered_map<EdgeIndex,const Candidate*> *prev_cmap = &ca;
  std::unordered_map<EdgeIndex,const Candidate*> *cur_cmap = &cb;
  for (int i=0; i<N; ++i) {
    const Point_Candidates &pcs = traj_candidates[i];
    for (const Candidate &c:pcs) {
      NodeIndex n = c.index;
      add_edge(c.edge->source, n, c.edge->index, c.offset);
      add_edge(n,c.edge->target, c.edge->index, c.edge->length - c.offset);
      cur_cmap->insert(std::make_pair(c.edge->index,&c));
      auto iter = prev_cmap->find(c.edge->index);
      if (iter!=prev_cmap->end()) {
        if (iter->second->offset <= c.offset) {
          add_edge(iter->second->index,
                   n, c.edge->index, c.offset-iter->second->offset);
        }
      }
    }
    std::unordered_map<EdgeIndex,const Candidate*> *temp = prev_cmap;
    prev_cmap = cur_cmap;
    cur_cmap = temp;
    cur_cmap->clear();
  }
}

Graph_T *DummyGraph::get_graph_ptr(){
  return &g;
}

const Graph_T &DummyGraph::get_boost_graph() const {
  return g;
}

int DummyGraph::get_num_vertices() const {
  return boost::num_vertices(g);
}

bool DummyGraph::containNodeIndex(NodeIndex external_index) const {
  return internal_index_map.find(external_index)!=internal_index_map.end();
}

NodeIndex DummyGraph::get_external_index(DummyIndex inner_index) const {
  return external_index_vec[inner_index];
}

DummyIndex DummyGraph::get_internal_index(NodeIndex external_index) const {
  return internal_index_map.at(external_index);
}

int DummyGraph::get_edge_index(NodeIndex source,NodeIndex target,double cost)
const {
  SPDLOG_TRACE("Dummy graph get edge index {} {} cost {}",source,target,cost);
  EdgeDescriptor e;
  OutEdgeIterator out_i, out_end;
  NodeIndex source_idx = get_internal_index(source);
  NodeIndex target_idx = get_internal_index(target);
  for (boost::tie(out_i, out_end) = boost::out_edges(source_idx,g);
       out_i != out_end; ++out_i) {
    e = *out_i;
    SPDLOG_TRACE("Target index {} {} id {} e length {} {}",
                 target_idx, boost::target(e,g),
                 g[e].index,
                 g[e].length,
                 std::abs(g[e].length - cost));
    if (target_idx == boost::target(e,g) &&
        (std::abs(g[e].length - cost) <= DOUBLE_MIN)) {
      return g[e].index;
    }
  }
  return -1;
}

void DummyGraph::print_node_index_map() const {
  std::cout<<"Inner index map\n";
  for (auto const& pair:internal_index_map) {
    std::cout << "{" << pair.first << ": " << pair.second << "}\n";
  }
}

void DummyGraph::add_edge(NodeIndex source, NodeIndex target,
                          EdgeIndex edge_index, double cost) {
  // SPDLOG_TRACE("  Add edge {} {} e {} cost {}",
  //               source,target,edge_index,cost);
  auto search1 = internal_index_map.find(source);
  auto search2 = internal_index_map.find(target);
  DummyIndex source_idx, target_idx;
  if (search1!=internal_index_map.end()) {
    source_idx = search1->second;
  } else {
    source_idx = external_index_vec.size();
    external_index_vec.push_back(source);
    internal_index_map.insert({source,source_idx});
  }
  if (search2!=internal_index_map.end()) {
    target_idx = search2->second;
  } else {
    target_idx = external_index_vec.size();
    external_index_vec.push_back(target);
    internal_index_map.insert({target,target_idx});
  }
  EdgeDescriptor e;
  bool inserted;
  boost::tie(e, inserted) = boost::add_edge(source_idx,target_idx,g);
  // id is the FID read, id_attr is the external property in SHP
  g[e].index = edge_index;
  g[e].length = cost;
}

CompositeGraph::CompositeGraph(const NetworkGraph &g,const DummyGraph &dg) :
  g_(g),dg_(dg){
  num_vertices = g_.get_num_vertices();
}

unsigned int CompositeGraph::get_dummy_node_start_index() const {
  return num_vertices;
}

int CompositeGraph::get_edge_index(NodeIndex u, NodeIndex v, double cost)
const {
  if (u >= num_vertices || v >= num_vertices) {
    return dg_.get_edge_index(u,v,cost);
  } else {
    return g_.get_edge_index(u,v,cost);
  }
}

EdgeID CompositeGraph::get_edge_id(NodeIndex u, NodeIndex v, double cost)
const {
  return g_.get_edge_id(get_edge_index(u,v,cost));
}

std::vector<CompEdgeProperty> CompositeGraph::out_edges(NodeIndex u) const {
  std::vector<CompEdgeProperty> out_edges;
  OutEdgeIterator out_i, out_end;
  EdgeDescriptor e;
  if (dg_.containNodeIndex(u)) {
    const Graph_T &dg = dg_.get_boost_graph();
    NodeIndex u_internal = dg_.get_internal_index(u);
    for (boost::tie(out_i, out_end) = boost::out_edges(u_internal,dg);
         out_i != out_end; ++out_i) {
      e = *out_i;
      NodeIndex v_internal = boost::target(e, dg);
      NodeIndex v = dg_.get_external_index(v_internal);
      out_edges.push_back(CompEdgeProperty{v,dg[e].length});
    }
  }
  if (u < num_vertices) {
    const Graph_T &g = g_.get_boost_graph();
    for (boost::tie(out_i, out_end)=boost::out_edges(u,g);
         out_i != out_end; ++out_i) {
      e = *out_i;
      NodeIndex v = boost::target(e, g);
      // Export a pair of (v,cost)
      out_edges.push_back(CompEdgeProperty{v,g[e].length});
    }
  }
  return out_edges;
}

bool CompositeGraph::check_dummy_node(NodeIndex u) const {
  return u>=num_vertices;
}
