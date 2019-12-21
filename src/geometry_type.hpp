#ifndef MM_GEOMTYPES_HPP
#define MM_GEOMTYPES_HPP

#ifdef USE_BG_GEOMETRY
#include <ogrsf_frmts.h> // C++ API for GDAL
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/extensions/gis/io/wkb/read_wkb.hpp>
#include <iterator>
#include <vector>
#else
#include <ogrsf_frmts.h> // C++ API for GDAL
#endif

namespace MM {

#ifdef USE_BG_GEOMETRY

namespace bg = boost::geometry;
typedef bg::model::point<double, 2, bg::cs::cartesian> bg_point_t;
typedef bg::model::linestring<bg_point_t> linestring_t;
/**
 *  Boost Geometry Linestring, compatible with OGRGeometry
 */
class BGLineString {
public:
inline double getX(int i){
    return bg::get<0>(line.at(i));
};
inline double getY(int i){
    return bg::get<1>(line.at(i));
};
inline void addPoint(double x,double y){
    bg::append(line, bg_point_t(x,y));
};
inline int getNumPoints(){
    return bg::num_points(line);
};
inline bool IsEmpty(){
    return bg::num_points(line)==0;
};
bg::wkt_manipulator<linestring_t> exportToWkt(){
    return bg::wkt(line);
};
linestring_t *get_geometry(){
    return &line;
};
inline double get_Length(){
    return bg::length(line);
};
private:
linestring_t line;
}; // BGLineString

typedef BGLineString LineString;

/**
 *  Convert an OGRLineString to Boost geometry, the caller is responsible to
 *  freeing the memory.
 *
 */
BGLineString *ogr2bg(OGRLineString *line){
    int binary_size = line->WkbSize();
    std::vector<unsigned char> wkb(binary_size);
    // http://www.gdal.org/ogr__core_8h.html#a36cc1f4d807ba8f6fb8951f3adf251e2
    line->exportToWkb(wkbNDR,&wkb[0]);
    BGLineString *l = new BGLineString();
    bg::read_wkb(wkb.begin(),wkb.end(),*(l->get_geometry()));
    return l;
};

#else

// Read WKT into a OGRLineString, the caller should take care of
// freeing the memory.

OGRLineString *read_wkt(const std::string& wkt){
  OGRGeometry *poGeometry;
	//char* pszWKT = const_cast<char*>(wkt.c_str());
	OGRErr tag = OGRGeometryFactory::createFromWkt(wkt.c_str(), NULL, &poGeometry);
  return (OGRLineString*) poGeometry;
};

typedef OGRLineString LineString;

#endif //USE_BG_GEOMETRY

}; // MM


#endif // MM_GEOMTYPES_HPP
