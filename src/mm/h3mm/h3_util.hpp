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

namespace FMM
{
namespace MM
{

std::string hexs2wkt(const std::vector<HexIndex> &hexs, int precision = 12)
{
    std::ostringstream oss;
    oss << "MULTIPOLYGON(";
    for (int i = 0; i < hexs.size(); ++i) {
        GeoBoundary boundary;
        oss << "((";
        h3ToGeoBoundary(hexs[i], &boundary);
        for (int v = 0; v < boundary.numVerts; v++) {
            oss << std::setprecision(precision)
                << radsToDegs(boundary.verts[v].lon) << " "
                << radsToDegs(boundary.verts[v].lat) << ",";
        }
        oss << std::setprecision(precision) << radsToDegs(boundary.verts[0].lon)
            << " " << radsToDegs(boundary.verts[0].lat);
        oss << "))" << (i < hexs.size() - 1 ? "," : "");
    }
    oss << ")";
    return oss.str();
};

HexIndex latlng2hex(double lat, double lng, int level)
{
    GeoCoord location;
    setGeoDegs(&location, lat, lng);
    HexIndex indexed = geoToH3(&location, level);
    return (HexIndex)indexed;
};

// The x is lng and y is the lat
HexIndex xy2hex(double x, double y, int level)
{
    return latlng2hex(y, x, level);
};

std::string hex2wkt(const HexIndex &index, int precision = 12)
{
    std::ostringstream oss;
    GeoBoundary boundary;
    oss << "POLYGON(";
    h3ToGeoBoundary(index, &boundary);
    for (int v = 0; v < boundary.numVerts; v++) {
        oss << std::setprecision(precision) << radsToDegs(boundary.verts[v].lon)
            << " " << radsToDegs(boundary.verts[v].lat) << ",";
    }
    oss << std::setprecision(precision) << radsToDegs(boundary.verts[0].lon)
        << " " << radsToDegs(boundary.verts[0].lat);
    oss << ")";
    return oss.str();
};

std::vector<HexIndex> hexpath(const HexIndex &start, const HexIndex &end)
{
    int line_size = h3LineSize(start, end);
    if (line_size > 0) {
        std::vector<H3Index> indices(line_size, 0);
        if (line_size > 0) {
            SPDLOG_TRACE("Line size {}", line_size);
            int err = h3Line(start, end, &indices[0]);
        }
        std::vector<HexIndex> result;
        for (H3Index i : indices) {
            result.push_back(i);
        }
        return result;
    }
    return {};
};

}; // namespace MM
}; // namespace FMM
#endif
