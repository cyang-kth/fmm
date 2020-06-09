#include "network/network.hpp"
#include "util/debug.hpp"
#include "util/util.hpp"
#include "algorithm/geom_algorithm.hpp"

#include <ogrsf_frmts.h> // C++ API for GDAL
#include <math.h> // Calulating probability
#include <algorithm> // Partial sort copy
 // Partial sort copy

// Data structures for Rtree
#include <boost/geometry/index/rtree.hpp>
#include <boost/function_output_iterator.hpp>

using namespace FMM;
using namespace FMM::CORE;
using namespace FMM::MM;
using namespace FMM::NETWORK;

bool Network::candidate_compare(const Candidate &a, const Candidate &b)
{
  if (a.dist!=b.dist)
  {
    return a.dist<b.dist;
  }
  else
  {
    return a.edge->index<b.edge->index;
  }
}

Network::Network(const std::string &filename,
                 const std::string &id_name,
                 const std::string &source_name,
                 const std::string &target_name)
{
  SPDLOG_INFO("Read network from file {}",filename);
  OGRRegisterAll();
  GDALDataset *poDS = (GDALDataset*) GDALOpenEx(
    filename.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL ) {
    SPDLOG_CRITICAL("Open dataset failed.");
    exit( 1 );
  }
  OGRLayer  *ogrlayer = poDS->GetLayer(0);
  int NUM_FEATURES = ogrlayer->GetFeatureCount();
  // edges= std::vector<Edge>(NUM_FEATURES);
  // Initialize network edges
  OGRFeatureDefn *ogrFDefn = ogrlayer->GetLayerDefn();
  OGRFeature *ogrFeature;

  // Fetch the field index given field name.
  int id_idx=ogrFDefn->GetFieldIndex(id_name.c_str());
  int source_idx=ogrFDefn->GetFieldIndex(source_name.c_str());
  int target_idx=ogrFDefn->GetFieldIndex(target_name.c_str());
  if (source_idx<0||target_idx<0||id_idx<0) {
    SPDLOG_CRITICAL("Id, source or target column not found");
    GDALClose( poDS );
    std::exit(EXIT_FAILURE);
  }

  if (wkbFlatten(ogrFDefn->GetGeomType()) != wkbLineString)
  {
    SPDLOG_CRITICAL("Geometry type of network is {}, should be linestring",
                    OGRGeometryTypeToName(ogrFDefn->GetGeomType()));
    GDALClose( poDS );
    std::exit(EXIT_FAILURE);
  } else {
    SPDLOG_DEBUG("Geometry type of network is {}",
                OGRGeometryTypeToName(ogrFDefn->GetGeomType()));
  }
  OGRSpatialReference *ogrsr = ogrFDefn->GetGeomFieldDefn(0)->GetSpatialRef();
  if (ogrsr != nullptr) {
    srid = ogrsr->GetEPSGGeogCS();
    if (srid==-1)
    {
      srid= 4326;
      SPDLOG_WARN("SRID is not found, set to 4326 by default");
    } else {
      SPDLOG_DEBUG("SRID is {}",srid);
    }
  } else {
    srid= 4326;
    SPDLOG_WARN("SRID is not found, set to 4326 by default");
  }
  // Read data from shapefile
  EdgeIndex index = 0;
  while( (ogrFeature = ogrlayer->GetNextFeature()) != NULL){
    EdgeID id = ogrFeature->GetFieldAsInteger(id_idx);
    NodeID source = ogrFeature->GetFieldAsInteger(source_idx);
    NodeID target = ogrFeature->GetFieldAsInteger(target_idx);
    OGRGeometry *rawgeometry = ogrFeature->GetGeometryRef();
    LineString geom;
    if (rawgeometry->getGeometryType()==wkbLineString) {
      geom = ogr2linestring((OGRLineString*) rawgeometry);
    } else if (rawgeometry->getGeometryType()==wkbMultiLineString) {
      SPDLOG_TRACE("Feature id {} s {} t {} is multilinestring",
                   id, source, target);
      SPDLOG_TRACE("Read only the first linestring");
      geom = ogr2linestring((OGRMultiLineString*) rawgeometry);
    } else {
      SPDLOG_CRITICAL("Unknown geometry type for feature id {} s {} t {}",
                      id, source, target);
    }
    NodeIndex s_idx,t_idx;
    if (node_map.find(source)==node_map.end()) {
      s_idx = node_id_vec.size();
      node_id_vec.push_back(source);
      node_map.insert({source,s_idx});
      vertex_points.push_back(geom.get_point(0));
    } else {
      s_idx = node_map[source];
    }
    if (node_map.find(target)==node_map.end()) {
      t_idx = node_id_vec.size();
      node_id_vec.push_back(target);
      node_map.insert({target,t_idx});
      int npoints = geom.get_num_points();
      vertex_points.push_back(geom.get_point(npoints-1));
    } else {
      t_idx = node_map[target];
    }
    edges.push_back({index,id,s_idx,t_idx,geom.get_length(),geom});
    edge_map.insert({id,index});
    ++index;
    OGRFeature::DestroyFeature(ogrFeature);
  }
  GDALClose( poDS );
  num_vertices = node_id_vec.size();
  SPDLOG_INFO("Number of edges {} nodes {}",edges.size(),num_vertices);
  SPDLOG_INFO("Field index: id {} source {} target {}",
      id_idx,source_idx,target_idx);
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
const std::vector<Edge> &Network::get_edges() const
{
  return edges;
}

const Edge& Network::get_edge(EdgeID id) const {
  return edges[get_edge_index(id)];
};

const Edge& Network::get_edge(EdgeIndex index) const{
  return edges[index];
};

// Get the ID attribute of an edge according to its index
EdgeID Network::get_edge_id(EdgeIndex index) const
{
  return index<edges.size() ? edges[index].id:-1;
}

EdgeIndex Network::get_edge_index(EdgeID id) const {
  return edge_map.at(id);
}

NodeID Network::get_node_id(NodeIndex index) const {
  return index<num_vertices ? node_id_vec[index] : -1;
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
    double x1,y1,x2,y2;
    ALGORITHM::boundingbox_geometry(edge->geom,&x1,&y1,&x2,&y2);
    boost_box b(Point(x1,y1), Point(x2,y2));
    rtree.insert(std::make_pair(b,edge));
  }
  SPDLOG_DEBUG("Create boost rtree done");
}

Traj_Candidates Network::search_tr_cs_knn(Trajectory &trajectory, std::size_t k,
                                          double radius) const {
  return search_tr_cs_knn(trajectory.geom,k,radius);
}

Traj_Candidates Network::search_tr_cs_knn(const LineString &geom, std::size_t k,
                                          double radius) const
{
  int NumberPoints = geom.get_num_points();
  Traj_Candidates tr_cs(NumberPoints);
  unsigned int current_candidate_index = num_vertices;
  for (int i=0; i<NumberPoints; ++i) {
    // SPDLOG_DEBUG("Search candidates for point index {}",i);
    // Construct a bounding boost_box
    double px = geom.get_x(i);
    double py = geom.get_y(i);
    Point_Candidates pcs;
    boost_box b(Point(geom.get_x(i)-radius,geom.get_y(i)-radius),
                Point(geom.get_x(i)+radius,geom.get_y(i)+radius));
    std::vector<Item> temp;
    // Rtree can only detect intersect with a the bounding box of
    // the geometry stored.
    rtree.query(boost::geometry::index::intersects(b),
                std::back_inserter(temp));
    int Nitems = temp.size();
    for (unsigned int j=0; j<Nitems; ++j) {
      // Check for detailed intersection
      // The two edges are all in OGR_linestring
      Edge *edge = temp[j].second;
      double offset;
      double dist;
      double closest_x,closest_y;
      ALGORITHM::linear_referencing(px,py,edge->geom,
                                    &dist,&offset,&closest_x,&closest_y);
      if (dist<=radius) {
        // index, offset, dist, edge, pseudo id, point
        Candidate c = {0,
                       offset,
                       dist,
                       edge,
                       Point(closest_x,closest_y)};
        pcs.push_back(c);
      }
    }
    if (pcs.empty()) {
      return Traj_Candidates();
    }
    // KNN part
    if (pcs.size()<=k) {
      tr_cs[i]=pcs;
    } else {
      tr_cs[i]=Point_Candidates(k);
      std::partial_sort_copy(
        pcs.begin(),pcs.end(),
        tr_cs[i].begin(),tr_cs[i].end(),
        candidate_compare);
    }
    for (int m=0; m<tr_cs[i].size(); ++m) {
      tr_cs[i][m].index = current_candidate_index+m;
    }
    current_candidate_index+=tr_cs[i].size();
    // SPDLOG_TRACE("current_candidate_index {}",current_candidate_index);
  }
  return tr_cs;
}

const LineString &Network::get_edge_geom(int edge_id) const {
  return edges[get_edge_index(edge_id)].geom;
}

LineString Network::complete_path_to_geometry(
  const LineString &traj, const C_Path &complete_path) const
{
  // if (complete_path->empty()) return nullptr;
  LineString line;
  if (complete_path.empty()) return line;
  int Npts = traj.get_num_points();
  int NCsegs = complete_path.size();
  if (NCsegs ==1) {
    double dist;
    double firstoffset;
    double lastoffset;
    const LineString &firstseg = get_edge_geom(complete_path[0]);
    ALGORITHM::linear_referencing(traj.get_x(0),traj.get_y(0),firstseg,
                                  &dist,&firstoffset);
    ALGORITHM::linear_referencing(traj.get_x(Npts-1),traj.get_y(Npts-1),
                                  firstseg,&dist,&lastoffset);
    LineString firstlineseg= ALGORITHM::cutoffseg_unique(firstseg, firstoffset,
                                                         lastoffset);
    append_segs_to_line(&line,firstlineseg,0);
  } else {
    const LineString &firstseg = get_edge_geom(complete_path[0]);
    const LineString &lastseg = get_edge_geom(complete_path[NCsegs-1]);
    double dist;
    double firstoffset;
    double lastoffset;
    ALGORITHM::linear_referencing(traj.get_x(0),traj.get_y(0),firstseg,
                                  &dist,&firstoffset);
    ALGORITHM::linear_referencing(traj.get_x(Npts-1),traj.get_y(Npts-1),
                                  lastseg,&dist,&lastoffset);
    LineString firstlineseg= ALGORITHM::cutoffseg(firstseg, firstoffset, 0);
    LineString lastlineseg= ALGORITHM::cutoffseg(lastseg, lastoffset, 1);
    append_segs_to_line(&line,firstlineseg,0);
    if (NCsegs>2) {
      for(int i=1; i<NCsegs-1; ++i) {
        const LineString &middleseg =  get_edge_geom(complete_path[i]);
        append_segs_to_line(&line,middleseg,1);
      }
    }
    append_segs_to_line(&line,lastlineseg,1);
  }
  return line;
}

const std::vector<Point> &Network::get_vertex_points() const {
  return vertex_points;
}

const Point &Network::get_vertex_point(NodeIndex u) const {
  return vertex_points[u];
}

LineString Network::route2geometry(const std::vector<EdgeID> &path) const
{
  LineString line;
  if (path.empty()) return line;
  // if (complete_path->empty()) return nullptr;
  int NCsegs = path.size();
  for(int i=0; i<NCsegs; ++i) {
    EdgeIndex e = get_edge_index(path[i]);
    const LineString &seg = edges[e].geom;
    if (i==0) {
      append_segs_to_line(&line,seg,0);
    } else {
      append_segs_to_line(&line,seg,1);
    }
  }
  //SPDLOG_DEBUG("Path geometry is {}",line.exportToWkt());
  return line;
}

LineString Network::route2geometry(const std::vector<EdgeIndex> &path) const
{
  LineString line;
  if (path.empty()) return line;
  // if (complete_path->empty()) return nullptr;
  int NCsegs = path.size();
  for(int i=0; i<NCsegs; ++i)
  {
    const LineString &seg = edges[path[i]].geom;
    if (i==0) {
      append_segs_to_line(&line,seg,0);
    } else {
      append_segs_to_line(&line,seg,1);
    }
  }
  //SPDLOG_DEBUG("Path geometry is {}",line.exportToWkt());
  return line;
}

void Network::append_segs_to_line(LineString *line,
                                  const LineString &segs, int offset)
{
  int Npoints = segs.get_num_points();
  for(int i=0; i<Npoints; ++i) {
    if (i>=offset) {
      line->add_point(segs.get_x(i),segs.get_y(i));
    }
  }
}
