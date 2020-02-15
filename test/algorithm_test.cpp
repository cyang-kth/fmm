#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "../src/algorithm.hpp"

using namespace MM;
using namespace MM::ALGORITHM;
using namespace std;

TEST_CASE( "Algorithm are tested", "[algorithm]" ) {
  spdlog::set_level((spdlog::level::level_enum) 0);
  spdlog::set_pattern("[%l][%s:%-3#] %v");
  std::string wkt_geom = "LineString(0 0,0 1,0 2,1 1,2 1,2 0)";
  LineString line = wkt2linestring(wkt_geom);

  SECTION( "approximate_equal" ) {
    double x1,y1,x2,y2;
    boundingbox_geometry(line,&x1,&y1,&x2,&y2);
    REQUIRE( x1 == 0 );
    REQUIRE( y1 == 0 );
    REQUIRE( x2 == 2 );
    REQUIRE( y2 == 2 );
  }

  SECTION( "boundingbox_geometry" ) {
    double x1,y1,x2,y2;
    boundingbox_geometry(line,&x1,&y1,&x2,&y2);
    REQUIRE( x1 == 0 );
    REQUIRE( y1 == 0 );
    REQUIRE( x2 == 2 );
    REQUIRE( y2 == 2 );
  }

  SECTION( "linear_referencing" ) {
    double px = 1;
    double py = 3;
    double result_dist,result_offset;
    linear_referencing(px,py,line,&result_dist,&result_offset);
    REQUIRE( result_dist == Approx(sqrt(2)));
    REQUIRE( result_offset == 2 );
  }

  SECTION( "locate_point_by_offset" ) {
    double px,py;
    locate_point_by_offset(line,2+sqrt(2),&px,&py);
    REQUIRE( px == Approx(1.0) );
    REQUIRE( py == Approx(1.0) );
  }

  SECTION( "cutoffseg_unique" ) {
    // Two vertices
    LineString result_1 = cutoffseg_unique(1,2+sqrt(2),line);
    LineString expected_1 = wkt2linestring("LineString(0 1,0 2,1 1)");
    REQUIRE(expected_1 == result_1);
    // One vertex and one midpoint
    LineString result_2 = cutoffseg_unique(0,2+sqrt(2)+0.1,line);
    LineString expected_2 = wkt2linestring("LineString(0 0,0 1,0 2,1 1,1.1 1)");
    REQUIRE(expected_2 == result_2);
    // Two midpoints
    LineString result_3 = cutoffseg_unique(2+sqrt(2)/2,2.5+sqrt(2),line);
    LineString expected_3 = wkt2linestring("LineString(0.5 1.5,1 1,1.5 1)");
    REQUIRE(expected_3 == result_3);

    // One midpoint to vertex
    LineString result_4 = cutoffseg_unique(2+sqrt(2)/2,3+sqrt(2),line);
    LineString expected_4 = wkt2linestring("LineString(0.5 1.5,1 1,2 1)");
    REQUIRE(expected_4 == result_4);

    // One midpoint to end
    LineString result_5 = cutoffseg_unique(2+sqrt(2)/2,4+sqrt(2),line);
    LineString expected_5 = wkt2linestring("LineString(0.5 1.5,1 1,2 1,2 0)");
    REQUIRE(expected_5 == result_5);
  }

  SECTION( "cutoffseg" ) {
    // Mode 0, export p to end
    // From start
    int mode = 0;
    LineString result_1 = cutoffseg(0,line,mode);
    LineString expected_1 = wkt2linestring(
      "LineString(0 0,0 1,0 2,1 1,2 1,2 0)");
    REQUIRE(expected_1 == result_1);
    // From start
    LineString result_2 = cutoffseg(1,line,mode);
    LineString expected_2 = wkt2linestring(
      "LineString(0 1,0 2,1 1,2 1,2 0)");
    REQUIRE(expected_2 == result_2);
    // From a midpoint
    LineString result_3 = cutoffseg(0.5,line,mode);
    LineString expected_3 = wkt2linestring(
      "LineString(0 0.5,0 1,0 2,1 1,2 1,2 0)");
    REQUIRE(expected_3 == result_3);

    // Mode 1, export start to p
    mode = 1;
    // Test for the end node
    LineString result_4 = cutoffseg(line.getLength(),line,mode);
    LineString expected_4 = wkt2linestring(
      "LineString(0 0,0 1,0 2,1 1,2 1,2 0)");
    REQUIRE(expected_4 == result_4);

    // Test for a vertex
    LineString result_5 = cutoffseg(line.getLength()-1,line,mode);
    LineString expected_5 = wkt2linestring(
      "LineString(0 0,0 1,0 2,1 1,2 1)");
    REQUIRE(expected_5 == result_5);

    // Test for a  midpoint
    LineString result_6 = cutoffseg(line.getLength()-0.5,line,mode);
    LineString expected_6 = wkt2linestring(
      "LineString(0 0,0 1,0 2,1 1,2 1,2 0.5)");
    REQUIRE(expected_6 == result_6);
  }
}
