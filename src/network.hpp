/**
 * Content
 * Definition of the Network class
 *
 * @author: Can Yang
 * @version: 2020.01.23
 *     Reformat indentation
 *     Change linestring pointer to reference
 * @version: 2017.11.11
 */

#ifndef MM_NETWORK_HPP
#define MM_NETWORK_HPP
#include <ogrsf_frmts.h>
#include <iostream>
#include <cmath>
#include <algorithm>

// Data structures for Rtree
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/function_output_iterator.hpp>

#include "types.hpp"
#include "debug.h"
#include "util.hpp"
#include "algorithm.hpp"
#include "gps.hpp"

namespace MM
{

// Define a type alias for the rtree used in map matching
/* Edge format in the network */

typedef boost::geometry::model::box<boost_point> boost_box;
// Item stored in rtree
typedef std::pair<boost_box,Edge*> Item;
typedef boost::geometry::index::rtree
  <Item,boost::geometry::index::quadratic<16> > Rtree;   // Rtree definition

// This function is used for KNN sort
static bool candidate_compare(const Candidate &a, const Candidate &b)
{
  if (a.dist!=b.dist)
  {
    return a.dist<b.dist;
  }
  else
  {
    return a.edge->index<b.edge->index;
  }
};

class Network
{
public:
  /**
   *  Constructor of Network
   *
   *  A vector of network edges will be constructed
   *
   *  @param filename: the path to a network file in ESRI shapefile format
   *  @param id_name: the name of the id attribute
   *  @param source_name: the name of the source attribute
   *  @param target_name: the name of the target attribute
   *
   */
  Network(const std::string &filename,
          const std::string &id_name="id",
          const std::string &source_name="source",
          const std::string &target_name="target")
  {
    SPDLOG_INFO("Read network from file {}",filename);
    OGRRegisterAll();
    GDALDataset *poDS = (GDALDataset*) GDALOpenEx(
      filename.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL );
    if( poDS == NULL )
    {
      SPDLOG_CRITICAL("Open dataset failed.");
      exit( 1 );
    }
    OGRLayer  *ogrlayer = poDS->GetLayer(0);
    int NUM_FEATURES = ogrlayer->GetFeatureCount();

    OGRFeatureDefn *ogrFDefn = ogrlayer->GetLayerDefn();
    OGRFeature *ogrFeature;

    // Fetch the field index given field name.
    int id_idx=ogrFDefn->GetFieldIndex(id_name.c_str());
    int source_idx=ogrFDefn->GetFieldIndex(source_name.c_str());
    int target_idx=ogrFDefn->GetFieldIndex(target_name.c_str());
    SPDLOG_DEBUG("SHP ID idx: {}",id_idx);
    SPDLOG_DEBUG("SHP source idx: {}",source_idx);
    SPDLOG_DEBUG("SHP target idx: {}",target_idx);
    if (source_idx<0||target_idx<0||id_idx<0)
    {
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
      SPDLOG_TRACE("Geometry type of network is {}",
                   OGRGeometryTypeToName(ogrFDefn->GetGeomType()));
    }
    SPDLOG_TRACE("Read spatial reference");
    OGRSpatialReference *ogrsr =
      ogrFDefn->GetGeomFieldDefn(0)->GetSpatialRef();
    if (ogrsr != nullptr) {
      srid = ogrsr->GetEPSGGeogCS();
      if (srid==-1)
      {
        srid= 4326;
        SPDLOG_WARN("SRID is not found, set to 4326 by default");
      }
    } else {
      srid= 4326;
      SPDLOG_WARN("SRID is not found, set to 4326 by default");
    }
    SPDLOG_TRACE("Read data from ogrlayer");
    EdgeIndex index = 0;
    while( (ogrFeature = ogrlayer->GetNextFeature()) != NULL)
    {
      EdgeID id = ogrFeature->GetFieldAsInteger(id_idx);
      NodeID source = ogrFeature->GetFieldAsInteger(source_idx);
      NodeID target = ogrFeature->GetFieldAsInteger(target_idx);
      SPDLOG_TRACE("Read feature index {} id {} s {} t {}",
                   index, id, source, target);
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
        vertex_points.push_back(geom.getPoint(0));
      } else {
        s_idx = node_map[source];
      }
      if (node_map.find(target)==node_map.end()) {
        t_idx = node_id_vec.size();
        node_id_vec.push_back(target);
        node_map.insert({target,t_idx});
        int npoints = geom.getNumPoints();
        vertex_points.push_back(geom.getPoint(npoints-1));
      } else {
        t_idx = node_map[target];
      }
      edges.push_back({index,id,s_idx,t_idx,geom.getLength(),geom});
      edge_map.insert({id,index});
      ++index;
      OGRFeature::DestroyFeature(ogrFeature);
    }
    GDALClose( poDS );
    num_vertices = node_id_vec.size();
    SPDLOG_INFO("Read network done edges {} nodes {} srid {}",
                num_vertices,edges.size(),srid);
  };    // Network constructor

  int get_node_count(){
    return node_id_vec.size();;
  };

  // Get the ID attribute of an edge according to its index
  EdgeID get_edge_id(EdgeIndex index) const
  {
    return edges[index].id;
  };

  EdgeIndex get_edge_index(EdgeID id){
    return edge_map[id];
  };

  inline std::vector<Edge> &get_edges(){
    return edges;
  };

  NodeID get_node_id(NodeIndex index){
    return index<num_vertices ? node_id_vec[index] : -1;
  }

  NodeIndex get_node_index(NodeID id){
    return node_map[id];
  };

  boost_point get_node_geom_from_idx(NodeIndex index){
    return vertex_points[index];
  };

  NodeIDVec &get_node_id_vec(){
    return node_id_vec;
  };

  inline LineString &get_edge_geom(EdgeID eid){
    return edges[edge_map[eid]].geom;
  };

  // Construct a Rtree using the vector of edges
  void build_rtree_index()
  {
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
      Edge *edge = &edges[i];
      double x1,y1,x2,y2;
      ALGORITHM::boundingbox_geometry(edge->geom,&x1,&y1,&x2,&y2);
      boost_box b(boost_point(x1,y1), boost_point(x2,y2));
      rtree.insert(std::make_pair(b,edge));
    }
  };
  /**
   *  Search for k nearest neighboring (KNN) candidates of a
   *  trajectory within a search radius
   *
   *  @param trajectory: input trajectory
   *  @param k: the number of coordindates
   *  @param r: the search radius
   *  @return Traj_Candidates: a 2D vector of Candidates containing
   *  the candidates selected for each point in a trajectory
   *
   */
  Traj_Candidates search_tr_cs_knn(Trajectory &trajectory, std::size_t k,
                                   double radius,double gps_error){
    return search_tr_cs_knn(trajectory.geom,k,radius,gps_error);
  }

  /**
   *  Search for k nearest neighboring (KNN) candidates of a
   *  linestring within a search radius
   */
  Traj_Candidates search_tr_cs_knn(const LineString &geom, std::size_t k,
                                   double radius, double gps_error)
  {
    int NumberPoints = geom.getNumPoints();
    Traj_Candidates tr_cs(NumberPoints);
    for (int i=0; i<NumberPoints; ++i)
    {
      double px = geom.getX(i);
      double py = geom.getY(i);
      Point_Candidates pcs;
      boost_box b(boost_point(geom.getX(i)-radius,geom.getY(i)-radius),
                  boost_point(geom.getX(i)+radius,geom.getY(i)+radius));
      std::vector<Item> temp;
      rtree.query(boost::geometry::index::intersects(b),
                  std::back_inserter(temp));
      for (Item &i:temp)
      {
        Edge *edge = i.second;
        double offset;
        double dist;
        ALGORITHM::linear_referencing(px,py,edge->geom,&dist,&offset);
        if (dist<=radius)
        {
          Candidate c = {offset,dist,Network::emission_prob(dist,gps_error),
                         edge,nullptr,0,0};
          pcs.push_back(c);
        }
      }
      // KNN part
      if (pcs.size()<=k)
      {
        tr_cs[i]=pcs;
      }
      else
      {
        // Find the KNN neighbors
        tr_cs[i]=Point_Candidates(k);
        std::partial_sort_copy(
          pcs.begin(),pcs.end(),
          tr_cs[i].begin(),tr_cs[i].end(),
          candidate_compare);
      }
    }
    return tr_cs;
  };

  /**
   * Added in 2018.01.17, by Diao
   * Modified in 2018.01.19 by Can
   *
   * a modified version of geometry path construction,
   * which the offset is taken into consideration, and the
   * final path with a correct start point and end point corresponding
   * to the original trajectory. Namely at both the first and last edge
   * in the complete path, only the part traversed is exported.
   *
   * Construct the geometry of a complete path,
   * @param o_path_ptr: a pointer to the o_path_ptr, where each element
   * is a Candidate object. This input is needed because the offset is used to
   * split the first and last edge in the complete path.
   * @param  complete_path: a pointer to a complete path, where each element
   * is an integer representing spatial contiguous edge index
   * @return  a pointer to the geometry of the complete path, The
   * caller should take care of freeing its memory.
   */
  LineString complete_path_to_geometry(const O_Path &o_path,
                                       const C_Path &complete_path)
  {
    LineString line;
    SPDLOG_TRACE("Opath {}", opath2string(o_path));
    SPDLOG_TRACE("Complete path {}", vec2string<EdgeID>(complete_path));
    if (complete_path.empty()) return line;
    // If complete path contains -1, export empty line
    for (auto &item:complete_path) {
      if (item == -1) return line;
    }
    int NOsegs = o_path.size();
    int NCsegs = complete_path.size();
    if (NCsegs ==1)
    {
      LineString &firstseg = get_edge_geom(complete_path[0]);
      double firstoffset = o_path[0]->offset;
      double lastoffset =  o_path[NOsegs-1]->offset;
      LineString firstlineseg= ALGORITHM::cutoffseg_unique(
        firstoffset,lastoffset,firstseg);
      append_segs_to_line(&line,firstlineseg,0);
    } else {
      LineString &firstseg = get_edge_geom(complete_path[0]);
      LineString &lastseg = get_edge_geom(complete_path[NCsegs-1]);
      double firstoffset = o_path[0]->offset;
      double lastoffset =  o_path[NOsegs-1]->offset;
      LineString firstlineseg= ALGORITHM::cutoffseg(firstoffset, firstseg, 0);
      LineString lastlineseg= ALGORITHM::cutoffseg(lastoffset, lastseg, 1);
      append_segs_to_line(&line,firstlineseg,0);
      if (NCsegs>2)
      {
        for(int i=1; i<NCsegs-1; ++i)
        {
          LineString &middleseg =  get_edge_geom(complete_path[i]);
          append_segs_to_line(&line,middleseg,1);
        }
      };
      append_segs_to_line(&line,lastlineseg,1);
    }
    return line;
  };

  MultiLineString ot_path_to_multilinestring(
    const O_Path &o_path,const T_Path &t_path)
  {
    SPDLOG_TRACE("Opath {}", opath2string(o_path));
    SPDLOG_TRACE("T path index {}", vec2string<int>(t_path.indices));
    SPDLOG_TRACE("Complete path {}", vec2string<EdgeID>(t_path.cpath));
    if (t_path.cpath.empty()) return {};
    std::vector<LineString> lines;
    // Iterate through consecutive indexes and write the traversed path
    int J = t_path.indices.size();
    for (int j=0; j<J-1; ++j) {
      O_Path lopath;
      lopath.push_back(o_path[j]);
      lopath.push_back(o_path[j+1]);
      C_Path lcpath;
      int a = t_path.indices[j];
      int b = t_path.indices[j+1];
      for (int i=a; i<b; ++i) {
        lcpath.push_back(t_path.cpath[i]);
      }
      lcpath.push_back(t_path.cpath[b]);
      SPDLOG_TRACE("Complete path j {} J {}", j, J);
      lines.push_back(complete_path_to_geometry(lopath,lcpath));
    }
    return MultiLineString(lines);
  };


  /**
   * Calculate the emission probability
   * @param  dist:the distance a GPS point to a candidate point
   * @return  the emission probability
   */
  static double emission_prob(double dist,double gps_error)
  {
    double a = dist / gps_error;
    return exp(-0.5 * a * a);     // The unit should be in range of 0,1
  };

  static double emission_prob_to_dist(double eprob,double gps_error){
    return sqrt(-2 * log(eprob))*gps_error;
  };
private:
  /**
   * Concatenate a linestring segs to a linestring line, used in the
   * function complete_path_to_geometry
   *
   * @param line: pointer to a linestring which will be updated
   * @param segs: pointer to a linestring
   * @param offset: the number of points skipped in segs.
   */
  static void append_segs_to_line(LineString *line, const LineString &segs,
                                  int offset=0)
  {
    int Npoints = segs.getNumPoints();
    for(int i=0; i<Npoints; ++i)
    {
      if (i>=offset)
      {
        line->addPoint(segs.getX(i),segs.getY(i));
      }
    }
  };
  int srid;   // Spatial reference id
  Rtree rtree;   // Network rtree structure
  std::vector<Edge> edges;   // all edges in the network
  NodeIDVec node_id_vec;
  unsigned int num_vertices;
  NodeIndexMap node_map;
  EdgeIndexMap edge_map;
  std::vector<boost_point> vertex_points;
}; // Network
} // MM
#endif /* MM_NETWORK_HPP */
