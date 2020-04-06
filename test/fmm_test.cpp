#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "util/debug.hpp"
#include "network/network.hpp"
#include "mm/fmm/fmm_algorithm.hpp"
#include "mm/transition_graph.hpp"
#include "core/gps.hpp"
#include "io/gps_reader.hpp"

using namespace FMM;
using namespace FMM::IO;
using namespace FMM::CORE;
using namespace FMM::NETWORK;
using namespace FMM::MM;

TEST_CASE( "fmm is tested", "[fmm]" ) {
  spdlog::set_level((spdlog::level::level_enum) 0);
  spdlog::set_pattern("[%l][%s:%-3#] %v");
  Network network("../data/network.gpkg");
  NetworkGraph graph(network);
  int multiplier = network.get_node_count();
  CSVTrajectoryReader reader("../data/trips.csv","id","geom");
  std::vector<Trajectory> trajectories = reader.read_all_trajectories();
  SECTION( "ubodt_csv_test" ) {
    const Trajectory &trajectory = trajectories[0];
    auto ubodt = UBODT::read_ubodt_csv("../data/ubodt.txt",multiplier);
    FastMapMatch model(network,graph,ubodt);
    FastMapMatchConfig config{4,0.4,0.5};
    MatchResult result = model.match_traj(trajectory,config);
    LineString expected_mgeom = wkt2linestring(
      "LINESTRING(2 0.250988700565,2 1,2 2,3 2,4 2,4 2.45776836158)");
    REQUIRE_THAT(result.cpath,Catch::Equals<int>({2,5,13,14,23}));
    REQUIRE(expected_mgeom==result.mgeom);
  }
}
