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
#include "../src/types.hpp"
#include "../src/reader.hpp"
#include "float.h"
#include "../src/network.hpp"
#include <algorithm> // std::reverse
namespace MM {
class NetworkGraph
{
public:
    static constexpr double DIST_NOT_FOUND = DBL_MAX;
    /**
     *  Note here, if we construct config with config = NetworkGraphConfig(filename);
     *  it will invoke the default constructor with no argument first, as it is
     *  equivalent to first initialize config then update its content.
     */
    NetworkGraph(Network *network) {
        std::vector<Edge> *edges = network->get_edges();
        std::cout << "Construct graph from network edges start" << '\n';
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
        int Npoints = boost::num_vertices(g);
        std::cout << "Graph nodes " << Npoints << '\n';
        std::cout << "Construct graph from network edges end" << '\n';
    };
    /**
     * Precompute an UBODT with delta and save it to the file
     * @param filename [description]
     * @param delta    [description]
     */
    void precompute_ubodt(const std::string &filename, double delta) {
        std::ofstream myfile(filename);
        std::cout << "Start to generate UBODT with delta " << delta << '\n';
        myfile << "source;target;next_n;last_n;next_e;distance\n";
        vertex_iterator vi, vend;
        for (boost::tie(vi, vend) = vertices(g); vi != vend; ++vi) {
            driving_distance(*vi, delta, myfile);
        }
        myfile.close();
    };
    /**
     * Find shortest path distance from source to target
     * @param  source [description]
     * @param  target [description]
     * @return        [description]
     *
     * In case that a shortest path is not found from source
     * to target, an extremely large value is returned as
     * NO_GAP_DISTANCE.
     *
     * http://www.boost.org/doc/libs/1_55_0/libs/graph/example/dijkstra-example.cpp
     *
     */
    double single_source_target_distance(int source, int target) {
        ++calling_times;
        bool found = false;
        std::vector<vertex_descriptor> predecessors(num_vertices(g));
        std::vector<double> distances(num_vertices(g));
        try {
            dijkstra_shortest_paths(
                g,
                source,
                weight_map(get(&Edge_Property::length, g)).
                predecessor_map(&predecessors[0]).
                distance_map(&distances[0]).
                visitor(one_to_one_distance_visitor(target))
            );
        } catch (found_goals& goal) {
            //std::cout << "Found goals" << '\n';
            found = true;
        }
        if (found) {
            return distances[target];
        } else {
            return DIST_NOT_FOUND;
        };
    };
    /**
     *  Return a shortest path from source to target as a vector of
     *  edge ids. If no path is found, an empty vector is returned.
     *
     */
    std::vector<int> single_source_target_path(int source, int target) {
        ++calling_times;
        bool found = false;
        std::vector<vertex_descriptor> predecessors(num_vertices(g));
        std::vector<double> distances(num_vertices(g));
        try {
            dijkstra_shortest_paths(
                g,
                source,
                weight_map(get(&Edge_Property::length, g)).
                predecessor_map(&predecessors[0]).
                distance_map(&distances[0]).
                visitor(
                    one_to_one_distance_visitor(target)
                )
            );
        } catch (found_goals& goal) {
            //std::cout << "Found goals" << '\n';
            found = true;
        }
        if (found) {
            // Return the sp path from the result
            std::vector<int> path;
            int current_node = target;
            while (current_node != source) {
                double cost = distances[current_node] - distances[predecessors[current_node]];
                int edge_id = get_edge_id(predecessors[current_node], current_node, cost);
                path.push_back(edge_id);
                current_node = predecessors[current_node];
            }
            std::reverse(path.begin(), path.end());
            return path;
        } else {
            return std::vector<int>();
        };
    };
    /**
     * Find the routing with the first and last node
     */
    bool single_source_target_with_first_prev_node(int source, int target,int *first_n,int *prev_n,double *cost) {
        bool found = false;
        std::vector<vertex_descriptor> predecessors(num_vertices(g));
        std::vector<double> distances(num_vertices(g));
        try {
            dijkstra_shortest_paths(
                g,
                source,
                weight_map(get(&Edge_Property::length, g)).
                predecessor_map(&predecessors[0]).
                distance_map(&distances[0]).
                visitor(
                    one_to_one_distance_visitor(target)
                )
            );
        } catch (found_goals& goal) {
            //std::cout << "Found goals" << '\n';
            found = true;
        }
        if (found) {
            // Return the sp path from the result
            int current_node = target;
            *prev_n =predecessors[target];
            while (current_node != source) {
                *first_n = current_node;
                current_node = predecessors[current_node];
            }
            *cost = distances[target];
        } else {
            *prev_n = -1;
            *first_n = -1;
            *cost = DIST_NOT_FOUND;
        };
        return found;
    };
    long int get_number_queries(){
        return calling_times;
    };
    C_Path *construct_complete_path_routing(O_Path *path)
    {
        if (path==nullptr) return nullptr;
        C_Path *edges= new C_Path();
        int N = path->size();
        edges->push_back((*path)[0]->edge->id);
        for(int i=0; i<N-1; ++i)
        {
            Candidate* a = (*path)[i];
            Candidate* b = (*path)[i+1];
            if ((a->edge->id!=b->edge->id) || (a->offset>b->offset))
            {
                auto segs = single_source_target_path(a->edge->target,b->edge->source);
                // No path exist.
                if (segs.empty() &&  a->edge->target!=b->edge->source)
                {
                    delete edges; // free the memory of edges
                    return nullptr;
                }
                for (int e:segs)
                {
                    edges->push_back(e);
                }
                edges->push_back(b->edge->id);
            }
        }
        return edges;
    };
private:
    /* Type definition for property stored at each edge */
    struct Edge_Property
    {
        int id;
        double length;
    };
    // No vertex property, but has edge property
    typedef boost::adjacency_list <boost::vecS, boost::vecS, boost::directedS, boost::no_property, Edge_Property> Graph_T; // graph type
    typedef Graph_T::vertex_descriptor vertex_descriptor;
    typedef Graph_T::edge_descriptor edge_descriptor;
    typedef boost::graph_traits<Graph_T>::vertex_iterator vertex_iterator;
    typedef boost::graph_traits<Graph_T>::out_edge_iterator out_edge_iterator;
    struct found_goals {}; // Used for driving distances
    /**
     *  This visitor is used for finding the shortest path
     *  distance from a single source to single target.
     */
    class one_to_one_distance_visitor : public boost::default_dijkstra_visitor {
    public:
        /**
         *  The distances field is also passed to the graph as a distance map
         *  that gets updated outside of the visitor class.
         */
        explicit one_to_one_distance_visitor(
            vertex_descriptor V
        ) : m_V(V) {};
        template <class Graph>void examine_vertex(vertex_descriptor u, Graph &g) {
            // Reach V in the routing process
            if (u == m_V) throw found_goals();
        };
    private:
        vertex_descriptor m_V; // target node
    };
    /*
    class one_to_many_visitor : public boost::default_dijkstra_visitor {
    public:
        explicit one_to_many_visitor(std::vector<vertex_descriptor> goals)
            : m_goals(goals.begin(), goals.end()) {}
        template <class Graph>
        void examine_vertex(vertex_descriptor u, Graph &g) {
            auto s_it = m_goals.find(u);
            if (s_it == m_goals.end()) return;
            // we found one more goal
            m_goals.erase(s_it);
            // We have found all goals
            if (m_goals.size() == 0) throw found_goals();
        }
    private:
        std::set<vertex_descriptor> m_goals;
    };
    */
    /**
     * The visitor is a class whose function examine_vertex() is called
     * whenever a new node is checked in conventional Dijkstra
     * algorithm.
     *
     * It is called in the driving distance function.
     */
    class driving_distance_visitor : public boost::default_dijkstra_visitor {
    public:
        /**
         *  The distances field is also passed to the graph as a distance map
         *  that gets updated outside of the visitor class.
         */
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
    /*  Public members and methods */
    // NetworkGraphConfig config;
    Graph_T g;
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
    // The reason that template is used here as we want to write to both std or to a file
    void driving_distance(const vertex_descriptor& source, double delta, std::ostream& stream) {
        if (source % 5000 == 0) {
            std::cout << "Progress source " << source << '\n';
        }
        // std::cout << "Progress source " << source << '\n';
        std::vector<vertex_descriptor> predecessors(num_vertices(g));
        // a list of costs stored for one node to all nodes in the graph
        std::vector<double> distances(num_vertices(g));
        std::deque<vertex_descriptor> nodesInDistance;

        // std::cout << "First value of predecessors is " << *(predecessors.begin()) << '\n';
        // std::cout << "First value of distance is " << *(distances.begin()) << '\n';
        // http://www.boost.org/doc/libs/1_61_0/libs/graph/doc/dijkstra_shortest_paths.html

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
            //std::cout << "Found goals" << '\n';
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
    long int calling_times = 0;
    // const static int BUFFER_LINE = 1024;
    static constexpr double DOUBLE_MIN = 1.e-6; // This is used for comparing double values
}; // NetworkGraph
}
#endif /* MM_NETWORK_GRAPH_HPP */
