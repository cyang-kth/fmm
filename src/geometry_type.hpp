#ifndef MM_GEOMTYPES_HPP
#define MM_GEOMTYPES_HPP

#ifdef FMM_BST_GEOMETRY

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>

typedef bg::model::point<double, 2, bg::cs::cartesian> point_t;
typedef bg::model::linestring<bg_point_t> linestring_t;

namespace MM {
/**
 *  Boost Geometry Linestring, compatible with OGRGeometry
 */
class BGLinestring {
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
};
}; // MM

#endif //FMM_BST_GEOMETRY

#endif // MM_GEOMTYPES_HPP
