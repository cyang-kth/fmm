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
#include <algorithm>
#include <unordered_map>
// OpenMP
#include <omp.h>

// Binary output of UBODT
#include <boost/archive/binary_oarchive.hpp>

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
    std::vector<Edge> &edges = network->get_edges();
    SPDLOG_INFO("Construct graph from network edges start");
    EdgeDescriptor e;
    bool inserted;
    g = Graph_T();     //18
    int N = edges.size();
    // std::cout<< "Network edges : " << N <<"\n";
    for (int i = 0; i < N; ++i) {
      Edge &edge = edges[i];
      boost::tie(e, inserted) = boost::add_edge(edge.source,edge.target,g);
      // id is the FID read, id_attr is the external property in SHP
      g[e].index = edge.index;
      g[e].length = edge.length;
    }
    num_vertices = boost::num_vertices(g);
    SPDLOG_INFO("Graph nodes {}",num_vertices);
    SPDLOG_INFO("Graph edges {}",boost::num_edges(g));
    SPDLOG_INFO("Construct graph from network edges end");
  }

  /**
   *  Routing from a single source to all nodes within an upperbound
   *  Pmap and dmap are updated to record the result.
   */
  void single_source_upperbound_dijkstra(NodeIndex s,
                                         double delta,
                                         PredecessorMap *pmap,
                                         DistanceMap *dmap){
    Heap Q;
    // Initialization
    Q.push(s,0);
    pmap->insert({s,s});
    dmap->insert({s,0});
    OutEdgeIterator out_i, out_end;
    double temp_dist = 0;
    // Dijkstra search
    while (!Q.empty()) {
      HeapNode node = Q.top();
      Q.pop();
      NodeIndex u = node.index;
      if (node.dist>delta) break;
      for (boost::tie(out_i, out_end) = boost::out_edges(u,g);
           out_i != out_end; ++out_i) {
        EdgeDescriptor e = *out_i;
        NodeIndex v = boost::target(e,g);
        temp_dist = node.dist + g[e].length;
        auto iter = dmap->find(v);
        if (iter!=dmap->end()) {
          // dmap contains node v
          if (iter->second>temp_dist) {
            // a smaller distance is found for v
            (*pmap)[v] = u;
            (*dmap)[v] = temp_dist;
            Q.decrease_key(v,temp_dist);
          };
        } else {
          // dmap does not contain v
          if (temp_dist<=delta){
            Q.push(v,temp_dist);
            pmap->insert({v,u});
            dmap->insert({v,temp_dist});
          }
        }
      }
    }
  }

  /**
   * Precompute an UBODT with upperbound and save it to the file
   * @param filename  output file to write the result
   * @param delta     upper bound
   * @param binary    format of saving ubodt as binary or not
   */
  void precompute_ubodt(const std::string &filename, double delta,
                        bool binary=true) {
    int step_size = num_vertices/10;
    if (step_size<10) step_size=10;
    std::ofstream myfile(filename);
    SPDLOG_INFO("Start to generate UBODT with delta {}",delta);
    SPDLOG_INFO("Output format {}", (binary ? "binary" : "csv"));
    if (binary) {
      boost::archive::binary_oarchive oa(myfile);
      for(NodeIndex source = 0; source < num_vertices; ++source)  {
        if (source%step_size==0)
          SPDLOG_INFO("Progress {} / {}", source, num_vertices);
        PredecessorMap pmap;
        DistanceMap dmap;
        single_source_upperbound_dijkstra(source,delta,&pmap,&dmap);
        write_result_binary(oa,source,pmap,dmap);
      }
    } else {
      myfile << "source;target;next_n;prev_n;next_e;distance\n";
      for(NodeIndex source = 0; source < num_vertices; ++source)  {
        if (source%step_size==0)
          SPDLOG_INFO("Progress {} / {}",source, num_vertices);
        PredecessorMap pmap;
        DistanceMap dmap;
        single_source_upperbound_dijkstra(source,delta,&pmap,&dmap);
        write_result_csv(myfile,source,pmap,dmap);
      }
    }
    myfile.close();
  }

  // Parallelly generate ubodt using OpenMP
  void precompute_ubodt_omp(const std::string &filename, double delta,
                            bool binary=true) {
    int step_size = num_vertices/10;
    if (step_size<10) step_size=10;
    std::ofstream myfile(filename);
    SPDLOG_INFO("Start to generate UBODT with delta {}",delta);
    SPDLOG_INFO("Output format {}", (binary ? "binary" : "csv"));
    if (binary) {
      boost::archive::binary_oarchive oa(myfile);
      int progress = 0;
      #pragma omp parallel
      {
        #pragma omp for
        for(int source = 0; source < num_vertices; ++source) {
          ++progress;
          if (progress % step_size == 0) {
            SPDLOG_INFO("Progress {} / {}", progress, num_vertices);
          }
          PredecessorMap pmap;
          DistanceMap dmap;
          std::stringstream node_output_buf;
          single_source_upperbound_dijkstra(source,delta,&pmap,&dmap);
          write_result_binary(oa,source,pmap,dmap);
        }
      }
    } else {
      myfile << "source;target;next_n;prev_n;next_e;distance\n";
      int progress = 0;
      #pragma omp parallel
      {
        #pragma omp for
        for(int source = 0; source < num_vertices; ++source) {
          ++progress;
          if (progress % step_size == 0) {
            SPDLOG_INFO("Progress {} / {}", progress, num_vertices);
          }
          PredecessorMap pmap;
          DistanceMap dmap;
          std::stringstream node_output_buf;
          single_source_upperbound_dijkstra(source,delta,&pmap,&dmap);
          write_result_csv(myfile,source,pmap,dmap);
        }
      }
    }
    myfile.close();
  }

  /**
   * Write the result of routing from a single source node
   * @param stream output stream
   * @param s      source node
   * @param pmap   predecessor map
   * @param dmap   distance map
   */
  void write_result_csv(std::ostream& stream, NodeIndex s,
                        PredecessorMap &pmap, DistanceMap &dmap){
    NodeIDVec &node_id_vec = network->get_node_id_vec();
    std::vector<Record> source_map;
    for (auto iter = pmap.begin(); iter!=pmap.end(); ++iter) {
      NodeIndex cur_node = iter->first;
      if (cur_node!=s) {
        NodeIndex prev_node = iter->second;
        NodeIndex v = cur_node;
        NodeIndex u;
        while ((u = pmap[v]) != s) {
          v = u;
        }
        NodeIndex successor = v;
        // Write the result to source map
        double cost = dmap[successor];
        EdgeIndex edge_index = get_edge_index(s, successor, cost);
        source_map.push_back(
          {s,
           cur_node,
           successor,
           prev_node,
           edge_index,
           dmap[cur_node],
           nullptr});
      }
    }
    #pragma omp critical
    for (Record &r:source_map) {
      stream << r.source<<";"
             << r.target<<";"
             << r.first_n<<";"
             << r.prev_n<<";"
             << r.next_e<<";"
             << r.cost<<"\n";
    }
  }

  /**
   * Write the result of routing from a single source node in
   * binary format
   *
   * @param stream output stream
   * @param s      source node
   * @param pmap   predecessor map
   * @param dmap   distance map
   */
  void write_result_binary(boost::archive::binary_oarchive& stream,
                           NodeIndex s,
                           PredecessorMap &pmap,
                           DistanceMap &dmap){
    NodeIDVec &node_id_vec = network->get_node_id_vec();
    std::vector<Record> source_map;
    for (auto iter = pmap.begin(); iter!=pmap.end(); ++iter) {
      NodeIndex cur_node = iter->first;
      if (cur_node!=s) {
        NodeIndex prev_node = iter->second;
        NodeIndex v = cur_node;
        NodeIndex u;
        // When u=s, v is the next node visited
        while ((u = pmap[v]) != s) {
          v = u;
        }
        NodeIndex successor = v;
        // Write the result
        double cost = dmap[successor];
        EdgeIndex edge_index = get_edge_index(s, successor, cost);
        source_map.push_back(
          {s,
           cur_node,
           successor,
           prev_node,
           edge_index,
           dmap[cur_node],
           nullptr});
      }
    }
    #pragma omp critical
    for (Record &r:source_map) {
      stream << r.source << r.target
             << r.first_n << r.prev_n <<r.next_e << r.cost;
    }
  }

  Graph_T &get_boost_graph(){
    return g;
  }

  Network *get_network(){
    return network;
  }

  unsigned int get_num_vertices(){
    return num_vertices;
  }

  EdgeIndex get_edge_index(NodeIndex source, NodeIndex target,
                           double dist) {
    EdgeDescriptor e;
    OutEdgeIterator out_i, out_end;
    bool found =false;
    for (boost::tie(out_i, out_end) = boost::out_edges(source, g);
         out_i != out_end; ++out_i) {
      e = *out_i;
      if (target == boost::target(e, g)) {
        found = true;
        if (abs(g[e].length - dist)<=DOUBLE_MIN) {
          break;
        }
      }
    }
    if (found) return g[e].index;
    SPDLOG_ERROR(
      "Edge not found from source {} to target {} dist {}",
      network->get_node_id(source),
      network->get_node_id(target), dist);
    return -1;
  }

private:
  Graph_T g;
  static constexpr double DOUBLE_MIN = 1.e-6;
  Network *network;
  unsigned int num_vertices=0;
};   // NetworkGraph
} // MM
#endif /* MM_NETWORK_GRAPH_HPP */
