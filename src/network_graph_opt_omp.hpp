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
#ifndef MM_NETWORK_GRAPH_OPT_OMP_HPP
#define MM_NETWORK_GRAPH_OPT_OMP_HPP
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <deque>
#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include "bgl_driving_dist.hpp"
#include <boost/property_map/property_map.hpp>
#include <boost/graph/copy.hpp>
#include "types.hpp"
#include "reader.hpp"
#include "float.h"
#include "network.hpp"
#include <omp.h>
#include <algorithm> // std::reverse
#include <unordered_map>
#include <boost/archive/binary_oarchive.hpp> // Binary output of UBODT
namespace MM {
class NetworkGraphOptOmp
{
public:
    // A infinity value used in the routing algorithm
    static constexpr double DIST_NOT_FOUND = DBL_MAX;
    /**
     *  Construct a network graph from a network object
     */
    NetworkGraphOptOmp(Network *network, std::ofstream &ofstream): m_fstream(ofstream) {
        std::vector<Edge> *edges = network->get_edges();
        std::cout << "Construct graph from network edges start" << '\n';
        // Key is the external ID and value is the index of vertice
        std::unordered_map<int, int> vertex_map;
        int current_idx = -1;
        edge_descriptor e;
        bool inserted;
        g = Graph_T(); //18
        int N = edges->size();
        int source_idx = 0;
        int target_idx = 0;
        // printf("Network edges :%d \n", N);
        for (int i = 0; i < N; ++i) {
            Edge &network_edge = (*edges)[i];
            auto search = vertex_map.find(network_edge.source);
            // Search for source node idx
            if (search != vertex_map.end()) {
                // A node exists already
                source_idx = search->second;
            } else {
                // A new node is found
                ++current_idx;
                vertex_map.insert({network_edge.source, current_idx});
                source_idx = current_idx;
                vertex_id_vec.push_back(network_edge.source);
            };
            // Search for target node idx
            search = vertex_map.find(network_edge.target);
            if (search != vertex_map.end()) {
                // A node exists already
                target_idx = search->second;
            } else {
                // A new node is found
                ++current_idx;
                vertex_map.insert({network_edge.target, current_idx});
                target_idx = current_idx;
                vertex_id_vec.push_back(network_edge.target);
            };
            // boost::tie(e, inserted) = add_edge(network_edge.source, network_edge.target, g);
            boost::tie(e, inserted) = add_edge(source_idx, target_idx, g);
            // id is the FID read, id_attr is the external property in SHP
            g[e].id = network_edge.id;
            g[e].length = network_edge.length;
            //// printf( "Edge read %d,%d,%d,%lf\n",network_edge.id,network_edge.source,network_edge.target,network_edge.length);
        }
        num_vertices = boost::num_vertices(g);
        std::cout << "Graph nodes " << num_vertices << '\n';
        int num_threads = omp_get_num_procs();
        std::cout << "Number of thread available is " << num_threads << '\n';
        // initialize_distances_predecessors(num_threads);
        std::cout << "Construct graph from network edges end" << '\n';
    };
    /**
     * Precompute an UBODT with delta and save it to the file
     * @param filename [description]
     * @param delta    [description]
     */
    void precompute_ubodt(double delta, bool binary = true) {
        int step_size = num_vertices / 10;
        if (step_size < 10) step_size = 10;
        // std::ofstream myfile(filename);
        std::cout << "Start to generate UBODT with delta " << delta << '\n';
        std::cout << "Output format " << (binary ? "binary" : "csv") << '\n';
        m_fstream << "source;target;next_n;prev_n;next_e;distance\n";
        int progress = 0;
        // if (K>50000){
        //     K =50000;
        // }
        // std::cout << "Start to generate UBODT with k " << K << '\n';
        #pragma omp parallel
        {
            double thread_start_time = omp_get_wtime();
            // The copy is not complete here 
            // boost::copy_graph(grid, graph, boost::vertex_copy(detail::grid_to_graph_vertex_copier(grid, graph))
            // .edge_copy(detail::grid_to_graph_edge_copier()));
            // Graph_T g;
            // boost::copy_graph(mg, g);
            std::vector<vertex_descriptor> predecessors_map(num_vertices);
            std::vector<double> distances_map(num_vertices);
            for (int i = 0; i < num_vertices; ++i) {
                distances_map[i] = std::numeric_limits<double>::max();
                predecessors_map[i] = i;
            }
            std::vector<vertex_descriptor> examined_vertices; // Nodes whose distance in the dist_map is updated.
            int thread_process_count=0;
            // If buf placed here, then the result almost doubles
            // Position 1
            // std::stringstream node_output_buf;
            #pragma omp for
            for (int source = 0; source < num_vertices; ++source) {
                std::vector<vertex_descriptor> nodesInDistance;
                examined_vertices.push_back(source);
                double inf = std::numeric_limits<double>::max();
                distances_map[source] = 0;
                // make_iterator_property_map maps the vertex indices vector to predecessors. 
                boost::dijkstra_shortest_paths_upperbound(g,
                        source,
                        make_iterator_property_map(predecessors_map.begin(), get(boost::vertex_index, g), predecessors_map[0]),
                        make_iterator_property_map(distances_map.begin(), get(boost::vertex_index, g), distances_map[0]),
                        get(&Edge_Property::length, g),
                        get(boost::vertex_index, g),
                        std::less<double>(), //DistanceCompare distance_compare,
                        boost::closed_plus<double>(inf),
                        inf,
                        0, delta, nodesInDistance,examined_vertices
                );
                std::vector<vertex_descriptor> successors = get_successors(nodesInDistance, predecessors_map);
                double cost;
                int edge_id;
                int k = 0;
                vertex_descriptor node;
                std::stringstream node_output_buf;
                // Position 2
                while (k < nodesInDistance.size()) {
                    node = nodesInDistance[k];
                    if (source != node) {
                        // The cost is need to identify the edge ID
                        // Position 3
                        //std::stringstream node_output_buf;
                        // node_output_buf << source <<"\n";
                        // It seems the bottleneck is not here
                        cost = distances_map[successors[k]] - distances_map[source];
                        edge_id = get_edge_id(source, successors[k], cost);
                        node_output_buf << vertex_id_vec[source]<< ";" << vertex_id_vec[node]
                         << ";" << vertex_id_vec[successors[k]] << ";" << vertex_id_vec[predecessors_map[node]] 
                         << ";" << edge_id << ";" << distances_map[node] << "\n";
                        //printf('Stringstream size %d\n',node_output_buf.tellg()); 
                    }
                    ++k;
                }
                ++progress;
                if (progress % step_size == 0) {
                    printf("Progress %d / %d \n",progress, num_vertices); 
                }
                // Clean the result
                int N = examined_vertices.size();
                for (int i = 0; i < N; ++i) {
                    vertex_descriptor v = examined_vertices[i];
                    distances_map[v] = std::numeric_limits<double>::max();
                    predecessors_map[v] = v;
                }
                examined_vertices.clear();
                // printf("Write result for node:%d\n",source);
                // std::string s = node_output_buf.str();
                #pragma omp critical
                m_fstream << node_output_buf.str();
                // thread_process_count = thread_process_count+1;
                // if (thread_process_count%5000==0){
                //     printf( "Progress %d taken by thread %d at %f\n",thread_process_count, omp_get_thread_num(), omp_get_wtime()-thread_start_time);
                // }
                //;
            } // end of omp for
            // wtime = omp_get_wtime() - wtime;
            // double thread_end_time = omp_get_wtime();
            // printf( "Time taken by thread %d is %f\n", omp_get_thread_num(), thread_end_time- thread_start_time );
        } // end of omp parallel
        m_fstream.close();
    };
private:
    /* Type definition for the property stored at each edge */
    struct Edge_Property
    {
        int id;
        double length;
    };
    // Boost graph type definition
    typedef boost::adjacency_list <boost::vecS, boost::vecS, boost::directedS, boost::no_property, Edge_Property> Graph_T;
    typedef Graph_T::vertex_descriptor vertex_descriptor;
    typedef Graph_T::edge_descriptor edge_descriptor;
    typedef boost::graph_traits<Graph_T>::vertex_iterator vertex_iterator;
    typedef boost::graph_traits<Graph_T>::out_edge_iterator out_edge_iterator;

    Graph_T g; // The member storing a boost graph
    /**
     *  Find the edge ID given a pair of nodes and its cost
     */
    int get_edge_id(vertex_descriptor source, vertex_descriptor target, double cost) {
        edge_descriptor e;
        out_edge_iterator out_i, out_end;
        for (boost::tie(out_i, out_end) = boost::out_edges(source, g);
                out_i != out_end; ++out_i) {
            e = *out_i; // Can we directly get edge id here or latter from the graph
            if (target == boost::target(e, g) && (g[e].length - cost <= DOUBLE_MIN)) {
                return  g[e].id;
            }
        }
        std::cout << "Edge not found for source " << source << " target " << target
                  << " cost " << cost << '\n';
        return -1;
    };

    /**
     *  Get the successors (next node visited) for each node in a
     *  shortest path tree defined by a deque and a predecessor vector
     */
    std::vector<vertex_descriptor> get_successors(std::vector<vertex_descriptor> &nodesInDistance, std::vector<vertex_descriptor>& predecessors) {
        int N = nodesInDistance.size();
        std::vector<vertex_descriptor> successors = std::vector<vertex_descriptor>(N);
        int i;
        vertex_descriptor u, v;
        vertex_descriptor source = nodesInDistance[0];// source node
        for (i = 0; i < N; ++i) {
            v = nodesInDistance[i];
            while ((u = predecessors[v]) != source) {
                v = u;
            }
            successors[i] = v;
        }
        return successors;
    };
    static constexpr double DOUBLE_MIN = 1.e-6; // This is used for comparing double values
    // Two maps record the routing output
    // The _pool variables are used by OpenMP
    // std::vector<std::vector<vertex_descriptor>> predecessors_map_pool;
    // a list of costs stored for one node to all nodes in the graph
    // std::vector<std::vector<double>> distances_map_pool;
    std::vector<int> vertex_id_vec; // stores the external ID of each vertex in G
    // std::vector<std::vector<vertex_descriptor>> examined_vertices_pool; // Nodes whose distance in the dist_map is updated.
    //std::vector<std::stringstream> stream_pool; // Nodes whose distance in the dist_map is updated.
    std::ofstream &m_fstream;
    int num_vertices = 0;
}; // NetworkGraphOptOmp
} // MM
#endif /* MM_NETWORK_GRAPH_OPT_OMP_HPP */
