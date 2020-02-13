#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "../src/algorithm.hpp"

using namespace MM;
using namespace MM::ALGORITHM;

TEST_CASE( "Algorithm are tested", "[algorithm]" ) {
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
    REQUIRE( result_dist == 0 );
    REQUIRE( result_offset == 0 );
  }

  SECTION( "locate_point_by_offset" ) {
    v.reserve( 10 );
    REQUIRE( v.size() == 5 );
    REQUIRE( v.capacity() >= 10 );
  }
  
  SECTION( "cutoffseg" ) {
    v.reserve( 10 );
    REQUIRE( v.size() == 5 );
    REQUIRE( v.capacity() >= 10 );
  }
}
