/**
 * This class is deprecated, use NetworkGraphOpt instead, which 
 * is optimized for large road network and supports discontinuous
 * node ID.
 * 
 * Content
 * Definition of the NetworkGraph class
 *      
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef MM_NETWORK_GRAPH_HPP
#define MM_NETWORK_GRAPH_HPP
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <deque>
#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>
#include "types.hpp"
#include "reader.hpp"
#include "float.h"
#include "network.hpp"
#include <algorithm> // std::reverse
namespace MM {
class NetworkGraph
{
public:
    // A infinity value used in the routing algorithm
    static constexpr double DIST_NOT_FOUND = DBL_MAX;
    /**
     *  Construct a network graph from a network object
     */
    NetworkGraph(Network *network) {
        std::vector<Edge> *edges = network->get_edges();
        std::cout << "Construct graph from network edges start" << std::endl;
        edge_descriptor e;
        bool inserted;
        g = Graph_T(); //18
        int N = edges->size();
        printf("Network edges :%d \n", N);
        for (int i = 0; i < N; ++i) {
            Edge &network_edge = (*edges)[i];
            boost::tie(e, inserted) = add_edge(network_edge.source, network_edge.target, g);
            // id is the FID read, id_attr is the external property in SHP
            g[e].id = network_edge.id;
            g[e].length = network_edge.length;
            //printf( "Edge read %d,%d,%d,%lf\n",network_edge.id,network_edge.source,network_edge.target,network_edge.length);
        }
        num_vertices = boost::num_vertices(g);
        std::cout << "Graph nodes " << num_vertices << std::endl;
        std::cout << "Construct graph from network edges end" << std::endl;
    };
    /**
     * Precompute an UBODT with delta and save it to the file
     * @param filename [description]
     * @param delta    [description]
     */
    void precompute_ubodt(const std::string &filename, double delta) {
        std::ofstream myfile(filename);
        std::cout << "Start to generate UBODT with delta " << delta << std::endl;
        int step_size = num_vertices/20;
        if (step_size<10) step_size=10;
        //int num_vertices = vertices(g);
        myfile << "source;target;next_n;prev_n;next_e;distance\n";
        vertex_iterator vi, vend;
        for (boost::tie(vi, vend) = vertices(g); vi != vend; ++vi) {
            if (*vi%step_size==0) std::cout<<"Progress "<<*vi<< " / " << num_vertices <<std::endl;
            driving_distance(*vi, delta, myfile);
        }
        myfile.close();
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
    struct found_goals {}; // Used for driving distances
    /**
     * The visitor is an inner class whose function examine_vertex()
     * is called whenever a new node is found in conventional Dijkstra
     * algorithm.
     *
     * It is called in the driving_distance function.
     */
    class driving_distance_visitor : public boost::default_dijkstra_visitor {
    public:
        // Create a visitor 
        explicit driving_distance_visitor(
            double distance_goal,
            std::deque< vertex_descriptor > &nodesInDistance,
            std::vector< double > &distances
        ) : m_distance_goal(distance_goal), m_nodes(nodesInDistance), m_dist(distances) {};
        template <class Graph>void examine_vertex(vertex_descriptor u, Graph &g) {
            m_nodes.push_back(u);
            if (m_dist[u] > m_distance_goal) {
                m_nodes.pop_back();
                throw found_goals();
            }
        };
    private:
        double m_distance_goal; //Delta
        std::deque< vertex_descriptor > &m_nodes; //Precedessors
        std::vector< double > &m_dist; // Distances
    }; // driving_distance_visitor
    
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
                  << " cost " << cost << std::endl;
        return -1;
    };
    
    /**
     *  Get the successors (next node visited) for each node in a 
     *  shortest path tree defined by a deque and a predecessor vector
     */
    std::vector<vertex_descriptor> get_successors(std::deque<vertex_descriptor> &nodesInDistance, std::vector<vertex_descriptor>& predecessors) {
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
    /**
     * Given a source node and an upper bound distance delta 
     * write the UBODT rows to a file
     */
    void driving_distance(const vertex_descriptor& source, double delta, std::ostream& stream) {
        // The two lines below can be problematic when the network is large but delta is small.
        // check the network_graph_opt.hpp for a solution
        // 
        // The implementation is based on 
        // https://github.com/pgRouting/pgrouting-build/blob/44deced99d05617a41948eec07d0b22c7f236cbf/src/dijkstra/src/pgr_dijkstra.hpp#L508
        // 
        // std::cout << "Progress source " << source << std::endl;
        std::vector<vertex_descriptor> predecessors(num_vertices);
        // a list of costs stored for one node to all nodes in the graph
        std::vector<double> distances(num_vertices);
        std::deque<vertex_descriptor> nodesInDistance;
        // http://www.boost.org/doc/libs/1_66_0/boost/graph/dijkstra_shortest_paths.hpp
        // The function below is defined as a Named Parameter Variant
        try {
            dijkstra_shortest_paths(
                g,
                source,
                weight_map(get(&Edge_Property::length, g)).
                predecessor_map(&predecessors[0]).
                distance_map(&distances[0]).
                visitor(
                    driving_distance_visitor(
                        delta, nodesInDistance, distances
                    )
                )
            );
        } catch (found_goals& goal) {
            //std::cout << "Found goals" << std::endl;
        }
        // Get successors for each node reached
        std::vector<vertex_descriptor> successors =
            get_successors(nodesInDistance, predecessors);
        double cost;
        int edge_id;
        int k = 0;
        vertex_descriptor node;
        while (k < nodesInDistance.size()) {
            node = nodesInDistance[k];
            if (source != node) {
                // The cost is need to identify the edge ID
                cost = distances[successors[k]] - distances[source];
                edge_id = get_edge_id(source, successors[k], cost);
                stream << source << ";" << node << ";" << successors[k] << ";"
                       << predecessors[node] << ";" << edge_id << ";" << distances[node]
                       << "\n";
            }
            ++k;
        }
    };
    static constexpr double DOUBLE_MIN = 1.e-6; // This is used for comparing double values
    int num_vertices=0;
}; // NetworkGraph
} // MM
#endif /* MM_NETWORK_GRAPH_HPP */
