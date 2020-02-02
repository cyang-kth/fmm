/**
 * Content
 * Definition of a TrajectoryReader which is a wrapper of
 * the standard shapefile reader in GDAL.
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */

#ifndef MM_READER_HPP /* Currently not used */
#define MM_READER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "debug.h"
#include "types.hpp"
#include "gps.hpp"
#include "util.hpp"

namespace MM
{
namespace IO
{

// An interface for trajectory reader
class TrajReaderInterface {
public:
  virtual Trajectory read_next_trajectory() = 0;
  virtual bool has_next_feature() = 0;
  std::vector<Trajectory> read_next_N_trajectories(int N){
    std::vector<Trajectory> trajectories;
    int i = 0;
    while(i<N && has_next_feature()) {
      trajectories.push_back(read_next_trajectory());
      ++i;
    }
    return trajectories;
  };
  std::vector<Trajectory> read_all_trajectories(){
    std::vector<Trajectory> trajectories;
    int i = 0;
    while(has_next_feature()) {
      trajectories.push_back(read_next_trajectory());
      ++i;
    }
    return trajectories;
  };
};

/**
 *  According to the documentation at http://gdal.org/1.11/ogr/ogr_apitut.html
 *
 *  Note that OGRFeature::GetGeometryRef() and OGRFeature::GetGeomFieldRef()
 *  return a pointer to the internal geometry owned by the OGRFeature. There
 *  we don't actually deleted the return geometry. However, the
 *  OGRLayer::GetNextFeature() method returns a copy of the feature that is
 *  now owned by us. So at the end of use we must free the feature.
 *
 *  It implies that when we delete the feature, the geometry returned by
 *  OGRFeature::GetGeometryRef() is also deleted. Therefore, we need to
 *  create a copy of the geometry and free it with
 *      OGRGeometryFactory::destroyGeometry(geometry_pointer);
 *
 */
class TrajectoryGDALReader : public TrajReaderInterface
{
public:
  /**
   *  Constructor of TrajectoryGDALReader
   *  @param filename, a GPS ESRI shapefile path
   *  @param id_name, the ID column name in the GPS shapefile
   */
  TrajectoryGDALReader(const std::string & filename,
                       const std::string & id_name)
  {
    SPDLOG_INFO("Reading GPS trajectories GDAL format from {}", filename);
    OGRRegisterAll();
    poDS = (GDALDataset*) GDALOpenEx(filename.c_str(),
                                     GDAL_OF_VECTOR, NULL, NULL, NULL );
    if( poDS == NULL )
    {
      SPDLOG_CRITICAL("Open GDAL dataset failed {}",filename);
      std::exit(EXIT_FAILURE);
    }
    ogrlayer = poDS->GetLayer(0);
    _cursor=0;
    OGRFeatureDefn *ogrFDefn = ogrlayer->GetLayerDefn();
    id_idx=ogrFDefn->GetFieldIndex(id_name.c_str());
    NUM_FEATURES= ogrlayer->GetFeatureCount();
    if (id_idx<0)
    {
      SPDLOG_CRITICAL("Id column {} not found",id_name);
      GDALClose( poDS );
      std::exit(EXIT_FAILURE);
    }
    if (wkbFlatten(ogrFDefn->GetGeomType()) != wkbLineString)
    {
      SPDLOG_CRITICAL("Geometry type is not linestring {}",
                      OGRGeometryTypeToName(ogrFDefn->GetGeomType()));
      GDALClose( poDS );
      std::exit(EXIT_FAILURE);
    } else {
      SPDLOG_INFO("Geometry type is {}",
                  OGRGeometryTypeToName(ogrFDefn->GetGeomType()));
    }
    SPDLOG_INFO("Index of id column {}",id_idx);
    SPDLOG_INFO("Total number of trajectories {}",NUM_FEATURES);
    SPDLOG_INFO("Finish reading meta data");
  };
  // If there are still features not read
  bool has_next_feature()
  {
    return _cursor<NUM_FEATURES;
  };
  // Read the next trajectory in the shapefile
  Trajectory read_next_trajectory()
  {
    OGRFeature *ogrFeature =ogrlayer->GetNextFeature();
    int trid = ogrFeature->GetFieldAsInteger(id_idx);
    OGRGeometry *rawgeometry = ogrFeature->GetGeometryRef();
    LineString linestring = ogr2linestring((OGRLineString*) rawgeometry);
    OGRFeature::DestroyFeature(ogrFeature);
    ++_cursor;
    return Trajectory{trid,linestring};
  };

  // Get the number of trajectories in the file
  int get_num_trajectories()
  {
    return NUM_FEATURES;
  };

  ~TrajectoryGDALReader()
  {
    GDALClose( poDS );
  };
private:
  int NUM_FEATURES=0;
  int id_idx;   // Index of the id column in shapefile
  int _cursor=0;   // Keep record of current features read
  GDALDataset *poDS;
  OGRLayer  *ogrlayer;
}; // TrajectoryGDALReader

class TrajectoryCSVReader : public TrajReaderInterface {
public:
  TrajectoryCSVReader(const std::string &e_filename,
                      const std::string &id_name,
                      const std::string &geom_name) :
    ifs(e_filename){
    SPDLOG_INFO("Reading GPS trajectories CSV format from {}", e_filename);
    std::string line;
    std::getline(ifs, line);
    std::stringstream check1(line);
    std::string intermediate;
    // Tokenizing w.r.t. space ' '
    int i = 0;
    while(getline(check1, intermediate, delim))
    {
      if (intermediate.compare(id_name) == 0) {
        id_idx = i;
      }
      if (intermediate.compare(geom_name) == 0) {
        geom_idx = i;
      }
      ++i;
    }
    if (id_idx<0 ||geom_idx<0) {
      SPDLOG_CRITICAL("Id {} or Geometry column {} not found",
                      id_name,geom_name);
      std::exit(EXIT_FAILURE);
    }
    SPDLOG_INFO("Id index {} Geometry index {}",id_idx,geom_idx);
  };
  Trajectory read_next_trajectory(){
    // Read the geom idx column into a trajectory
    std::string line;
    std::getline(ifs, line);
    std::stringstream ss(line);
    int trid;
    int index=0;
    std::string intermediate;
    LineString linestring;
    while (std::getline(ss,intermediate,delim)) {
      if (index == id_idx) {
        trid = std::stoi(intermediate);
      }
      if (index == geom_idx) {
        boost::geometry::read_wkt(intermediate,linestring->get_geometry());
      }
      ++index;
    }
    return Trajectory{trid,linestring};
  };
  bool has_next_feature() {
    return ifs.peek() != EOF;
  };
  void reset_cursor(){
    ifs.clear();
    ifs.seekg(0, std::ios::beg);
    std::string line;
    std::getline(ifs, line);
  };
  void close(){
    ifs.close();
  };
private:
  std::fstream ifs;
  int id_idx=-1;
  int geom_idx=-1;
  char delim = ';';
}; // TrajectoryCSVReader

class GPSReader {
public:
  // GDAL format
  GPSReader(const std::string &filename,
            const std::string &id_name){
    mode = 0;
    reader = ;
    reader = TrajectoryGDALReader(filename,id_name);
  };

  // Trajectory CSV format
  GPSReader(const std::string &filename, const std::string &id_name,
            const std::string &geom_name){
    mode = 1;
    reader = TrajectoryCSVReader(filename,id_name,geom_name);
  };

  inline Trajectory read_next_trajectory(){
    return reader->read_next_trajectory();
  };
  inline bool has_next_feature(){
    return reader->has_next_feature;
  };
  inline std::vector<Trajectory> read_next_N_trajectories(int N){
    return reader->read_next_N_trajectories(N);
  };
  inline std::vector<Trajectory> read_all_trajectories(){
    return reader->read_all_trajectories();
  };

private:
  TrajReaderInterface *reader;
  // 0 for GDAL, 1 for CSV
  int mode;
};

} // IO
} // MM
#endif // MM_READER_HPP
