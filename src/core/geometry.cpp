#include "core/geometry.hpp"

#include <ogrsf_frmts.h> // C++ API for GDAL
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <iterator>
#include <vector>
#include <sstream>

#include "boost/geometry/extensions/gis/io/wkb/read_wkb.hpp"
#include "boost/geometry/extensions/gis/io/wkb/write_wkb.hpp"

std::ostream& FMM::CORE::operator<<(std::ostream& os,
    const FMM::CORE::LineString& rhs){
  os<< std::setprecision(12) << boost::geometry::wkt(rhs.line);
  return os;
};

FMM::CORE::LineString FMM::CORE::ogr2linestring(const OGRLineString *line){
  int binary_size = line->WkbSize();
  std::vector<unsigned char> wkb(binary_size);
  // http://www.gdal.org/ogr__core_8h.html#a36cc1f4d807ba8f6fb8951f3adf251e2
  line->exportToWkb(wkbNDR,&wkb[0]);
  LineString l;
  boost::geometry::read_wkb(wkb.begin(),wkb.end(),l.get_geometry());
  return l;
};

FMM::CORE::LineString FMM::CORE::ogr2linestring(
  const OGRMultiLineString *mline){
  FMM::CORE::LineString l;
  if (!mline->IsEmpty() && mline->getNumGeometries()>0){
    const OGRGeometry *line = mline->getGeometryRef(0);
    int binary_size = line->WkbSize();
    std::vector<unsigned char> wkb(binary_size);
    line->exportToWkb(wkbNDR,&wkb[0]);
    boost::geometry::read_wkb(wkb.begin(),wkb.end(),l.get_geometry());
  }
  return l;
};

FMM::CORE::LineString FMM::CORE::wkt2linestring(const std::string &wkt){
  FMM::CORE::LineString line;
  boost::geometry::read_wkt(wkt,line.get_geometry());
  return line;
};

OGRLineString *FMM::CORE::linestring2ogr(const FMM::CORE::LineString &line){
  std::vector<unsigned char> wkb;
  boost::geometry::write_wkb(line.get_geometry_const(),std::back_inserter(wkb));
  OGRGeometry *poGeometry;
  OGRGeometryFactory::createFromWkb(&wkb[0], NULL, &poGeometry);
  return (OGRLineString *) poGeometry;
};

OGRPoint *FMM::CORE::point2ogr(const FMM::CORE::Point &p){
  std::vector<unsigned char> wkb;
  boost::geometry::write_wkb(p,std::back_inserter(wkb));
  OGRGeometry *poGeometry;
  OGRGeometryFactory::createFromWkb(&wkb[0], NULL, &poGeometry);
  return (OGRPoint *) poGeometry;
};
