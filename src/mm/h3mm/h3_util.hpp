#ifndef H3_UTIL_HEADER
#define H3_UTIL_HEADER

#include "h3_header.hpp"
#include "util/debug.hpp"
#include <inttypes.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <iomanip>

namespace FMM{
namespace MM{

std::string hexs2wkt(const std::vector<H3Index> &hexs, int precision=12){
  std::ostringstream oss;
  oss << "MULTIPOLYGON(";
  for (int i = 0 ; i<hexs.size();++i){
    GeoBoundary boundary;
    oss << "((";
    h3ToGeoBoundary(hexs[i], &boundary);
    for (int v = 0; v < boundary.numVerts; v++) {
      oss << std::setprecision(precision) << radsToDegs(boundary.verts[v].lat)
          << " " << radsToDegs(boundary.verts[v].lon) << ",";
    }
    oss << std::setprecision(precision) << radsToDegs(boundary.verts[0].lat)
          << " " << radsToDegs(boundary.verts[0].lon);
    oss << "))" << (i<hexs.size()-1?",":"");
  }
  oss << ")";
  return oss.str();
};

H3Index xy2hex(double px, double py, int level){
  GeoCoord location;
  setGeoDegs(&location, px, py);
  H3Index indexed = geoToH3(&location,level);
  return indexed;
};

std::string hex2wkt(const H3Index &index, int precision=12){
  std::ostringstream oss;
  GeoBoundary boundary;
  oss << "POLYGON(";
  h3ToGeoBoundary(index, &boundary);
  for (int v = 0; v < boundary.numVerts; v++) {
    oss << std::setprecision(precision) << radsToDegs(boundary.verts[v].lat)
        << " " << radsToDegs(boundary.verts[v].lon) << ",";
  }
  oss << std::setprecision(precision) << radsToDegs(boundary.verts[0].lat)
        << " " << radsToDegs(boundary.verts[0].lon);
  oss << ")";
  return oss.str();
};

std::vector<H3Index> hexpath(const H3Index &start,const H3Index &end){
  int line_size = h3LineSize(start,end);
  if (line_size>0){
    std::vector<H3Index> indices(line_size,0);
    if (line_size>0){
      SPDLOG_TRACE("Line size {}",line_size);
      int err = h3Line(start,end,&indices[0]);
    }
    return indices;
  }
  return {};
};

};
};
#endif
