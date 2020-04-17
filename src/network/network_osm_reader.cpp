// network_osm_reader.cpp -- function to read OSM data as network

#include "network/network.hpp"
#include "util/debug.hpp"

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdio.h>

#include <osmium/handler.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/visitor.hpp>


using namespace FMM;
using namespace FMM::CORE;
using namespace FMM::MM;
using namespace FMM::NETWORK;



/** Helper class to filter out OSM ways that are not roads */
class way_filter {
protected:
  /** key -- value pairs; if any of the keys is present with any of the corresponding values, this item is skipped */
  std::unordered_map<std::string, std::unordered_set<std::string> > exclude_tags;
  /** key -- value pairs; all of the keys here need to be present with any of the values given -- if no value given, the key
   * has to be present with any value */
  std::unordered_map<std::string, std::unordered_set<std::string> > include_tags;

public:
  /** test if the given way matches the filters */
  bool operator () (const osmium::Way& way) const {
		/* test include_tags first -- note that this is an O(n*m) operation where n is the size of include_tags, and 
		 * m is the number of tags the way has; since the size of include_tags will be typically small (1),
		 * this will not be a practical problem*/
		for(const auto& p : include_tags) {
			const char* value = way.get_value_by_key(p.first.c_str());
			if(!value) return false;
			if(p.second.size() && p.second.count(std::string(value)) == 0) return false;
		}
		/* test exclude_tags next -- iterate over all tags for the way for this */
		for(const osmium::Tag& tag : way.tags()) {
			const auto& it = exclude_tags.find(std::string(tag.key()));
			if(it != exclude_tags.end()) {
				if(it->second.count(std::string(tag.value())) > 0) return false;
			}
		}
		return true;
	}
  
  /** create default filter to include roads */
  way_filter() {
    /* must be highway */
    include_tags.insert(std::make_pair(std::string("highway"),std::unordered_set<std::string>()));
    /* exclude ways unsuitable for cars */
    exclude_tags.insert(std::make_pair(std::string("highway"),std::unordered_set<std::string>{"steps","corridor",
      "pedestrian","proposed","construction","abandoned","platform","raceway","track","bus_guideway","escape",
      "footway","bridleway","path","cycleway","elevator","platform"}));
    
    /* exclude unsuitable surface */
    exclude_tags.insert(std::make_pair(std::string("surface"),std::unordered_set<std::string>{"wood","unpaved","bricks",
      "grass","sand","dirt","gravel","ground","dirt/sand","grass_paver","mud","rocky","sett","pebblestone","pebblestones"}));
    /* exclude any way that marks an area (not typical for highways anyway */
    exclude_tags.insert(std::make_pair(std::string("area"),std::unordered_set<std::string>{"yes"}));
    /* exclude private roads */
    exclude_tags.insert(std::make_pair(std::string("service"),std::unordered_set<std::string>{"private"}));
    exclude_tags.insert(std::make_pair(std::string("access"),std::unordered_set<std::string>{"private","no"}));
    
  }
};



/** 
 * Class for to read and process OpenStreetMap road network
 */
class OSMHandler : public osmium::handler::Handler {
protected:
  /** Store the coordinates of all nodes internally */
  std::unordered_map<osmium::unsigned_object_id_type,
    osmium::Location> node_coords;
  
  /** Store the network in a graph format for processing.
   * For each edge, it stores the direction as a flag
   * (1: forward, 0: both ways, -1: backward).
   * Each edge is stored in both directions */
  std::unordered_map<osmium::unsigned_object_id_type,
    std::unordered_map<osmium::unsigned_object_id_type,
      int > > node_graph;
  
  /** filter used to select which ways to include */
  const way_filter wf;
  
  /** Function to determine if an OSM way's direction. Returns 0 for two-way, 1 for one-way following the nodes' order, -1 for reversed case. */
  static int is_oneway(const osmium::Way& way)  {
		int oneway = 0;
		const char* value = way.get_value_by_key("oneway");
		if(value) {
			if(!strcmp(value,"yes") || !strcmp(value,"1") || !strcmp(value,"true")) oneway = 1;
			if(!strcmp(value,"-1")) oneway = -1;
		}
		if(!oneway) {
			value = way.get_value_by_key("highway");
			if(value && !strcmp(value,"motorway")) oneway = 1;
			else {
				value = way.get_value_by_key("junction");
				if(value && !strcmp(value,"roundabout")) oneway = 1;
			}
		}
		return oneway;
	}
  
public:
  /** Initialize a new instance based on the way_filter provided */
  OSMHandler() { }
  
  /** Function to handle a node read from OSM data (part of the osmium::handler::Handler interface) */
  void node(const osmium::Node& node) {
    node_coords[node.id()] = node.location();
  }
  
  /** Function to handle a way read from OSM data (part of the osmium::handler::Handler interface) */
  void way(const osmium::Way& way) {
    /* filter out ways that are not roads */
    if(!wf(way)) return;
    /* check that the way has more than 1 node */
    if(way.nodes().size() < 2) return;
    
    /* check the direction of this way */
    int dir = is_oneway(way);
    
    /* process all nodes in this way */
    osmium::unsigned_object_id_type last_node_id = 18446744073709551615UL;
    for(auto& nr : way.nodes()) {
      auto node_id = nr.ref();
      if(last_node_id != 18446744073709551615UL) {
        /* add all links to the graph */
        node_graph[last_node_id][node_id] = dir;
        node_graph[node_id][last_node_id] = -1*dir;
      }
      last_node_id = node_id;
    }
  }
  
  /** \brief Function that generates a simplified graph.
   * 
   * One edge is defined by a path between any two OSM nodes 
   * with degree != 2. Typically, an edge contains multiple OSM 
   * nodes which have the purpose of determining the correct 
   * geometry of the edge. Also, there is not a one-to-one 
   * correspondence between edges and OSM ways: an edge can 
   * contain multiple OSM ways (e.g. if some property changes on 
   * the way), and an OSM way can belong to multiple edge (an OSM 
   * way can be created for a road that spans multiple 
   * intersections).
   * 
   * This function identifies the edges and adds them to an
   * FMM::NETWORK::Network class.
   */
  void generate_simplified_graph(Network& network) const {
    std::unordered_set<osmium::unsigned_object_id_type> nodes_seen;
    std::unordered_set<osmium::unsigned_object_id_type> extra_start;
    /* Note: we cannot use OSM IDs, since a way can be part
     * of multiple edges.
     * TODO: find a way to store these edge IDs for later!! */
    EdgeID edge_id = 0; 
    //~ std::unordered_set<osmium::unsigned_object_id_type> ways_seen;
    
    /* two passes of the main loop, 2nd pass needed only for
     * cases when a oneway road becomes bidirectional at a node
     * with degree == 2 */
    for(int i=0;i<2;i++) {
      auto loop_it1 = node_graph.begin();
      while(true) {
        std::unordered_map<osmium::unsigned_object_id_type,
          std::unordered_map<osmium::unsigned_object_id_type,
          int> >::const_iterator node_it;
        
        if(i == 0) {
          if(loop_it1 == node_graph.end()) break;
          node_it = loop_it1;
          ++loop_it1;
          if( !(node_it->second.size() == 1 || node_it->second.size() > 2) ) continue;
        }
        else {
          auto loop_it2 = extra_start.begin();
          if(loop_it2 == extra_start.end()) break;
          node_it = node_graph.find(*loop_it2);
          if(node_it == node_graph.end())
            throw std::runtime_error("OSMHandler::generate_simplified_graph(): node not found!\n");
          extra_start.erase(loop_it2);
        }
        
        const auto& n = *node_it;
        osmium::unsigned_object_id_type n1 = n.first;
        /* optional to filter for connected components here */
        //~ if(sccs.size()) if(sccs.at(n1) != sccid) continue;
        
        /* check all possible paths going out from this node */
        for(const auto& ps : n.second) {
          /* edge properties */
          osmium::unsigned_object_id_type first_node = n1;
          osmium::unsigned_object_id_type last_node = n1; /* from node */
          osmium::unsigned_object_id_type end_node = ps.first; /* to node */
          int dir = ps.second; /* edge directions */
          
          /* if the first node on the path has been seen
           * already, there is no need to look further */
          if(nodes_seen.count(end_node)) continue;
          
          /* potential filter for connected components */
          //~ if(sccs.size()) if(sccs.at(end_node) != sccid) continue;
          
          /* start of a new edge */
          LineString edge;
          /* add first two nodes */
          {
            const auto& loc1 = node_coords.at(first_node);
            const auto& loc2 = node_coords.at(end_node);
            edge.add_point(Point(loc1.lon(),loc1.lat()));
            edge.add_point(Point(loc2.lon(),loc2.lat()));
          }
          
          while(true) {
            const auto& nn = node_graph.at(end_node);
            /* break if current node's degree != 2 */
            if(nn.size() != 2) break; 
            
            /* find the next node -- note that this loop
             * will have maximum two iterations, since
             * the node has degree 2 if we are here */
            int dir2;
            for(const auto& x : nn) {
              if(x.first != last_node) {
                last_node = end_node;
                end_node = x.first;
                dir2 = x.second;
                break;
              }
            }
            
            /* check that direction is still consistent */
            if(dir2 != dir) {
              if( !(dir2 == 0 || dir == 0) ) {
                /* this is error, "source" or "sink" node detected */
                SPDLOG_WARN("OSMHandler::generate_simplified_graph(): source or sink node detected: {}!\n",last_node);
              }
                
              /* treat this as two segments */
              end_node = last_node;
              extra_start.insert(end_node);
              break;
            
              /*
              fprintf(stderr,"OSMHandler::generate_simplified_graph(): edges change direction in path chain!\n");
              fprintf(stderr,"nodes: %lu -- %lu\n",last_node,end_node);
              throw std::runtime_error("OSMMap::save_csv(): edges change direction in path chain!\n"); */
            }
            
            /* add new node to path */
            {
              const auto& loc = node_coords.at(end_node);
              edge.add_point(Point(loc.lon(),loc.lat()));
            }
            /* add middle nodes as seen */
            nodes_seen.insert(last_node);
          }
          /* end of edge, we haveboost::geometry::model::linestring added all nodes,
           * add it to the network */
          
          /* 1. forward edge */
          if(dir >= 0) {
            network.add_edge(edge_id,first_node,end_node,edge);
            edge_id++;
          }
          /* 2. reverse edge */
          if(dir <= 0) {
            reverse_edge(edge);
            network.add_edge(edge_id,end_node,first_node,edge);
            edge_id++;
          }
        }
        nodes_seen.insert(n1);
      }
    }
  }
  
  
  /** Helper function to reverse an edge */
  static void reverse_edge(LineString& edge) {
    int num_points = edge.get_num_points();
    for(int i = 0, j = num_points - 1; i < j; i++, j--) {
      Point pi = edge.get_point(i);
      Point pj = edge.get_point(j);
      edge.set_x(i, pj.get<0>());
      edge.set_y(i, pj.get<1>());
      edge.set_x(j, pi.get<0>());
      edge.set_y(j, pi.get<1>());
    }
  }
};



void Network::read_osm_file(const std::string &filename) {
  SPDLOG_INFO("Read osm network {} ", filename);
  auto otypes = osmium::osm_entity_bits::node|osmium::osm_entity_bits::way;
  osmium::io::Reader reader{filename, otypes};
  OSMHandler handler;
  osmium::apply(reader, handler);
  reader.close();
  handler.generate_simplified_graph(*this);
  build_rtree_index();
  SPDLOG_INFO("Read osm network done with edges read {}",edges.size());
};



