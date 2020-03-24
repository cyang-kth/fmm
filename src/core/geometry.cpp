#include "core/geometry.hpp"

#include <ogrsf_frmts.h> // C++ API for GDAL
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <iterator>
#include <vector>
#include <sstream>

#include "boost/geometry/extensions/gis/io/wkb/read_wkb.hpp"
#include "boost/geometry/extensions/gis/io/wkb/write_wkb.hpp"

namespace MM {

std::ostream& operator<<(std::ostream& os, const LineString& rhs){
  os<< std::setprecision(12) << bg::wkt(rhs.line);
  return os;
};

/**
 *  Convert an OGRLineString to Boost geometry, the caller is responsible to
 *  freeing the memory.
 *
 */
LineString ogr2linestring(const OGRLineString *line){
  int binary_size = line->WkbSize();
  std::vector<unsigned char> wkb(binary_size);
  // http://www.gdal.org/ogr__core_8h.html#a36cc1f4d807ba8f6fb8951f3adf251e2
  line->exportToWkb(wkbNDR,&wkb[0]);
  LineString l;
  bg::read_wkb(wkb.begin(),wkb.end(),l.get_geometry());
  return l;
};

LineString ogr2linestring(const OGRMultiLineString *mline){
  LineString l;
  if (!mline->IsEmpty() && mline->getNumGeometries()>0){
    const OGRGeometry *line = mline->getGeometryRef(0);
    int binary_size = line->WkbSize();
    std::vector<unsigned char> wkb(binary_size);
    line->exportToWkb(wkbNDR,&wkb[0]);
    bg::read_wkb(wkb.begin(),wkb.end(),l.get_geometry());
  }
  return l;
};

LineString wkt2linestring(const std::string &wkt){
  LineString line;
  boost::geometry::read_wkt(wkt,line.get_geometry());
  return line;
};

OGRLineString *linestring2ogr(const LineString &line, int srid){
  std::vector<unsigned char> wkb;
  bg::write_wkb(line.get_geometry_const(),std::back_inserter(wkb));
  OGRGeometry *poGeometry;
  OGRGeometryFactory::createFromWkb(&wkb[0], NULL, &poGeometry);
  return (OGRLineString *) poGeometry;
};

OGRPoint *point2ogr(const Point &p, int srid){
  std::vector<unsigned char> wkb;
  bg::write_wkb(p,std::back_inserter(wkb));
  OGRGeometry *poGeometry;
  OGRGeometryFactory::createFromWkb(&wkb[0], NULL, &poGeometry);
  return (OGRPoint *) poGeometry;
};

}; // MM
