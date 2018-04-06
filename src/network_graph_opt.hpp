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
#ifndef MM_NETWORK_GRAPH_OPT_HPP
#define MM_NETWORK_GRAPH_OPT_HPP
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <deque>
#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/dijkstra_shortest_paths_no_color_map.hpp>
#include <boost/property_map/property_map.hpp>
#include "types.hpp"
#include "reader.hpp"
#include "float.h"
#include "network.hpp"
#include <algorithm> // std::reverse
#include <unordered_map>
#include <boost/archive/binary_oarchive.hpp> // Binary output of UBODT
namespace MM{
class NetworkGraphOpt
{
public:
    // A infinity value used in the routing algorithm
    static constexpr double DIST_NOT_FOUND = DBL_MAX;
    /**
     *  Construct a network graph from a network object
     */
    NetworkGraphOpt(Network *network) {
        std::vector<Edge> *edges = network->get_edges();
        std::cout << "Construct graph from network edges start" << '\n';
        // Key is the external ID and value is the index of vertice
        std::unordered_map<int,int> vertex_map; 
        int current_idx=-1;
        edge_descriptor e;
        bool inserted;
        g = Graph_T(); //18
        int N = edges->size();
        int source_idx = 0;
        int target_idx = 0;
        printf("Network edges :%d \n", N);
        for (int i = 0; i < N; ++i) {
            Edge &network_edge = (*edges)[i];
            auto search = vertex_map.find(network_edge.source);
            // Search for source node idx 
            if(search != vertex_map.end()) {
                // A node exists already
                source_idx = search->second;
            } else {
                // A new node is found
                ++current_idx;
                vertex_map.insert({network_edge.source,current_idx});
                source_idx = current_idx;
                vertex_id_vec.push_back(network_edge.source);
            };
            // Search for target node idx 
            search = vertex_map.find(network_edge.target);
            if(search != vertex_map.end()) {
                // A node exists already
                target_idx = search->second;
            } else {
                // A new node is found
                ++current_idx;
                vertex_map.insert({network_edge.target,current_idx});
                target_idx = current_idx;
                vertex_id_vec.push_back(network_edge.target);
            };
            // boost::tie(e, inserted) = add_edge(network_edge.source, network_edge.target, g);
            boost::tie(e, inserted) = add_edge(source_idx,target_idx, g);
            // id is the FID read, id_attr is the external property in SHP
            g[e].id = network_edge.id;
            g[e].length = network_edge.length;
            //printf( "Edge read %d,%d,%d,%lf\n",network_edge.id,network_edge.source,network_edge.target,network_edge.length);
        }
        num_vertices = boost::num_vertices(g);
        std::cout << "Graph nodes " << num_vertices << '\n';
        initialize_distances_predecessors();
        std::cout << "Construct graph from network edges end" << '\n';
    };
    /**
     * Precompute an UBODT with delta and save it to the file
     * @param filename [description]
     * @param delta    [description]
     */
    void precompute_ubodt(const std::string &filename, double delta, bool binary=true) {
        int step_size = num_vertices/10;
        if (step_size<10) step_size=10;
        std::ofstream myfile(filename);
        std::cout << "Start to generate UBODT with delta " << delta << '\n';
        std::cout << "Output format " << (binary?"binary":"csv") << '\n';
        if (binary){
            boost::archive::binary_oarchive oa(myfile);
            vertex_iterator vi, vend;
            for (boost::tie(vi, vend) = vertices(g); vi != vend; ++vi) {
                if (*vi%step_size==0) std::cout<<"Progress "<<*vi<< " / " << num_vertices <<'\n';
                driving_distance_binary(*vi, delta, oa);
            }
        } else {
            myfile << "source;target;next_n;prev_n;next_e;distance\n";
            vertex_iterator vi, vend;
            for (boost::tie(vi, vend) = vertices(g); vi != vend; ++vi) {
                if (*vi%step_size==0) std::cout<<"Progress "<<*vi<< " / " << num_vertices <<'\n';
                driving_distance_csv(*vi, delta, myfile);
            }
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
            std::vector< double > &distances,
            std::vector< vertex_descriptor > &examined_vertices_ref
        ) : m_distance_goal(distance_goal), m_nodes(nodesInDistance), m_dist(distances),
        m_examined_vertices(examined_vertices_ref) {};
        template <class Graph>void examine_vertex(vertex_descriptor u, Graph &g) {
            DEBUG (2) std::cout << "Examine node " << u << '\n';
            m_nodes.push_back(u);
            if (m_dist[u] > m_distance_goal) {
                m_nodes.pop_back();
                throw found_goals();
            }
        };
        template <class Graph>void edge_relaxed(edge_descriptor e, Graph &g) {
            // Add v to the examined vertices
            DEBUG (2) std::cout << "Examine edge" << e << '\n';
            m_examined_vertices.push_back(boost::target(e, g));
        };
    private:
        double m_distance_goal; //Delta
        std::deque< vertex_descriptor > &m_nodes; //Precedessors
        std::vector< double > &m_dist; // Distances
        std::vector< vertex_descriptor > & m_examined_vertices; //Examined nodes
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
                  << " cost " << cost << '\n';
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
    void driving_distance_csv(const vertex_descriptor& source, double delta, std::ostream& stream) {
        DEBUG (2) std::cout << "Debug progress source " << source << '\n';
        std::deque<vertex_descriptor> nodesInDistance;
        examined_vertices.push_back(source);
        double inf = std::numeric_limits<double>::max();
        distances_map[source]=0;
        // In BGL, http://www.boost.org/doc/libs/1_66_0/boost/graph/dijkstra_shortest_paths_no_color_map.hpp
        // The named parameter version is only defined for dijkstra_shortest_paths_no_color_map
        // Therefore, we need to explicitly pass in the arguments
        // boost::choose_param choose parameter with default value
        try {
            // This part to be fixed
            dijkstra_shortest_paths_no_color_map_no_init(
                g,
                source,
                //boost::predecessor_map(&predecessors_map[0]),
                make_iterator_property_map(predecessors_map.begin(),get(boost::vertex_index, g),predecessors_map[0]),
                //boost::distance_map(boost::make_iterator_property_map(distances_map.begin(), get(boost::vertex_index, g))),
                //&distances_map[0],
                make_iterator_property_map(distances_map.begin(),get(boost::vertex_index, g),distances_map[0]),
                //make_iterator_property_map(distances_map.begin(), boost::vertex_index_map(get(boost::vertex_index, g)),distances_map[0]),
                //boost::distance_map(boost::make_iterator_property_map(distances_map.begin(), get(boost::vertex_index, g))),
                get(&Edge_Property::length, g),
                get(boost::vertex_index, g),
                std::less<double>(), //DistanceCompare distance_compare,
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
        DEBUG (2) std::cout << "Find nodes in distance # "<< nodesInDistance.size() <<'\n';
        std::vector<vertex_descriptor> successors =
            get_successors(nodesInDistance, predecessors_map);
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
                stream << vertex_id_vec[source] << ";" << vertex_id_vec[node] << ";" << vertex_id_vec[successors[k]] << ";"
                       << vertex_id_vec[predecessors_map[node]] << ";" << edge_id << ";" << distances_map[node]
                       << "\n";
                // stream << source << ";" << node << ";" << successors[k] << ";"
                //        << predecessors_map[node] << ";" << edge_id << ";" << distances_map[node]
                //        << "\n";
            }
            ++k;
        }
        DEBUG (2) std::cout << "Clean examined vertices"<< examined_vertices.size() <<'\n';
        clean_distances_predecessors();
    };
    void driving_distance_binary(const vertex_descriptor& source, double delta, boost::archive::binary_oarchive& oa) {
        DEBUG (2) std::cout << "Debug progress source " << source << '\n';
        std::deque<vertex_descriptor> nodesInDistance;
        examined_vertices.push_back(source);
        double inf = std::numeric_limits<double>::max();
        distances_map[source]=0;
        try {
            // This part to be fixed
            dijkstra_shortest_paths_no_color_map_no_init(
                g,
                source,
                make_iterator_property_map(predecessors_map.begin(),get(boost::vertex_index, g),predecessors_map[0]),
                make_iterator_property_map(distances_map.begin(),get(boost::vertex_index, g),distances_map[0]),
                get(&Edge_Property::length, g),
                get(boost::vertex_index, g),
                std::less<double>(), //DistanceCompare distance_compare,
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
        DEBUG (2) std::cout << "Find nodes in distance # "<< nodesInDistance.size() <<'\n';
        std::vector<vertex_descriptor> successors =get_successors(nodesInDistance, predecessors_map);
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
        DEBUG (2) std::cout << "Clean examined vertices"<< examined_vertices.size() <<'\n';
        clean_distances_predecessors();
    };
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
    };
    static constexpr double DOUBLE_MIN = 1.e-6; // This is used for comparing double values
    // Two maps record the routing output
    std::vector<vertex_descriptor> predecessors_map;
    // a list of costs stored for one node to all nodes in the graph
    std::vector<double> distances_map;
    std::vector<int> vertex_id_vec; // stores the external ID of each vertex in G
    std::vector<vertex_descriptor> examined_vertices; // Nodes whose distance in the dist_map is updated.
    int num_vertices=0;
}; // NetworkGraphOpt
} // MM
#endif /* MM_NETWORK_GRAPH_OPT_HPP */
