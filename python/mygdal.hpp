#include "gdal/ogrsf_frmts.h" // C++ API for GDAL
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <string>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>

// OGRLineString *geom_cast(OGRGeometry *poGeometry){
//     return dynamic_cast<OGRLineString *>(poGeometry);
// };

void test()
{
    std::string wktstring = "LINESTRING (2 1,2 0)";
    OGRGeometry *poGeometry;
    char* pszWKT = const_cast<char*>(wktstring.c_str());
    OGRGeometryFactory::createFromWkt(&pszWKT, NULL, &poGeometry);
    OGRLineString *p2 = (OGRLineString *) poGeometry;
    std::cout<<"Number of points "<< p2->getNumPoints()<<"\n";
    std::cout<<"Length "<< p2->get_Length()<<"\n";
    char *wkt;
    poGeometry->exportToWkt(&wkt);
    std::cout<<"Geometry in WKT: "<<wkt<<'\n';
    CPLFree(wkt);
    OGRGeometryFactory::destroyGeometry(poGeometry);
    typedef boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian> point_t;
    typedef boost::geometry::model::linestring<point_t> linestring_t;
    std::cout << "Boost example " << std::endl;
    linestring_t ls1;
    boost::geometry::append(ls1, point_t(0.0, 0.0));
    boost::geometry::append(ls1, point_t(1.0, 0.0));
    boost::geometry::append(ls1, point_t(1.0, 2.0));
    double l = boost::geometry::length(ls1);
    std::cout << "Length is "<< l << std::endl;
    std::cout << "Number of points: " << boost::geometry::num_points(ls1) << std::endl;
};
