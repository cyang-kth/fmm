#ifndef FMM_GRAPH_TYPE_HPP
#define FMM_GRAPH_TYPE_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/property_map/property_map.hpp>

#include "network/type.hpp"

namespace FMM{
namespace NETWORK{

struct EdgeProperty
{
  EdgeIndex index;
  double length;
};

typedef boost::adjacency_list <boost::vecS, boost::vecS, boost::directedS,
                               boost::no_property, EdgeProperty> Graph_T;

typedef Graph_T::edge_descriptor EdgeDescriptor;
typedef boost::graph_traits<Graph_T>::vertex_iterator NodeIterator;
typedef boost::graph_traits<Graph_T>::out_edge_iterator OutEdgeIterator;

// Data types used for routing
typedef std::unordered_map<NodeIndex,NodeIndex> PredecessorMap;
typedef std::unordered_map<NodeIndex,double> DistanceMap;
}
}

#endif // FMM_GRAPH_TYPE_HPP
