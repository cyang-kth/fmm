#ifdef __cplusplus
extern "C" {
#endif

#include <h3api.h>
#include <geoCoord.h>

#ifdef __cplusplus
}
#endif

#include <inttypes.h>
#include <stdio.h>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>

std::string hexs2wkt(const std::vector<H3Index> &hexs){
  std::ostringstream oss;
  oss << "MULTIPOLYGON(";
  for (int i = 0 ;i < hexs.size();++i){
    GeoBoundary boundary;
    oss<<"(";
    h3ToGeoBoundary(hexs[i], &boundary);
    for (int v = 0; v < boundary.numVerts; v++) {
      oss << radsToDegs(boundary.verts[v].lat) << " " <<
        radsToDegs(boundary.verts[v].lon)<<",";
    }
    oss<<"),";
  }
  oss << ")";
  return oss.str();
};
std::string hex2wkt(H3Index &index){
  std::ostringstream oss;
  GeoBoundary boundary;
  oss << "POLYGON(";
  h3ToGeoBoundary(index, &boundary);
  for (int v = 0; v < boundary.numVerts; v++) {
    oss << std::setprecision(12) << radsToDegs(boundary.verts[v].lat) << " " <<
      radsToDegs(boundary.verts[v].lon) << ",";
  }
  oss << std::setprecision(12) << radsToDegs(boundary.verts[0].lat) << " " <<
    radsToDegs(boundary.verts[0].lon);
  oss << ")";
  return oss.str();
};

H3Index xy2hex(double px, double py, int level){
  GeoCoord location;
  setGeoDegs(&location, px, py);
  H3Index indexed = geoToH3(&location,level);
  return indexed;
};

int main(int argc, char *argv[]) {
    // Get the H3 index of some location and print it.
    GeoCoord location;
    location.lat = degsToRads(40.689167);
    location.lon = degsToRads(-74.044444);
    int resolution = 10;
    H3Index indexed = geoToH3(&location, resolution);
    printf("The index is: %" PRIx64 "\n", indexed);

    // Get the vertices of the H3 index.
    GeoBoundary boundary;
    h3ToGeoBoundary(indexed, &boundary);
    // Indexes can have different number of vertices under some cases,
    // which is why boundary.numVerts is needed.
    for (int v = 0; v < boundary.numVerts; v++) {
        printf("Boundary vertex #%d: %lf, %lf\n", v,
               radsToDegs(boundary.verts[v].lat),
               radsToDegs(boundary.verts[v].lon));
    }

    // Get the center coordinates.
    GeoCoord center;
    h3ToGeo(indexed, &center);
    printf("Center coordinates: %lf, %lf\n", radsToDegs(center.lat),
           radsToDegs(center.lon));
    double px = 18.07918538750904;
    double py =  59.324335716328385;
    auto index = xy2hex(px,py,15);
    std::cout<<"hex "<< index << "\n";
    std::cout<<"Polygon "<< hex2wkt(index) << "\n";
}
