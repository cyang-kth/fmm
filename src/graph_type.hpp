/**
 * Definition of graph types
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */

#ifndef MM_GRAPH_TYPE_HPP
#define MM_GRAPH_TYPE_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/property_map/property_map.hpp>

#include "types.hpp"

namespace MM{

struct EdgeProperty
{
  EdgeIndex index;
  double length;
};

typedef boost::adjacency_list <boost::vecS, boost::vecS, boost::directedS,
                               boost::no_property, EdgeProperty> Graph_T;

// The EdgeDescriptor is different from EdgeIndex, it can be used
// to access the edge property of a graph as g[e].property
typedef Graph_T::edge_descriptor EdgeDescriptor;
typedef boost::graph_traits<Graph_T>::vertex_iterator NodeIterator;
typedef boost::graph_traits<Graph_T>::out_edge_iterator OutEdgeIterator;

// Data types used for routing
typedef std::unordered_map<NodeIndex,NodeIndex> PredecessorMap;
typedef std::unordered_map<NodeIndex,double> DistanceMap;

}

#endif // MM_GRAPH_TYPE_HPP
