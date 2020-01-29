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
 * @version: 2020.01.23
 *     Reformat indentation
 *     Change linestring pointer to reference
 * @version: 2018.03.09
 */
#ifndef MM_NETWORK_GRAPH_HPP
#define MM_NETWORK_GRAPH_HPP
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <iostream>
#include <fstream>
#include <deque>
#include <algorithm> // std::reverse
#include <unordered_map>

#include <boost/archive/binary_oarchive.hpp> // Binary output of UBODT

#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/dijkstra_shortest_paths_no_color_map.hpp>

#include "graph_type.hpp"
#include "network.hpp"
#include "heap.hpp"

namespace MM {

class NetworkGraph
{
public:
  /**
   *  Construct a network graph from a network object
   */
  NetworkGraph(Network *network_arg) : network(network_arg) {
    std::vector<Edge> *edges = network->get_edges();
    std::cout << "Construct graph from network edges start" << '\n';
    // Key is the external ID and value is the index of vertice
    NodeIndex current_idx = 0;
    EdgeDescriptor e;
    bool inserted;
    g = Graph_T();     //18
    int N = edges->size();
    // std::cout<< "Network edges : " << N <<"\n";
    for (int i = 0; i < N; ++i) {
      Edge &edge = (*edges)[i];
      boost::tie(e, inserted) = boost::add_edge(edge.source,edge.target,g);
      // id is the FID read, id_attr is the external property in SHP
      g[e].index = edge.index;
      g[e].length = edge.length;
    }
    num_vertices = boost::num_vertices(g);
    std::cout << "Graph nodes " << num_vertices << '\n';
    std::cout << "Graph edges " << boost::num_edges(g) << '\n';
    std::cout << "Construct graph from network edges end" << '\n';
  };

  Graph_T &get_boost_graph(){
    return g;
  };
  Network *get_network(){
    return network;
  };
  unsigned int get_num_vertices(){
    return num_vertices;
  };

  /**
   *  Get the successors (next node visited) for each node in a
   *  shortest path tree defined by a deque and a predecessor vector
   */
  std::vector<vertex_descriptor> get_successors(
    std::deque<vertex_descriptor> &nodesInDistance,
    std::vector<vertex_descriptor> &predecessors) {
    int N = nodesInDistance.size();
    std::vector<vertex_descriptor> successors =
      std::vector<vertex_descriptor>(N);
    int i;
    vertex_descriptor u, v;
    vertex_descriptor source = nodesInDistance[0];    // source node
    for (i = 0; i < N; ++i) {
      v = nodesInDistance[i];
      while ((u = predecessors[v]) != source) {
        v = u;
      }
      successors[i] = v;
    }
    return successors;
  };

  /**
   *  Routing from a single source to all nodes within an upperbound
   *  Results are returned in pmap and dmap.
   */
  void single_source_upperbound_routing(NodeIndex s,
                                        double delta,
                                        PredecessorMap *pmap,
                                        DistanceMap *dmap){
    Heap Q;
    // Initialization
    Q.push({s,0});
    pmap->insert({s,s});
    dmap->insert({s,0});

    OutEdgeIterator out_i, out_end;
    double temp_dist = 0;

    // Search Astar
    while (!Q.empty()) {
      HeapNode &node = Q.top();
      Q.pop();
      NodeIndex u = node.index;
      if (node.dist>delta) break;
      for (boost::tie(out_i, out_end) = boost::out_edges(u,g);
           out_i != out_end; ++out_i) {
        EdgeDescriptor e = *out_i;
        NodeIndex v = boost::target(e,g);
        temp_dist = node.dist + g[e].length;
        // HeapNode node_v{v,temp_dist,temp_tentative_dist};
        auto iter = dmap->find(v);
        if (iter!=dmap->end()) {
          if (iter->second.dist>temp_dist) {
            // There is still need to update the tentative distance
            // because dist is updated.
            (*pmap)[v] = u;
            (*dmap)[v] = temp_dist;
            Q.decrease_key(v,temp_dist);
          };
        } else {
          Q.push({v,temp_dist});
          pmap->insert({v,u});
          dmap->insert({v,temp_dist});
        }
      }
    } // end of while
  }

  void write_result_csv(std::ostream& stream, NodeIndex s,
                        PredecessorMap &pmap, DistanceMap &dmap){
    std::vector<vertex_descriptor> successors =
      get_successors(nodesInDistance, predecessors_map);
    double cost;
    int edge_id;
    int k = 0;
    vertex_descriptor node;
    std::stringstream node_output_buf;
    while (k < nodesInDistance.size()) {
      node = nodesInDistance[k];
      if (source != node) {
        // The cost is need to identify the edge ID
        cost = distances_map[successors[k]] - distances_map[source];
        edge_id = get_edge_id(source, successors[k], cost);
        stream << vertex_id_vec[source] << ";"
               << vertex_id_vec[node] << ";"
               << vertex_id_vec[successors[k]] << ";"
               << vertex_id_vec[predecessors_map[node]] << ";"
               << edge_id << ";" << distances_map[node]
               << "\n";
      }
      ++k;
    }
  }

  void write_result_binary(std::ostream& stream, NodeIndex s,
                           PredecessorMap &pmap, DistanceMap &dmap){

  }


  /**
   * Precompute an UBODT with delta and save it to the file
   * @param filename [description]
   * @param delta    [description]
   */
  void precompute_ubodt(const std::string &filename, double delta,
                        bool binary=true) {
    int step_size = num_vertices/10;
    if (step_size<10) step_size=10;
    std::ofstream myfile(filename);
    std::cout << "Start to generate UBODT with delta " << delta << '\n';
    std::cout << "Output format " << (binary ? "binary" : "csv") << '\n';
    if (binary) {
      boost::archive::binary_oarchive oa(myfile);
      vertex_iterator vi, vend;
      for (boost::tie(vi, vend) = vertices(g); vi != vend; ++vi) {
        if (*vi%step_size==0)
          std::cout<<"Progress "<<*vi<< " / " << num_vertices <<'\n';
        driving_distance_binary(*vi, delta, oa);
      }
    } else {
      myfile << "source;target;next_n;prev_n;next_e;distance\n";
      vertex_iterator vi, vend;
      for (boost::tie(vi, vend) = vertices(g); vi != vend; ++vi) {
        if (*vi%step_size==0)
          std::cout<<"Progress "<<*vi<< " / " << num_vertices <<'\n';
        driving_distance_csv(*vi, delta, myfile);
      }
    }
    myfile.close();
  }


  /**
   * Given a source node and an upper bound distance delta
   * write the UBODT rows to a file
   */
  void driving_distance_csv(const vertex_descriptor& source, double delta,
                            std::ostream& stream) {
    DEBUG (2) std::cout << "Debug progress source " << source << '\n';
    std::deque<vertex_descriptor> nodesInDistance;
    examined_vertices.push_back(source);
    double inf = std::numeric_limits<double>::max();
    distances_map[source]=0;
    // http://www.boost.org/doc/libs/1_66_0/boost/graph/
    // dijkstra_shortest_paths_no_color_map.hpp
    // The named parameter version is only defined for
    // dijkstra_shortest_paths_no_color_map
    // Therefore, we need to explicitly pass in the arguments
    try {
      // This part to be fixed
      dijkstra_shortest_paths_no_color_map_no_init(
        g,
        source,
        make_iterator_property_map(
          predecessors_map.begin(),
          get(boost::vertex_index, g),
          predecessors_map[0]),
        make_iterator_property_map(
          distances_map.begin(),get(boost::vertex_index, g),distances_map[0]),
        get(&Edge_Property::length, g),
        get(boost::vertex_index, g),
        std::less<double>(),         //DistanceCompare distance_compare,
        boost::closed_plus<double>(inf),
        inf,
        0,
        driving_distance_visitor(
          delta, nodesInDistance, distances_map, examined_vertices
          )
        );
    } catch (found_goals& goal) {
      //std::cout << "Found goals" << '\n';
    }
    std::vector<vertex_descriptor> successors =
      get_successors(nodesInDistance, predecessors_map);
    double cost;
    int edge_id;
    int k = 0;
    vertex_descriptor node;
    std::stringstream node_output_buf;
    while (k < nodesInDistance.size()) {
      node = nodesInDistance[k];
      if (source != node) {
        // The cost is need to identify the edge ID
        cost = distances_map[successors[k]] - distances_map[source];
        edge_id = get_edge_id(source, successors[k], cost);
        stream << vertex_id_vec[source] << ";"
               << vertex_id_vec[node] << ";"
               << vertex_id_vec[successors[k]] << ";"
               << vertex_id_vec[predecessors_map[node]] << ";"
               << edge_id << ";" << distances_map[node]
               << "\n";
      }
      ++k;
    }
    clean_distances_predecessors();
  }

  void driving_distance_binary(
    const vertex_descriptor& source, double delta,
    boost::archive::binary_oarchive& oa) {
    std::deque<vertex_descriptor> nodesInDistance;
    examined_vertices.push_back(source);
    double inf = std::numeric_limits<double>::max();
    distances_map[source]=0;
    try {
      // This part to be fixed
      dijkstra_shortest_paths_no_color_map_no_init(
        g,
        source,
        make_iterator_property_map(
          predecessors_map.begin(),
          get(boost::vertex_index, g),predecessors_map[0]),
        make_iterator_property_map(
          distances_map.begin(),
          get(boost::vertex_index, g),distances_map[0]),
        get(&Edge_Property::length, g),
        get(boost::vertex_index, g),
        std::less<double>(),
        boost::closed_plus<double>(inf),
        inf,
        0,
        driving_distance_visitor(
          delta, nodesInDistance, distances_map, examined_vertices
          )
        );
    } catch (found_goals& goal) {
      //std::cout << "Found goals" << '\n';
    }
    // Get successors for each node reached

    std::vector<vertex_descriptor> successors = get_successors(
      nodesInDistance, predecessors_map);
    double cost;
    int edge_id;
    int k = 0;
    vertex_descriptor node;
    while (k < nodesInDistance.size()) {
      node = nodesInDistance[k];
      if (source != node) {
        // The cost is need to identify the edge ID
        cost = distances_map[successors[k]] - distances_map[source];
        edge_id = get_edge_id(source, successors[k], cost);
        oa << vertex_id_vec[source];
        oa << vertex_id_vec[node];
        oa << vertex_id_vec[successors[k]];
        oa << vertex_id_vec[predecessors_map[node]];
        oa << edge_id;
        oa << distances_map[node];
      }
      ++k;
    }
    clean_distances_predecessors();
  }
  /*
     Clean the distance map and predecessor map
   */
  void initialize_distances_predecessors(){
    // Need initialization
    predecessors_map= std::vector<vertex_descriptor>(num_vertices);
    distances_map = std::vector<double>(num_vertices);
    for (int i = 0; i < num_vertices; ++i) {
      distances_map[i] = std::numeric_limits<double>::max();
      predecessors_map[i] = i;
    }

  }
  void clean_distances_predecessors(){
    // Update the properties of examined nodes
    int N = examined_vertices.size();
    for (int i = 0; i < N; ++i) {
      vertex_descriptor v = examined_vertices[i];
      distances_map[v] = std::numeric_limits<double>::max();
      predecessors_map[v] = v;
    }
    examined_vertices.clear();
    // Clear the examined vertices
  }
  // This is used for comparing double values
  static constexpr double DOUBLE_MIN = 1.e-6;
  // Two maps record the routing output
  std::vector<vertex_descriptor> predecessors_map;
  // a list of costs stored for one node to all nodes in the graph
  std::vector<double> distances_map;
  // stores the external ID of each vertex in G
  std::vector<int> vertex_id_vec;
  // Nodes whose distance in the dist_map is updated.
  std::vector<vertex_descriptor> examined_vertices;
private:
  Graph_T g;
  static constexpr double DOUBLE_MIN = 1.e-6;
  Network *network;
  unsigned int num_vertices=0;
};   // NetworkGraph
} // MM
#endif /* MM_NETWORK_GRAPH_HPP */
