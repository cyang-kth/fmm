/**
 * Fast map matching.
 *
 * Network class
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef FMM_NETWORK_HPP
#define FMM_NETWORK_HPP

#include "network/type.hpp"
#include "core/gps.hpp"
#include "mm/mm_type.hpp"
#include <ogrsf_frmts.h> // C++ API for GDAL
#include <iostream>
#include <math.h> // Calulating probability
#include <iomanip>
#include <algorithm> // Partial sort copy
#include <unordered_set> // Partial sort copy

// Data structures for Rtree
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/function_output_iterator.hpp>

namespace FMM {
/**
 * Classes related with network and graph
 */
namespace NETWORK {
/**
 * Road network class
 */
class Network {
 public:
  /**
   * Box of a edge
   */
  typedef boost::geometry::model::box<FMM::CORE::Point> boost_box;
  /**
   * Item stored in a node of Rtree
   */
  typedef std::pair<boost_box, Edge *> Item;
  /**
   * Rtree of road edges
   */
  typedef boost::geometry::index::rtree<
      Item, boost::geometry::index::quadratic<16> > Rtree;
  /**
   *  Constructor of Network
   *
   *  @param filename: the path to a network file in ESRI shapefile format
   *  @param id_name: the name of the id field
   *  @param source_name: the name of the source field
   *  @param target_name: the name of the target field
   *
   */
  Network(const std::string &filename,
          const std::string &id_name = "id",
          const std::string &source_name = "source",
          const std::string &target_name = "target");   // Network constructor
  /**
   * Get number of nodes in the network
   * @return number of nodes
   */
  int get_node_count() const;
  /**
   * Get number of edges in the network
   * @return number of edges
   */
  int get_edge_count() const;
  /**
   * Get edges in the network
   * @return a constant reference to the edges
   */
  const std::vector<Edge> &get_edges() const;
  /**
   * Get edge ID from index
   * @param index index of edge
   * @return edge ID
   */
  EdgeID get_edge_id(EdgeIndex index) const;
  /**
   * Get edge index from ID
   * @param id edge id
   * @return edge index
   */
  EdgeIndex get_edge_index(EdgeID id) const;
  /**
   * Get node ID from index
   * @param index index of node
   * @return node ID
   */
  NodeID get_node_id(NodeIndex index) const;
  /**
   * Get node index from id
   * @param id node id
   * @return node index
   */
  NodeIndex get_node_index(NodeID id) const;
  /**
   * Get node geometry from index
   * @param index node index
   * @return point of a node
   */
  FMM::CORE::Point get_node_geom_from_idx(NodeIndex index) const;

  /**
   *  Search for k nearest neighboring (KNN) candidates of a
   *  trajectory within a search radius
   *
   *  @param trajectory: input trajectory
   *  @param k: the number of candidates
   *  @param radius: the search radius
   *  @return a 2D vector of Candidates containing
   *  the candidates selected for each point in a trajectory
   *
   */
  FMM::MM::Traj_Candidates search_tr_cs_knn(
      FMM::CORE::Trajectory &trajectory, std::size_t k, double radius) const;

  /**
   * Search for k nearest neighboring (KNN) candidates of a
   * linestring within a search radius
   *
   * @param geom
   * @param k number of candidates
   * @param radius search radius
   * @return a 2D vector of Candidates containing
   * the candidates selected for each point in a linestring
   */
  FMM::MM::Traj_Candidates search_tr_cs_knn(const FMM::CORE::LineString &geom,
                                            std::size_t k,
                                            double radius) const;
  /**
   * Get edge geometry
   * @param edge_id edge id
   * @return Geometry of edge
   */
  const FMM::CORE::LineString &get_edge_geom(EdgeID edge_id) const;
  /**
   * Extract the geometry of a complete path, whose two end segment will be
   * clipped according to the input trajectory
   * @param traj input trajectory
   * @param complete_path complete path
   */
  FMM::CORE::LineString complete_path_to_geometry(
      const FMM::CORE::LineString &traj,
      const MM::C_Path &complete_path) const;
  /**
   * Get all node geometry
   * @return a vector of points
   */
  const std::vector<FMM::CORE::Point> &get_vertex_points() const;
  /**
   * Get node geometry
   * @param u node index
   * @return geometry of node
   */
  const FMM::CORE::Point &get_vertex_point(NodeIndex u) const;
  /**
   * Extract the geometry of a route in the network
   * @param path a route stored with edge ID
   * @return the geometry of the route
   */
  FMM::CORE::LineString route2geometry(const std::vector<EdgeID> &path) const;
  /**
   * Extract the geometry of a route in the network
   * @param path a route stored with edge Index
   * @return the geometry of the route
   */
  FMM::CORE::LineString route2geometry(
      const std::vector<EdgeIndex> &path) const;
  /**
   * Compare two candidate according to their GPS error
   * @param a candidate a
   * @param b candidate b
   * @return true if a.dist<b.dist
   */
  static bool candidate_compare(const MM::Candidate &a, const MM::Candidate &b);
 private:
  /**
   * Concatenate a linestring segs to a linestring line, used in the
   * function complete_path_to_geometry
   *
   * @param line: linestring which will be updated
   * @param segs: segs that will be appended to line
   * @param offset: the number of points skipped in segs.
   */
  static void append_segs_to_line(FMM::CORE::LineString *line,
                                  const FMM::CORE::LineString &segs,
                                  int offset = 0);
  /**
   * Build rtree for the network
   */
  void build_rtree_index();
  int srid;   // Spatial reference id
  Rtree rtree;   // Network rtree structure
  std::vector<Edge> edges;   // all edges in the network
  NodeIDVec node_id_vec;
  unsigned int num_vertices;
  NodeIndexMap node_map;
  EdgeIndexMap edge_map;
  std::vector<FMM::CORE::Point> vertex_points;
}; // Network
} // NETWORK
} // FMM
#endif /* FMM_NETWORK_HPP */
