/**
 * Content
 * Definition of a TrajectoryReader which is a wrapper of
 * the standard shapefile reader in GDAL.
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */
#include "io/mm_reader.hpp"
#include "util/debug.hpp"
#include "util/util.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

namespace MM
{
namespace IO
{

GDALMatchResultReader::GDALMatchResultReader(const std::string & filename,
                                   const std::string & id_name,
                                   const std::string & opath_name,
                                   const std::string & cpath_name) {
  SPDLOG_INFO("Read matched trajectory from file {}",filename);
  OGRRegisterAll();
  poDS = (GDALDataset*) GDALOpenEx(filename.c_str(),
                                   GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    SPDLOG_CRITICAL("Open data source fail");
    exit( 1 );
  }
  ogrlayer = poDS->GetLayer(0);
  _cursor=0;
  // Get the number of features first
  OGRFeatureDefn *ogrFDefn = ogrlayer->GetLayerDefn();
  NUM_FEATURES= ogrlayer->GetFeatureCount();
  // This should be a local field rather than a new variable
  id_idx=ogrFDefn->GetFieldIndex(id_name.c_str());
  if (id_idx<0)
  {
    SPDLOG_CRITICAL("Id column {} not found",id_name);
    GDALClose( poDS );
    std::exit(EXIT_FAILURE);
  }
  opath_idx=ogrFDefn->GetFieldIndex(opath_name.c_str());
  if (opath_idx<0)
  {
    SPDLOG_CRITICAL("Opath column {} not found",id_name);
    GDALClose( poDS );
    std::exit(EXIT_FAILURE);
  }
  cpath_idx=ogrFDefn->GetFieldIndex(cpath_name.c_str());
  if (cpath_idx<0)
  {
    SPDLOG_CRITICAL("Cpath column {} not found",id_name);
    GDALClose( poDS );
    std::exit(EXIT_FAILURE);
  }
  if (wkbFlatten(ogrFDefn->GetGeomType()) != wkbLineString)
  {
    SPDLOG_CRITICAL("Geometry type is {}, which should be linestring",
                    OGRGeometryTypeToName(ogrFDefn->GetGeomType()));
    GDALClose( poDS );
    std::exit(EXIT_FAILURE);
  } else {
    SPDLOG_INFO("Geometry type is {}",
                OGRGeometryTypeToName(ogrFDefn->GetGeomType()));
  }
  SPDLOG_INFO("ID index {} cpath index {}",id_idx,cpath_idx);
  SPDLOG_INFO("Total number of trajectories {}", NUM_FEATURES);
  SPDLOG_INFO("Finish reading meta data");
};
// If there are still features not read
bool GDALMatchResultReader::has_next_feature()
{
  return _cursor<NUM_FEATURES;
};
// Read the next trajectory in the shapefile
MatchResult GDALMatchResultReader::read_next_trajectory()
{
  OGRFeature *ogrFeature =ogrlayer->GetNextFeature();
  int trid = ogrFeature->GetFieldAsInteger(id_idx);

  int opath_size;
  const int *opath_data = ogrFeature->GetFieldAsIntegerList(
    opath_idx, &opath_size);
  std::vector<int> opath{opath_data,opath_data+opath_size};

  int cpath_size;
  const int *cpath_data = ogrFeature->GetFieldAsIntegerList(
    cpath_idx, &cpath_size);
  std::vector<int> cpath{cpath_data,cpath_data+cpath_size};
  OGRGeometry *rawgeometry = ogrFeature->GetGeometryRef();
  LineString linestring = ogr2linestring((OGRLineString*) rawgeometry);
  OGRFeature::DestroyFeature(ogrFeature);
  ++_cursor;
  return MatchResult{trid,opath,cpath,linestring};
};

// Get the number of trajectories in the file
int GDALMatchResultReader::get_num_trajectories()
{
  return NUM_FEATURES;
};

void GDALMatchResultReader::close(){
  GDALClose( poDS );
};

} // IO
} // MM
