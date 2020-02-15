#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "../src/network_graph.hpp"

using namespace MM;
using namespace std;

TEST_CASE( "Network graph is tested", "[network_graph]" ) {
  spdlog::set_level((spdlog::level::level_enum) 0);
  spdlog::set_pattern("[%l][%s:%-3#] %v");
  Network network("../data/network.gpkg");
  NetworkGraph ng(&network);

  SECTION( "single_source_upperbound_dijkstra" ) {

  }

  SECTION( "get_edge_index" ) {
    double px = 1;
    double py = 3;
    double result_dist,result_offset;
    linear_referencing(px,py,line,&result_dist,&result_offset);
    REQUIRE( result_dist == Approx(sqrt(2)));
    REQUIRE( result_offset == 2 );
  }

}
