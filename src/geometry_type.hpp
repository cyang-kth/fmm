#ifndef MM_GEOMTYPES_HPP
#define MM_GEOMTYPES_HPP

#ifdef FMM_BST_GEOMETRY

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>

typedef bg::model::point<double, 2, bg::cs::cartesian> point_t;
typedef bg::model::linestring<bg_point_t> linestring_t;

namespace MM {
/**
 *  Boost Geometry Point, compatible with OGRGeometry
 */
class BGPoint {
public:
double getX(int i){
    return;
};
double getX(int i){
    return;
};
double setX(int i){
    return;
};
double setX(int i){
    return;
};
private:
point_t;
};
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
void setX(double i){
    
};
void setY(double i){
    
};
int getNumPoints(){
    return bg::num_points(line);
};
void getPoint(int i,Point *point){
    return;
};
private:
linestring_t line;
};
}; // MM

#endif //FMM_BST_GEOMETRY

#endif // MM_GEOMTYPES_HPP
