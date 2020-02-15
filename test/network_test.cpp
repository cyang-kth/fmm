#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "../src/network.hpp"

using namespace MM;
using namespace std;

TEST_CASE( "Network is tested", "[network]" ) {
  spdlog::set_level((spdlog::level::level_enum) 0);
  spdlog::set_pattern("[%l][%s:%-3#] %v");
  Network network("../data/network.gpkg");

  SECTION( "node edge getter test" ) {

  }

  SECTION( "search_tr_cs_knn" ) {

  }

  SECTION( "complete_path_to_geometry" ) {

  }

}
