#ifndef MM_GEOMTYPES_HPP
#define MM_GEOMTYPES_HPP

#include <ogrsf_frmts.h> // C++ API for GDAL
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <iterator>
#include <vector>

#include "boost/geometry/extensions/gis/io/wkb/read_wkb.hpp"
#include "boost/geometry/extensions/gis/io/wkb/write_wkb.hpp"

namespace MM {

namespace bg = boost::geometry;
// Point for rtree box
typedef bg::model::point<double, 2, bg::cs::cartesian> boost_point;
typedef bg::model::linestring<boost_point> linestring_t;
typedef bg::model::multi_linestring<linestring_t> mlinestring_t;

/**
 *  Boost Geometry Linestring, compatible with OGRGeometry
 */
class LineString {
public:
  inline double getX(int i) const {
    return bg::get<0>(line.at(i));
  };
  inline double getY(int i) const {
    return bg::get<1>(line.at(i));
  };
  inline void setX(int i, double v) {
    bg::set<0>(line.at(i),v);
  };
  inline void setY(int i, double v) {
    bg::set<1>(line.at(i),v);
  };
  inline void addPoint(double x,double y){
    bg::append(line, boost_point(x,y));
  };
  inline void addPoint(const boost_point& point){
    bg::append(line, point);
  };
  inline boost_point getPoint(int i) const {
    return boost_point(bg::get<0>(line.at(i)),bg::get<1>(line.at(i)));
  };
  inline int getNumPoints() const {
    return bg::num_points(line);
  };
  inline bool isEmpty() const {
    return bg::num_points(line)==0;
  };
  bg::wkt_manipulator<linestring_t> exportToWkt() const {
    return bg::wkt(line);
  };
  linestring_t &get_geometry(){
    return line;
  };
  inline void clear(){
    bg::clear(line);
  };
  inline double getLength() const {
    return bg::length(line);
  };
private:
  linestring_t line;
}; // LineString

class MultiLineString {
public:
  MultiLineString(){};
  MultiLineString(const std::vector<LineString> &lines){
    mline.resize(lines.size());
    for (int i=0; i<lines.size(); ++i) {
      int J = lines[i].getNumPoints();
      for (int j=0; j<J; ++j) {
        bg::append(mline[i],lines[i].getPoint(j));
      }
    }
  };
  inline bool isEmpty() const {
    return bg::num_points(mline)==0;
  };
  bg::wkt_manipulator<mlinestring_t> exportToWkt() const {
    return bg::wkt(mline);
  };
private:
  mlinestring_t mline;
};

/**
 *  Convert an OGRLineString to Boost geometry, the caller is responsible to
 *  freeing the memory.
 */
LineString ogr2linestring(OGRLineString *line){
  int binary_size = line->WkbSize();
  std::vector<unsigned char> wkb(binary_size);
  line->exportToWkb(wkbNDR,&wkb[0]);
  LineString l;
  bg::read_wkb(wkb.begin(),wkb.end(),l.get_geometry());
  return l;
};

LineString ogr2linestring(OGRMultiLineString *mline){
  if (mline->IsEmpty()) return {};
  OGRGeometryCollection *lines = mline->toUpperClass();
  OGRGeometry *line = lines->getGeometryRef(0);
  int binary_size = line->WkbSize();
  std::vector<unsigned char> wkb(binary_size);
  line->exportToWkb(wkbNDR,&wkb[0]);
  LineString l;
  bg::read_wkb(wkb.begin(),wkb.end(),l.get_geometry());
  return l;
};

OGRLineString *linestring2ogr(LineString &line, int srid=4326){
  std::vector<unsigned char> wkb;
  bg::write_wkb(line.get_geometry(),std::back_inserter(wkb));
  OGRGeometry *poGeometry;
  OGRGeometryFactory::createFromWkb(&wkb[0], NULL, &poGeometry);
  return (OGRLineString *) poGeometry;
};

OGRPoint *bg2ogr_point(boost_point &p, int srid=4326){
  std::vector<unsigned char> wkb;
  bg::write_wkb(p,std::back_inserter(wkb));
  OGRGeometry *poGeometry;
  OGRGeometryFactory::createFromWkb(&wkb[0], NULL, &poGeometry);
  return (OGRPoint *) poGeometry;
};

}; // MM


#endif // MM_GEOMTYPES_HPP
