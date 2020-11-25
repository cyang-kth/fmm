/**
 * 2020-11-25 Remove linestring2ogr, wkb dependency 
 */

#include "core/geometry.hpp"

#include <ogrsf_frmts.h> // C++ API for GDAL
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <iterator>
#include <vector>
#include <sstream>

std::ostream& FMM::CORE::operator<<(std::ostream& os,
    const FMM::CORE::LineString& rhs){
  os<< std::setprecision(12) << boost::geometry::wkt(rhs.line);
  return os;
};

FMM::CORE::LineString FMM::CORE::ogr2linestring(const OGRLineString *line){
  FMM::CORE::LineString l;
  int Npoints = line->getNumPoints();
  for (int i=0;i<Npoints;++i){
    l.add_point(line->getX(i),line->getY(i));
  }
  return l;
};

FMM::CORE::LineString FMM::CORE::ogr2linestring(
  const OGRMultiLineString *mline){
  FMM::CORE::LineString l;
  if (!mline->IsEmpty() && mline->getNumGeometries()>0){
    const OGRLineString *line = (OGRLineString *) mline->getGeometryRef(0);
    int Npoints = line->getNumPoints();
    for (int i=0;i<Npoints;++i){
      l.add_point(line->getX(i),line->getY(i));
    }
  }
  return l;
};

FMM::CORE::LineString FMM::CORE::wkt2linestring(const std::string &wkt){
  FMM::CORE::LineString line;
  boost::geometry::read_wkt(wkt,line.get_geometry());
  return line;
};
