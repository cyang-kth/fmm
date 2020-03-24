/**
 * Content
 * Definition of MatchResultWriter Class, which contains functions for
 * writing the results.
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#include "io/mm_writer.hpp"
#include "util/util.hpp"
#include "util/debug.hpp"

#include <iostream>
#include <sstream>

namespace MM
{

namespace IO {

/**
 * Constructor
 * @param result_file, the path to an output file
 * @param network_ptr, a pointer to the network
 */
CSVMatchResultWriter::CSVMatchResultWriter(const std::string &result_file,
                                           OutputConfig &config_arg) :
  m_fstream(result_file), config(config_arg)
{
  write_header();
};

void CSVMatchResultWriter::write_result(int id, const O_Path &opath,
                                        const C_Path &cpath,
                                        const LineString &geom){
  std::stringstream buf;
  buf << id;
  if (config.write_opath) {
    buf << ";" << opath;
  }
  if (config.write_cpath) {
    buf << ";" << cpath;
  }
  if (config.write_mgeom) {
    buf << ";";
    buf << geom;
  }
  buf << '\n';
    #pragma omp critical
  m_fstream << buf.rdbuf();
};

void CSVMatchResultWriter::write_header() {
  std::string header = "id";
  if (config.write_opath) header+=";opath";
  if (config.write_cpath) header+=";cpath";
  if (config.write_mgeom) header+=";mgeom";
  m_fstream << header << '\n';
};

void CSVMatchResultWriter::write_geometry(std::stringstream &buf,
                                          const LineString &line){
  buf << std::setprecision(12) << line.export_wkt();
}
std::string CSVMatchResultWriter::match_result2geojson(
    const MatchResult &result) {
  std::ostringstream oss;
  oss <<"{\"type\":\"Feature\",\"geometry\":"
      << result.mgeom.export_json()
      << ",\"properties\":{"
      <<"\"trid\":" << result.id
      << ",\"cpath\":["
      << result.cpath << "]}}\n";
  std::string s = oss.str();
  return s;
};


/**
 * Constructor
 * @param result_file, the path to an output file
 * @param network_ptr, a pointer to the network
 */
GDALMatchResultWriter::GDALMatchResultWriter(const std::string &result_file,
                                             OutputConfig &config_arg) :
  config(config_arg) {
  const char *pszDriverName = "SQLite";
  GDALDriver *poDriver;
  GDALAllRegister();
  poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
  if ( poDriver == NULL )
  {
    SPDLOG_CRITICAL("{} driver not avaiable",pszDriverName);
    std::exit(EXIT_FAILURE);
  }
  poDS = poDriver->Create(result_file.c_str(), 0, 0, 0, GDT_Unknown,
                          NULL);
  if ( poDS == NULL )
  {
    SPDLOG_CRITICAL("Creation of output file failed.");
    std::exit(EXIT_FAILURE);
  }
  poLayer = poDS->CreateLayer("layer", NULL, wkbLineString, NULL );
  if ( poLayer == NULL )
  {
    SPDLOG_CRITICAL("Layer creation failed.");
    std::exit(EXIT_FAILURE);
  }
  OGRFieldDefn oField1("id", OFTInteger);
  if (poLayer->CreateField( &oField1 ) != OGRERR_NONE )
  {
    SPDLOG_CRITICAL("Creating int field failed.");
    std::exit(EXIT_FAILURE);
  }
  id_idx = poLayer->FindFieldIndex("id",0);
  if (config.write_opath) {
    OGRFieldDefn oField2("opath", OFTIntegerList);
    if (poLayer->CreateField( &oField2 ) != OGRERR_NONE )
    {
      SPDLOG_CRITICAL("Creating int list field for opath failed.");
      std::exit(EXIT_FAILURE);
    }
    opath_idx = poLayer->FindFieldIndex("opath",0);
  }
  if (config.write_cpath) {
    OGRFieldDefn oField3("cpath", OFTIntegerList);
    if (poLayer->CreateField( &oField3 ) != OGRERR_NONE )
    {
      SPDLOG_CRITICAL("Creating int list field for cpath failed.");
      std::exit(EXIT_FAILURE);
    }
    cpath_idx = poLayer->FindFieldIndex("cpath",0);
  }
  SPDLOG_INFO("Finish with writing meta data");
};

void GDALMatchResultWriter::write_result(
  int id, O_Path &opath, C_Path &cpath, LineString &geom){
  OGRFeature *poFeature;
  poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());
  poFeature->SetField(id_idx,id);
  if (config.write_opath) poFeature->SetField(opath_idx,
                                              opath.size(),
                                              &(opath[0]));
  if (config.write_cpath) poFeature->SetField(cpath_idx,
                                              cpath.size(),
                                              &(cpath[0]));
  OGRGeometry *poGeometry = linestring2ogr(geom);
  poFeature->SetGeometryDirectly(poGeometry);
  // Think about this part
    #pragma omp critical
  if ( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
  {
    SPDLOG_ERROR( "Failed to create feature in GPKG.\n");
    std::exit(EXIT_FAILURE);
  }
  OGRFeature::DestroyFeature( poFeature );
};

};     //IO
} //MM
