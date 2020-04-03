#ifndef MM_GEOMTYPES_HPP
#define MM_GEOMTYPES_HPP

#include <ogrsf_frmts.h> // C++ API for GDAL
#include <boost/geometry.hpp>
#include <string>
#include <sstream>

namespace MM {

typedef boost::geometry::model::point<double, 2,
    boost::geometry::cs::cartesian> Point; // Point for rtree box
/**
 *  Boost Geometry Linestring, compatible with OGRGeometry
 */
class LineString {
public:
  typedef boost::geometry::model::linestring<Point> linestring_t;
  inline double get_x(int i) const{
    return boost::geometry::get<0>(line.at(i));
  };
  inline double get_y(int i) const{
    return boost::geometry::get<1>(line.at(i));
  };
  inline void set_x(int i, double v){
    boost::geometry::set<0>(line.at(i),v);
  };
  inline void set_y(int i, double v){
    boost::geometry::set<1>(line.at(i),v);
  };
  inline void add_point(double x,double y){
    boost::geometry::append(line, Point(x,y));
  };
  inline void add_point(const Point& point){
    boost::geometry::append(line, point);
  };
  inline Point get_point(int i) const{
    return Point(boost::geometry::get<0>(
      line.at(i)),boost::geometry::get<1>(line.at(i)));
  };
  inline const Point &at(int i) const{
    return line.at(i);
  }
  inline int get_num_points() const{
    return boost::geometry::num_points(line);
  };
  inline bool is_empty(){
    return boost::geometry::num_points(line)==0;
  };
  inline void clear(){
    boost::geometry::clear(line);
  };
  inline double get_length() const {
    return boost::geometry::length(line);
  };
  inline std::string export_wkt() const{
    std::ostringstream ss;
    ss << boost::geometry::wkt(line);
    return ss.str();
  };
  inline std::string export_json() const{
    std::ostringstream ss;
    int N = get_num_points();
    if (N>0){
      ss << "{\"type\":\"LineString\",\"coordinates\": [";
      for (int i=0;i<N;++i){
        ss << "[" << get_x(i) << "," << get_y(i) <<"]"
           << (i==N-1 ? "": ",");
      }
      ss << "]}";
    }
    return ss.str();
  };
  inline const linestring_t &get_geometry_const() const{
    return line;
  };
  linestring_t &get_geometry(){
    return line;
  };
  inline bool operator==(const LineString& rhs) const {
    int N = get_num_points();
    if (rhs.get_num_points()!=N)
      return false;
    bool result = true;
    for (int i=0;i<N;++i){
      if (boost::geometry::distance(get_point(i),rhs.get_point(i))>1e-6)
        result = false;
    }
    return result;
  };
  friend std::ostream& operator<<(std::ostream& os, const LineString& rhs);
private:
  linestring_t line;
}; // LineString

/**
 *  Convert an OGRLineString to Boost geometry, the caller is responsible to
 *  freeing the memory.
 *
 */
LineString ogr2linestring(const OGRLineString *line);

LineString ogr2linestring(const OGRMultiLineString *mline);

LineString wkt2linestring(const std::string &wkt);

OGRLineString *linestring2ogr(const LineString &line, int srid=4326);

OGRPoint *point2ogr(const Point &p, int srid=4326);

}; // MM


#endif // MM_GEOMTYPES_HPP
