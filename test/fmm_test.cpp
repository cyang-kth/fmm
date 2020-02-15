#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "../src/network.hpp"
#include "../src/ubodt.hpp"
#include "../src/transition_graph.hpp"
#include "../src/gps.hpp"
#include "../src/reader.hpp"

using namespace MM;
using namespace std;

TEST_CASE( "Network is tested", "[network]" ) {
  spdlog::set_level((spdlog::level::level_enum) 0);
  spdlog::set_pattern("[%l][%s:%-3#] %v");
  Network network("../data/network.gpkg");

  SECTION( "node edge getter test" ) {

  }

  SECTION( "search_tr_cs_knn" ) {
    double px = 1;
    double py = 3;
    double result_dist,result_offset;
    linear_referencing(px,py,line,&result_dist,&result_offset);
    REQUIRE( result_dist == Approx(sqrt(2)));
    REQUIRE( result_offset == 2 );
  }

  SECTION( "complete_path_to_geometry" ) {
    double px,py;
    locate_point_by_offset(line,2+sqrt(2),&px,&py);
    REQUIRE( px == Approx(1.0) );
    REQUIRE( py == Approx(1.0) );
  }
}
