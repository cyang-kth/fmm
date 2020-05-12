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

#include <unordered_map>
#include <unordered_set>
#include <osmium/handler.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/visitor.hpp>

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
  explicit WayFilter(bool do_filter = true);
  /**
   * Check if the given way should be included or not
   * @param way OSM way to be examined
   * @return true if the way will be included as part of the road network;
   * otherwise false is returned
   */
  bool check_include(const osmium::Way& way) const;
protected:
  /**
   * Tags to be excluded from the network
   * Key is the tag name and Value is the set of values to be excluded
   * from the network. If the way contains a tag whose value is in the
   * excluded set, then it will be filtered out.
   */
  std::unordered_map<
    std::string,std::unordered_set<std::string>> exclude_tags;
};

/**
 * Class for to read and process OpenStreetMap road network
 */
class OSMHandler : public osmium::handler::Handler {
public:
  /** Initialize a new instance based on the WayFilter provided */
  explicit OSMHandler(bool do_filter = false);
  /** Function to handle a way read from OSM data
   * (part of the osmium::handler::Handler interface) */
  void way(const osmium::Way& way);
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
  void generate_simplified_graph(Network& network) const;
protected:
  static void reverse_edge(LineString& edge);
  /** Function to determine if an OSM way's direction.
   *
   * \returns 0 for two-way, 1 for one-way following the nodes' order,
   *  -1 for reversed case.
   */
   /** Helper function to reverse an edge */
  static int is_oneway(const osmium::Way& way);
private:
  /** filter used to select which ways to include */
  const WayFilter wf;
};
}
}

#endif
