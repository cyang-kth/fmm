#ifndef MM_GEOMTYPES_HPP
#define MM_GEOMTYPES_HPP

#ifdef USE_BG_GEOMETRY
#include "gdal/ogrsf_frmts.h" // C++ API for GDAL
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/extensions/gis/io/wkb/read_wkb.hpp>
#else
#include "gdal/ogrsf_frmts.h" // C++ API for GDAL
#endif

namespace MM {
    
#ifdef USE_BG_GEOMETRY

namespace bg = boost::geometry;
typedef bg::model::point<double, 2, bg::cs::cartesian> point_t;
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
    bg::append(line, point_t(x,y));
};
int getNumPoints(){
    return bg::num_points(line);
};
private:
linestring_t line;
}; // BGLineString

typedef BGLineString LineString;

BGLineString *ogr2bg(OGRLineString *line){
    int binary_size = line->WkbSize();
    unsigned char wkb[binary_size];
    // http://www.gdal.org/ogr__core_8h.html#a36cc1f4d807ba8f6fb8951f3adf251e2
    line->exportToWkb(wkbNDR,wkb);
    BGLineString l;
    bg::read_wkb();
    bg::read_wkb(wkb.begin(), wkb.end(),l)
    // http://www.gdal.org/cpl__string_8h.html
    //char *hex_wkb = CPLBinaryToHex(binary_size,wkb);
    // m_fstream<<hex_wkb<<std::endl;
    // CPLFree(hex_wkb);
    // line->exporttowkb();
};

#else

typedef OGRLineString LineString;

#endif //USE_BG_GEOMETRY

}; // MM


#endif // MM_GEOMTYPES_HPP
