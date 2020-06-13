/**
 * Fast map matching.
 *
 * Boost graph types
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */


#ifndef FMM_GRAPH_TYPE_HPP
#define FMM_GRAPH_TYPE_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/property_map/property_map.hpp>

#include "network/type.hpp"

namespace FMM{
namespace NETWORK{

/**
 *  Road edge property.
 */
struct EdgeProperty
{
  EdgeIndex index; /**< Index of the edge */
  double length; /**< length of the edge */
};

/**
 *  Boost graph type
 */
typedef boost::adjacency_list <boost::vecS, boost::vecS, boost::directedS,
                               boost::no_property, EdgeProperty> Graph_T;

/**
 * Boost graph edge type
 */
typedef Graph_T::edge_descriptor EdgeDescriptor;
/**
 * Boost graph node iterator
 */
typedef boost::graph_traits<Graph_T>::vertex_iterator NodeIterator;
/**
 * Boost graph out edge iterator
 */
typedef boost::graph_traits<Graph_T>::out_edge_iterator OutEdgeIterator;

/**
 * Predecessor Map. It stores for each node, the previous node
 * visited, which is part of the shortest path routing result.
 */
typedef std::unordered_map<NodeIndex,NodeIndex> PredecessorMap;

/**
 * Successor Map. It stores for each node, the next node
 * visited, which is part of the bidirectional shortest path routing result.
 */
typedef std::unordered_map<NodeIndex,NodeIndex> SuccessorMap;

/**
 * Distance map. It stores for each node, the distance visited from a source
 * node, which is part of the shortest path routing result.
 */
typedef std::unordered_map<NodeIndex,double> DistanceMap;
}
}

#endif // FMM_GRAPH_TYPE_HPP
