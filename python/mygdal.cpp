#include "gdal/ogrsf_frmts.h" // C++ API for GDAL
#include <iostream>
#include <stdio.h>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/extensions/gis/io/wkb/read_wkb.hpp>
#include <iterator>
#include <vector>

namespace bg = boost::geometry;
typedef bg::model::point<double, 2, bg::cs::cartesian> bg_point_t;
typedef bg::model::linestring<bg_point_t> linestring_t;

int main()
{
    char *wkt = (char*)  "LINESTRING (2 1, 2 0, 3 0.5)";
    OGRGeometry* cutoffline;
    OGRGeometryFactory::createFromWkt(&wkt,NULL,&cutoffline);
    OGRLineString *line = (OGRLineString *)cutoffline;
    std::cout<<"Number of points "<< line->getNumPoints()<<"\n";
    std::cout<<"Length "<< line->get_Length()<<"\n";
    std::cout<<"Test executed"<<std::endl;
    int binary_size = line->WkbSize();
    std::vector<unsigned char> wkb(binary_size);
    std::cout<<"WKB size "<< binary_size <<std::endl;
    //unsigned char wkb[binary_size];
    // http://www.gdal.org/ogr__core_8h.html#a36cc1f4d807ba8f6fb8951f3adf251e2
    line->exportToWkb(wkbNDR,&wkb[0]);
    std::cout<<"Finish export wkb"<<std::endl;
    linestring_t l;
    bg::read_wkb(wkb.begin(),wkb.end(),l);
    std::cout<<"Finish conversion"<<std::endl;
    std::cout<< "Boost geometry wkt is " << bg::wkt(l)<<std::endl;
    OGRGeometryFactory::destroyGeometry(cutoffline);
    return 0;
};
