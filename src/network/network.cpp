#include "network/network.hpp"
#include "util/debug.hpp"
#include "util/util.hpp"
#include "algorithm/geom_algorithm.hpp"

#include <ogrsf_frmts.h> // C++ API for GDAL
#include <cmath> // Calulating probability
#include <algorithm> // Partial sort copy
#include <stdexcept>

// Data structures for Rtree
#include <boost/geometry/index/rtree.hpp>

#include <boost/format.hpp>

using namespace FMM;
using namespace FMM::CORE;
using namespace FMM::MM;
using namespace FMM::NETWORK;

bool Network::candidate_compare(const Candidate &a, const Candidate &b) {
  if (a.dist != b.dist) {
    return a.dist < b.dist;
  } else {
    return a.edge->index < b.edge->index;
  }
}

Network::Network(const std::string &filename,
                 const std::string &id_name,
                 const std::string &source_name,
                 const std::string &target_name) {
  if (FMM::UTIL::check_file_extension(filename, "shp")) {
    read_ogr_file(filename,id_name,source_name,target_name);
  } else {
    std::string message = (boost::format("Network file not supported %1%") % filename).str();
    SPDLOG_CRITICAL(message);
    throw std::runtime_error(message);
  }
};

void Network::add_edge(EdgeID edge_id, NodeID source, NodeID target,
                       const FMM::CORE::LineString &geom){
  NodeIndex s_idx, t_idx;
  if (node_map.find(source) == node_map.end()) {
    s_idx = node_id_vec.size();
    node_id_vec.push_back(source);
    node_map.insert({source, s_idx});
    vertex_points.push_back(geom.get_point(0));
  } else {
    s_idx = node_map[source];
  }
  if (node_map.find(target) == node_map.end()) {
    t_idx = node_id_vec.size();
    node_id_vec.push_back(target);
    node_map.insert({target, t_idx});
    int npoints = geom.get_num_points();
    vertex_points.push_back(geom.get_point(npoints - 1));
  } else {
    t_idx = node_map[target];
  }
  EdgeIndex index = edges.size();
  edges.push_back({index, edge_id, s_idx, t_idx, geom.get_length(), geom});
  edge_map.insert({edge_id, index});
};

void Network::read_ogr_file(const std::string &filename,
                            const std::string &id_name,
                            const std::string &source_name,
                            const std::string &target_name) {
  SPDLOG_INFO("Read network from file {}", filename);
  OGRRegisterAll();
  GDALDataset *poDS = (GDALDataset *) GDALOpenEx(
    filename.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
  if (poDS == NULL) {
    std::string message = "Open dataset failed.";
    SPDLOG_CRITICAL(message);
    throw std::runtime_error(message);
  }
  OGRLayer *ogrlayer = poDS->GetLayer(0);
  int NUM_FEATURES = ogrlayer->GetFeatureCount();
  // edges= std::vector<Edge>(NUM_FEATURES);
  // Initialize network edges
  OGRFeatureDefn *ogrFDefn = ogrlayer->GetLayerDefn();
  OGRFeature *ogrFeature;

  // Fetch the field index given field name.
  int id_idx = ogrFDefn->GetFieldIndex(id_name.c_str());
  int source_idx = ogrFDefn->GetFieldIndex(source_name.c_str());
  int target_idx = ogrFDefn->GetFieldIndex(target_name.c_str());
  if (source_idx < 0 || target_idx < 0 || id_idx < 0) {
    std::string error_message = fmt::format(
      "Field not found: {} index {}, {} index {}, {} index {}",
      id_name, id_idx, source_name, source_idx,
      target_name, target_idx);
    SPDLOG_CRITICAL(error_message);
    GDALClose(poDS);
    throw std::runtime_error(error_message);
  }

  if (wkbFlatten(ogrFDefn->GetGeomType()) != wkbLineString) {
    std::string error_message = fmt::format(
      "Geometry type of network is {}, should be linestring",
      OGRGeometryTypeToName(ogrFDefn->GetGeomType()));
    SPDLOG_CRITICAL(error_message);
    GDALClose(poDS);
    throw std::runtime_error(error_message);
  } else {
    SPDLOG_DEBUG("Geometry type of network is {}",
                 OGRGeometryTypeToName(ogrFDefn->GetGeomType()));
  }
  const OGRSpatialReference *ogrsr =
    ogrFDefn->GetGeomFieldDefn(0)->GetSpatialRef();
  if (ogrsr != nullptr) {
    srid = ogrsr->GetEPSGGeogCS();
    if (srid == -1) {
      srid = 4326;
      SPDLOG_WARN("SRID is not found, set to 4326 by default");
    } else {
      SPDLOG_DEBUG("SRID is {}", srid);
    }
  } else {
    srid = 4326;
    SPDLOG_WARN("SRID is not found, set to 4326 by default");
  }
  // Read data from shapefile
  EdgeIndex index = 0;
  while ((ogrFeature = ogrlayer->GetNextFeature()) != NULL) {
    EdgeID id = ogrFeature->GetFieldAsInteger64(id_idx);
    NodeID source = ogrFeature->GetFieldAsInteger64(source_idx);
    NodeID target = ogrFeature->GetFieldAsInteger64(target_idx);
    OGRGeometry *rawgeometry = ogrFeature->GetGeometryRef();
    LineString geom;
    if (rawgeometry->getGeometryType() == wkbLineString) {
      geom = ogr2linestring((OGRLineString *) rawgeometry);
    } else if (rawgeometry->getGeometryType() == wkbMultiLineString) {
      SPDLOG_TRACE("Feature id {} s {} t {} is multilinestring",
                   id, source, target);
      SPDLOG_TRACE("Read only the first linestring");
      geom = ogr2linestring((OGRMultiLineString *) rawgeometry);
    } else {
      SPDLOG_CRITICAL("Unknown geometry type for feature id {} s {} t {}",
                      id, source, target);
    }
    NodeIndex s_idx, t_idx;
    if (node_map.find(source) == node_map.end()) {
      s_idx = node_id_vec.size();
      node_id_vec.push_back(source);
      node_map.insert({source, s_idx});
      vertex_points.push_back(geom.get_point(0));
    } else {
      s_idx = node_map[source];
    }
    if (node_map.find(target) == node_map.end()) {
      t_idx = node_id_vec.size();
      node_id_vec.push_back(target);
      node_map.insert({target, t_idx});
      int npoints = geom.get_num_points();
      vertex_points.push_back(geom.get_point(npoints - 1));
    } else {
      t_idx = node_map[target];
    }
    edges.push_back({index, id, s_idx, t_idx, geom.get_length(), geom});
    edge_map.insert({id, index});
    ++index;
    OGRFeature::DestroyFeature(ogrFeature);
  }
  GDALClose(poDS);
  num_vertices = node_id_vec.size();
  SPDLOG_INFO("Number of edges {} nodes {}", edges.size(), num_vertices);
  SPDLOG_INFO("Field index: id {} source {} target {}",
              id_idx, source_idx, target_idx);
  build_rtree_index();
  SPDLOG_INFO("Read network done.");
}    // Network constructor

int Network::get_node_count() const {
  return node_id_vec.size();
}

int Network::get_edge_count() const {
  return edges.size();
}

// Get the edge vector
const std::vector<Edge> &Network::get_edges() const {
  return edges;
}

const Edge& Network::get_edge(EdgeID id) const {
  return edges[get_edge_index(id)];
};

const Edge& Network::get_edge(EdgeIndex index) const {
  return edges[index];
};

// Get the ID attribute of an edge according to its index
EdgeID Network::get_edge_id(EdgeIndex index) const {
  return index < edges.size() ? edges[index].id : -1;
}

EdgeIndex Network::get_edge_index(EdgeID id) const {
  return edge_map.at(id);
}

NodeID Network::get_node_id(NodeIndex index) const {
  return index < num_vertices ? node_id_vec[index] : -1;
}

NodeIndex Network::get_node_index(NodeID id) const {
  return node_map.at(id);
}

Point Network::get_node_geom_from_idx(NodeIndex index) const {
  return vertex_points[index];
}

// Construct a Rtree using the vector of edges
void Network::build_rtree_index() {
  // Build an rtree for candidate search
  SPDLOG_DEBUG("Create boost rtree");
  // create some Items
  for (std::size_t i = 0; i < edges.size(); ++i) {
    // create a boost_box
    Edge *edge = &edges[i];
    double x1, y1, x2, y2;
    ALGORITHM::boundingbox_geometry(edge->geom, &x1, &y1, &x2, &y2);
    boost_box b(Point(x1, y1), Point(x2, y2));
    rtree.insert(std::make_pair(b, edge));
  }
  SPDLOG_DEBUG("Create boost rtree done");
}

Traj_Candidates Network::search_tr_cs_knn(Trajectory &trajectory, std::size_t k,
                                          double radius) const {
  return search_tr_cs_knn(trajectory.geom, k, radius);
}

namespace {
/**
 * Euclidean distance from the point (px, py) to an axis aligned box, 0 when
 * the point lies inside the box. The box of an edge contains its geometry,
 * so this is a lower bound of the distance from the point to the edge.
 */
inline double point_box_distance(double px, double py,
                                 const Network::boost_box &box) {
  namespace bg = boost::geometry;
  double dx = 0, dy = 0;
  double min_x = bg::get<bg::min_corner, 0>(box);
  double max_x = bg::get<bg::max_corner, 0>(box);
  double min_y = bg::get<bg::min_corner, 1>(box);
  double max_y = bg::get<bg::max_corner, 1>(box);
  if (px < min_x) {
    dx = min_x - px;
  } else if (px > max_x) {
    dx = px - max_x;
  }
  if (py < min_y) {
    dy = min_y - py;
  } else if (py > max_y) {
    dy = py - max_y;
  }
  return std::sqrt(dx * dx + dy * dy);
}
} // namespace

Traj_Candidates Network::search_tr_cs_knn(const LineString &geom, std::size_t k,
                                          double radius) const {
  int NumberPoints = geom.get_num_points();
  Traj_Candidates tr_cs(NumberPoints);
  unsigned int current_candidate_index = num_vertices;
  // Buffers reused for every point of the trajectory
  std::vector<Item> temp;
  Point_Candidates pcs;
  // The rtree is queried with a box that is usually much smaller than the
  // search radius and enlarged until the k best candidates are provably
  // found (see the end of the loop below). The initial box size adapts to
  // the density of edges around the previous point.
  const double min_query_radius = radius / 1024;
  double query_radius = radius / 16;
  for (int i = 0; i < NumberPoints; ++i) {
    // SPDLOG_DEBUG("Search candidates for point index {}",i);
    double px = geom.get_x(i);
    double py = geom.get_y(i);
    // Tolerance added to the lower bound tests below. It is far above the
    // rounding error of the distance computations (which scales with the
    // magnitude of the coordinates) and far below any meaningful distance,
    // so the pruning can never drop a candidate the exact test would keep.
    const double tol = 1e-9 * std::max({std::fabs(px), std::fabs(py), 1.0});
    while (true) {
      // Construct a bounding boost_box
      boost_box b(Point(px - query_radius, py - query_radius),
                  Point(px + query_radius, py + query_radius));
      temp.clear();
      // Rtree can only detect intersect with a the bounding box of
      // the geometry stored.
      rtree.query(boost::geometry::index::intersects(b),
                  std::back_inserter(temp));
      // Keep the k best candidates in pcs, sorted by candidate_compare, so
      // that pcs.back() is the current k-th best. candidate_compare is a
      // total order, hence this yields exactly the same candidates in the
      // same order as computing every candidate within radius and partially
      // sorting them, but the exact distance is only computed for the edges
      // whose bounding box is closer than the current k-th best candidate.
      pcs.clear();
      for (const Item &item : temp) {
        double lower_bound = point_box_distance(px, py, item.first);
        double threshold = pcs.size() < k ? radius : pcs.back().dist;
        if (lower_bound > threshold + tol) continue;
        // Check for detailed intersection
        Edge *edge = item.second;
        double offset;
        double dist;
        double closest_x, closest_y;
        ALGORITHM::linear_referencing(px, py, edge->geom,
                                      &dist, &offset, &closest_x, &closest_y);
        if (dist > radius) continue;
        // index, offset, dist, edge, pseudo id, point
        Candidate c = {0,
                       offset,
                       dist,
                       edge,
                       Point(closest_x, closest_y)};
        if (pcs.size() >= k) {
          if (!candidate_compare(c, pcs.back())) continue;
          pcs.pop_back();
        }
        pcs.insert(std::upper_bound(pcs.begin(), pcs.end(), c,
                                    candidate_compare), c);
      }
      if (query_radius >= radius) break;
      // Every edge outside the query box is farther than query_radius from
      // the point. Once k candidates are found and the k-th one is closer
      // than that, no edge outside the box can be among the k best and the
      // result is the same as for a query with the full search radius.
      if (pcs.size() >= k && pcs.back().dist + tol < query_radius) break;
      query_radius = std::min(radius, query_radius * 4);
    }
    // Start the search for the next point with a box of twice the distance
    // of the k-th best candidate of this point.
    if (pcs.size() >= k) {
      query_radius = std::min(radius, std::max(min_query_radius,
                                               2 * pcs.back().dist));
    } else {
      query_radius = radius;
    }
    SPDLOG_DEBUG("Candidate count point {}: {}", i, pcs.size());
    if (pcs.empty()) {
      SPDLOG_DEBUG("Candidate not found for point {}: {} {}",i,px,py);
      return Traj_Candidates();
    }
    tr_cs[i] = pcs;
    for (std::size_t m = 0; m < tr_cs[i].size(); ++m) {
      tr_cs[i][m].index = current_candidate_index + m;
    }
    current_candidate_index += tr_cs[i].size();
    // SPDLOG_TRACE("current_candidate_index {}",current_candidate_index);
  }
  return tr_cs;
}

const LineString &Network::get_edge_geom(EdgeID edge_id) const {
  return edges[get_edge_index(edge_id)].geom;
}

LineString Network::complete_path_to_geometry(
  const LineString &traj, const C_Path &complete_path) const {
  // if (complete_path->empty()) return nullptr;
  LineString line;
  if (complete_path.empty()) return line;
  int Npts = traj.get_num_points();
  int NCsegs = complete_path.size();
  if (NCsegs == 1) {
    double dist;
    double firstoffset;
    double lastoffset;
    const LineString &firstseg = get_edge_geom(complete_path[0]);
    ALGORITHM::linear_referencing(traj.get_x(0), traj.get_y(0), firstseg,
                                  &dist, &firstoffset);
    ALGORITHM::linear_referencing(traj.get_x(Npts - 1), traj.get_y(Npts - 1),
                                  firstseg, &dist, &lastoffset);
    LineString firstlineseg = ALGORITHM::cutoffseg_unique(firstseg, firstoffset,
                                                          lastoffset);
    append_segs_to_line(&line, firstlineseg, 0);
  } else {
    const LineString &firstseg = get_edge_geom(complete_path[0]);
    const LineString &lastseg = get_edge_geom(complete_path[NCsegs - 1]);
    double dist;
    double firstoffset;
    double lastoffset;
    ALGORITHM::linear_referencing(traj.get_x(0), traj.get_y(0), firstseg,
                                  &dist, &firstoffset);
    ALGORITHM::linear_referencing(traj.get_x(Npts - 1), traj.get_y(Npts - 1),
                                  lastseg, &dist, &lastoffset);
    LineString firstlineseg = ALGORITHM::cutoffseg(firstseg, firstoffset, 0);
    LineString lastlineseg = ALGORITHM::cutoffseg(lastseg, lastoffset, 1);
    append_segs_to_line(&line, firstlineseg, 0);
    if (NCsegs > 2) {
      for (int i = 1; i < NCsegs - 1; ++i) {
        const LineString &middleseg = get_edge_geom(complete_path[i]);
        append_segs_to_line(&line, middleseg, 1);
      }
    }
    append_segs_to_line(&line, lastlineseg, 1);
  }
  return line;
}

const std::vector<Point> &Network::get_vertex_points() const {
  return vertex_points;
}

const Point &Network::get_vertex_point(NodeIndex u) const {
  return vertex_points[u];
}

LineString Network::route2geometry(const std::vector<EdgeID> &path) const {
  LineString line;
  if (path.empty()) return line;
  // if (complete_path->empty()) return nullptr;
  int NCsegs = path.size();
  for (int i = 0; i < NCsegs; ++i) {
    EdgeIndex e = get_edge_index(path[i]);
    const LineString &seg = edges[e].geom;
    if (i == 0) {
      append_segs_to_line(&line, seg, 0);
    } else {
      append_segs_to_line(&line, seg, 1);
    }
  }
  //SPDLOG_DEBUG("Path geometry is {}",line.exportToWkt());
  return line;
}

LineString Network::route2geometry(const std::vector<EdgeIndex> &path) const {
  LineString line;
  if (path.empty()) return line;
  // if (complete_path->empty()) return nullptr;
  int NCsegs = path.size();
  for (int i = 0; i < NCsegs; ++i) {
    const LineString &seg = edges[path[i]].geom;
    if (i == 0) {
      append_segs_to_line(&line, seg, 0);
    } else {
      append_segs_to_line(&line, seg, 1);
    }
  }
  //SPDLOG_DEBUG("Path geometry is {}",line.exportToWkt());
  return line;
}

void Network::append_segs_to_line(LineString *line,
                                  const LineString &segs, int offset) {
  int Npoints = segs.get_num_points();
  for (int i = 0; i < Npoints; ++i) {
    if (i >= offset) {
      line->add_point(segs.get_x(i), segs.get_y(i));
    }
  }
}
