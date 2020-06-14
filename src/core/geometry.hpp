/**
 * Fast map matching.
 *
 * Definition of geometry
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef FMM_GEOMTYPES_HPP
#define FMM_GEOMTYPES_HPP

#include <ogrsf_frmts.h> // C++ API for GDAL
#include <boost/geometry.hpp>
#include <string>
#include <sstream>

namespace FMM {
/**
 * Core data types
 */
namespace CORE{

/**
 *  Point class
 */
typedef boost::geometry::model::point<double, 2,
    boost::geometry::cs::cartesian> Point; // Point for rtree box
/**
 *  Linestring geometry class
 *
 *  This class wraps a boost linestring geometry.
 */
class LineString {
public:
  /**
   * This is the boost geometry linestring class, stored inside the
   * LineString class.
   */
  typedef boost::geometry::model::linestring<Point> linestring_t;
  /**
   * Get the x coordinate of i-th point in the line
   * @param i point index
   * @return x coordinate
   */
  inline double get_x(int i) const{
    return boost::geometry::get<0>(line.at(i));
  };
  /**
   * Get the y coordinate of i-th point in the line
   * @param i point index starting from 0 to N-1, where N is the
   * number of points in the line
   * @return y coordinate
   */
  inline double get_y(int i) const{
    return boost::geometry::get<1>(line.at(i));
  };
  /**
   * Set x coordinate of i-th point in the line
   * @param i point index
   * @param v the value to update the old coordinate
   */
  inline void set_x(int i, double v){
    boost::geometry::set<0>(line.at(i),v);
  };
  /**
   * Set y coordinate of i-th point in the line
   * @param i point index
   * @param v the value to update the old coordinate
   */
  inline void set_y(int i, double v){
    boost::geometry::set<1>(line.at(i),v);
  };
  /**
   * Add a point to the end of the current line
   * @param x x coordinate of the point to add
   * @param y y coordinate of the point to add
   */
  inline void add_point(double x,double y){
    boost::geometry::append(line, Point(x,y));
  };
  /**
   * Add a point to the end of the current line
   * @param point the point to be added
   */
  inline void add_point(const Point& point){
    boost::geometry::append(line, point);
  };
  /**
   * Get the i-th point in the line
   * @param  i point index starting from 0 to N-1
   * @return The i-th point of the line.
   *
   * Note that the point is a copy of the original point.
   * Manipulating the returned point will not change the
   * original line.
   */
  inline Point get_point(int i) const{
    return Point(boost::geometry::get<0>(
      line.at(i)),boost::geometry::get<1>(line.at(i)));
  };
  /**
   * Get a constance reference of the i-th point in the line
   * @param  i point index
   * @return  A constant reference to the ith point of line, which
   * avoids create a new point.
   */
  inline const Point &at(int i) const{
    return line.at(i);
  }
  /**
   * Get the number of points in a line
   * @return the point number
   */
  inline int get_num_points() const{
    return boost::geometry::num_points(line);
  };
  /**
   * Check if the line is empty or not
   * @return true if the line is empty, otherwise false
   */
  inline bool is_empty(){
    return boost::geometry::num_points(line)==0;
  };
  /**
   * Remove all points in the current line.
   */
  inline void clear(){
    boost::geometry::clear(line);
  };
  /**
   * Get the length of the line
   * @return the length value
   */
  inline double get_length() const {
    return boost::geometry::length(line);
  };
  /**
   * Export a string containing WKT representation of the line.
   * @return The WKT of the line
   *
   * Example: LINESTRING (30 10, 10 30, 40 40)
   */
  inline std::string export_wkt(int precision=8) const{
    std::ostringstream ss;
    ss << std::setprecision(precision) << boost::geometry::wkt(line);
    return ss.str();
  };
  /**
   * Export a string containing GeoJSON representation of the line.
   * @return The GeoJSON of the line
   */
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
  /**
   * Get a const reference to the inner boost geometry linestring
   * @return const reference to the inner boost geometry linestring
   */
  inline const linestring_t &get_geometry_const() const{
    return line;
  };
  /**
   * Get a reference to the inner boost geometry linestring
   * @return a reference to the inner boost geometry linestring
   */
  linestring_t &get_geometry(){
    return line;
  };

  /**
   * Compare if two linestring are the same.
   *
   * It the two lines overlap with each other within a threshold of 1e-6,
   * they are considered equal. This is used in the test class.
   *
   * @param rhs the linestring
   * @return true if the two lines are equal.
   */
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
  /**
   * Overwrite the operator of << of linestring
   * @param os an input stream
   * @param rhs a linestring object
   * @return the wkt representation of the line will be written to the stream.
   */
  friend std::ostream& operator<<(std::ostream& os, const LineString& rhs);
private:
  linestring_t line;
}; // LineString

std::ostream& operator<<(std::ostream& os,const FMM::CORE::LineString& rhs);

/**
 * Convert a OGRLineString to a linestring
 * @param line a pointer to OGRLineString
 * @return a linestring
 */
LineString ogr2linestring(const OGRLineString *line);

/**
 * Convert a OGRMultiLineString to a linestring.
 *
 * If the multilinestring contains multiple lines, only
 * the first linestring will be converted and returned as
 * a result.
 *
 * This function is used in reading data from shapefile.
 *
 * @param mline a pointer to the OGRMultiLineString
 * @return a linestring.
 *
 */
LineString ogr2linestring(const OGRMultiLineString *mline);

/**
 * Convert a wkt into a linestring
 * @param  wkt A wkt representation of a line
 * @return  a linestring
 */
LineString wkt2linestring(const std::string &wkt);

/**
 * Convert a linestring into a OGRLineString
 * @param  line input line
 * @return  A OGRLineString, the caller is responsible for
 * freeing the memory.
 */
OGRLineString *linestring2ogr(const LineString &line);

/**
 * Convert a point into a OGRPoint
 * @param  p input point
 * @return  A OGRPoint, the caller is responsible for
 * freeing the memory.
 */
OGRPoint *point2ogr(const Point &p);

}; // CORE

}; // FMM


#endif // FMM_GEOMTYPES_HPP
