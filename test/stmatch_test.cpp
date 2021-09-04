#include "core/geometry.hpp"
#include "mm/stmatch/stmatch_algorithm.hpp"
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

TEST_CASE("stmatch is tested", "[stmatch]") {
  spdlog::set_level((spdlog::level::level_enum) 0);
  spdlog::set_pattern("[%l][%s:%-3#] %v");

  SECTION("basic matching") {
    Network network("../data/network.gpkg");
    NetworkGraph graph(network);
    STMATCH stmatch(network, graph);
    CSVTrajectoryReader reader("../data/trips.csv","id","geom");
    std::vector<Trajectory> trajectories = reader.read_all_trajectories();
    const Trajectory &traj = trajectories[0];
    STMATCHConfig config(4, 0.4, 0.5);
    MatchResult result = stmatch.match_traj(traj, config);
    LineString expected_mgeom = wkt2linestring(
      "LINESTRING(2 0.250988700565,2 1,2 2,3 2,4 2,4 2.45776836158)");
    REQUIRE_THAT(result.cpath,Catch::Equals<FMM::NETWORK::EdgeID>({2,5,13,14,23}));
    REQUIRE(expected_mgeom == result.mgeom);
  }

  SECTION("bidirectional") {
    Network network("../data/network.gpkg");
    NetworkGraph graph(network);
    STMATCH stmatch(network, graph);
    Trajectory traj(1, wkt2linestring("LINESTRING (1.9 3.5,1.6 3.5,1.5 3.5,1.3 3.5,"
                                      "1.0 3.5,0.8 3.5,0.6 3.5)"));
    MatchResult result =
        stmatch.match_traj(traj, STMATCHConfig(8, 1.0, 1.0, 30, 1.5, 0.5));
    REQUIRE_THAT(result.cpath, Catch::Equals<FMM::NETWORK::EdgeID>({28}));
  }
}
