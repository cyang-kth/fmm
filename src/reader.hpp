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
class TrajectoryReader
{
public:
  /**
   *  Constructor of TrajectoryReader
   *  @param filename, a GPS ESRI shapefile path
   *  @param id_name, the ID column name in the GPS shapefile
   */
  TrajectoryReader(const std::string & filename,const std::string & id_name)
  {
    std::cout<<"Reading meta data of GPS trajectories from: "<<filename<< '\n';
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
  // Read the next N trajectories in the shapefile
  std::vector<Trajectory> read_next_N_trajectories(int N=30000)
  {
    int trajectories_size = NUM_FEATURES-_cursor<N ? NUM_FEATURES-_cursor : N;
    std::vector<Trajectory> trajectories;
    int i=0;
    while(i<trajectories_size)
    {
      OGRFeature *ogrFeature =ogrlayer->GetNextFeature();
      int trid = ogrFeature->GetFieldAsInteger(id_idx);
      OGRGeometry *rawgeometry = ogrFeature->GetGeometryRef();
      LineString linestring = ogr2linestring((OGRLineString*) rawgeometry);
      OGRFeature::DestroyFeature(ogrFeature);
      trajectories.push_back({trid,linestring});
      ++i;
    }
    _cursor+=trajectories_size;
    return trajectories;
  };
  // Read all trajectories at once, which can consume a lot of
  // memories
  std::vector<Trajectory> read_all_trajectories()
  {
    std::cout<<"\t Read all trajectoires" << '\n';
    std::vector<Trajectory> trajectories;
    int i=0;
    while(i<NUM_FEATURES)
    {
      OGRFeature *ogrFeature =ogrlayer->GetNextFeature();
      int trid = ogrFeature->GetFieldAsInteger(id_idx);
      OGRGeometry *rawgeometry = ogrFeature->GetGeometryRef();
      LineString linestring = ogr2linestring((OGRLineString*) rawgeometry);
      OGRFeature::DestroyFeature(ogrFeature);
      trajectories.push_back({trid,linestring});
      ++i;
    }
    std::cout<<"\t Read trajectory set size : "<< i <<'\n';
    return trajectories;
  };
  // Get the number of trajectories in the file
  int get_num_trajectories()
  {
    return NUM_FEATURES;
  };
  ~TrajectoryReader()
  {
    GDALClose( poDS );
  };
private:
  int NUM_FEATURES=0;
  int id_idx;   // Index of the id column in shapefile
  int _cursor=0;   // Keep record of current features read
  GDALDataset *poDS;
  OGRLayer  *ogrlayer;
}; // TrajectoryReader
} // IO
} // MM
#endif // MM_READER_HPP
