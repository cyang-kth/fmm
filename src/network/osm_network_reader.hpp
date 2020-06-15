/**
 * Fast map matching.
 *
 * OpenStreetMap network reader classes
 *
 * The file contains classes related with reading data from
 * OpenStreetMap format based on the library of libosmium.
 *
 * @author: Daniel Kondor
 *          Can Yang (updated and documentation added)
 * @version: 2018.03.09
 */

#ifndef FMM_NETWORK_OSM_READER_HPP
#define FMM_NETWORK_OSM_READER_HPP

#ifndef SKIP_OSM_BUILD

#include "network/network.hpp"
#include "util/debug.hpp"
#include <unordered_map>
#include <unordered_set>
#include <osmium/handler.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/visitor.hpp>
#include <osmium/geom/ogr.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>

namespace FMM {
namespace NETWORK {
/**
 * Helper class to filter out OSM ways that are not roads
 */
class WayFilter {
public:
  /** create default filter to include roads */
  /**
   * Constructor of way filter
   * @param  do_filter perform filter or not
   */
  explicit WayFilter(){
    exclude_tags.insert(
      std::make_pair(std::string("area"),
                     std::unordered_set<std::string>{"yes"}));
    exclude_tags.insert(
      std::make_pair(
        std::string("highway"),std::unordered_set<std::string>
        {
          "cycleway","footway","path",
          "pedestrian","steps","track","corridor","elevator","escalator",
          "proposed","construction","bridleway","abandoned","platform",
          "raceway"}));
    exclude_tags.insert(
      std::make_pair(std::string("motor_vehicle"),
                     std::unordered_set<std::string>{"no"}));
    exclude_tags.insert(
      std::make_pair(std::string("motorcar"),
                     std::unordered_set<std::string>{"no"}));
    exclude_tags.insert(
      std::make_pair(std::string("service"),
                     std::unordered_set<std::string>{
          "parking","parking_aisle","private","emergency_access"}));
    exclude_tags.insert(
      std::make_pair(std::string("access"),
                     std::unordered_set<std::string>{"private"}));
  };
  /**
   * Check if the given way should be excluded or not
   * @param way OSM way to be examined
   * @return true if the way the road network;
   * otherwise false is returned
   */
  inline bool is_excluded(const osmium::Way& way) const {
    const char* value = way.get_value_by_key("highway");
    if(!value) return true;
    /* test exclude_tags next -- iterate over all tags for the way for this */
    for(const osmium::Tag& tag : way.tags()) {
      const auto& it = exclude_tags.find(std::string(tag.key()));
      if(it != exclude_tags.end()) {
        if(it->second.count(std::string(tag.value())) > 0) return true;
      }
    }
    return false;
  };
protected:
  /**
   * Tags to be excluded from the network
   * Key is the tag name and Value is the set of values to be excluded
   * from the network. If the way contains a tag whose value is in the
   * excluded set, then it will be filtered out.
   */
  std::unordered_map<
    std::string,std::unordered_set<std::string> > exclude_tags;
}; // WayFilter

class OSMHandler : public osmium::handler::Handler {
public:
  OSMHandler(Network *network_arg) : network(*network_arg){
  };
  inline void way(const osmium::Way& way){
    ++way_processed;
    if (way_processed%100000==0){
      SPDLOG_INFO("Processed ways in osm file {}",way_processed);
    }
    if (filter.is_excluded(way)) return;
    ++way_kept;
    if (way.nodes().size()>1) {
      try{
        EdgeID eid= way.id();
        int source = way.nodes().front().ref();
        int target = way.nodes().back().ref();
        // SPDLOG_INFO("Read road edge {} {} {} nodes {}",
        //              eid, source, target, way.nodes().size());
        std::unique_ptr<OGRLineString> line = factory.create_linestring(way);

        CORE::LineString geom = CORE::ogr2linestring(line.get());
        int one_way = is_oneway(way);
        if (one_way==0) {
          // Two way
          network.add_edge(eid,source,target,geom);
          network.add_edge(-eid,target,source,
                           ALGORITHM::reverse_geometry(geom));
        } else if (one_way==1) {
          // One way the same direction
          network.add_edge(eid,source,target,geom);
        } else {
          // One way the reverse direction
          network.add_edge(-eid,target,source,
                           ALGORITHM::reverse_geometry(geom));
        }
      } catch (const std::exception& e) { // caught by reference to base
        // std::cout << " a standard exception was caught, with message '"
        //           << e.what() << "'\n";
      }
    }
  };
  /** Function to determine if an OSM way's direction.
   *
   * \returns 0 for two-way, 1 for one-way following the nodes' order,
   *  -1 for reversed case.
   */
  static inline int is_oneway(const osmium::Way& way)  {
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
  };
  inline unsigned int get_way_processed() const {
    return way_processed;
  }
  inline unsigned int get_way_kept() const {
    return way_kept;
  }
private:
  Network &network;
  WayFilter filter;
  unsigned int way_processed = 0;
  unsigned int way_kept = 0;
  osmium::geom::OGRFactory<> factory;
};

class OSMNetworkReader {
public:
  static inline void read_osm_data_into_network(const std::string &filename,
  Network *network){
    auto otypes = osmium::osm_entity_bits::node|osmium::osm_entity_bits::way;
    osmium::io::Reader reader{filename, otypes};
    using index_type =
      osmium::index::map::SparseMemArray
      <osmium::unsigned_object_id_type, osmium::Location>;
    using location_handler_type
      = osmium::handler::NodeLocationsForWays<index_type>;
    index_type index;
    location_handler_type location_handler{index};
    location_handler.ignore_errors();
    OSMHandler handler(network);
    osmium::apply(reader, location_handler, handler);
    reader.close();
    SPDLOG_INFO("Ways kept {} among {}",handler.get_way_kept(),
      handler.get_way_processed());
  };
};

}; // NETWORK
}; // FMM

#endif // SKIP_OSM_BUILD

#endif // FMM_NETWORK_OSM_READER_HPP
