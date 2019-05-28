#include "gdal/ogrsf_frmts.h" // C++ API for GDAL
#include <iostream>
#include <stdio.h>

void test()
{
    char *wkt = (char*) "LINESTRING (2 1, 2 0, 3 0.5)";
    OGRGeometry* cutoffline;
    OGRGeometryFactory::createFromWkt(&wkt,NULL,&cutoffline);
    OGRLineString *line = (OGRLineString *)cutoffline;
    std::cout<<"Number of points "<< line->getNumPoints()<<"\n";
    std::cout<<"Length "<< line->get_Length()<<"\n";
    OGRGeometryFactory::destroyGeometry(cutoffline);
    std::cout<<"Test executed"<<std::endl;
};
