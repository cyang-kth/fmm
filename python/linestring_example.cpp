#include <iostream>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>

namespace bg = boost::geometry;

int main()
{
    typedef bg::model::point<double, 2, bg::cs::cartesian> point_t;
    typedef bg::model::linestring<point_t> linestring_t;

    linestring_t ls1;

#if !defined(BOOST_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX) \
 && !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
    linestring_t ls2{{0.0, 0.0}, {1.0, 0.0}, {1.0, 2.0}};
#endif

    bg::append(ls1, point_t(0.0, 0.0));
    bg::append(ls1, point_t(1.0, 0.0));
    bg::append(ls1, point_t(1.0, 2.0));
    
    std::cout << boost::geometry::wkt(ls1.at(0)) << std::endl;
    std::cout << "X coordinates at point 0 is "<<bg::get<0>(ls1.at(0)) << std::endl;
    std::cout << "Y coordinates at point 1 is "<<bg::get<1>(ls1.at(1)) << std::endl;
    std::cout << boost::geometry::wkt(ls1[1]) << std::endl;
    std::cout << boost::geometry::wkt(ls1) << std::endl;
    double l = bg::length(ls1);
    std::cout << l << std::endl;
    
    return 0;
}
