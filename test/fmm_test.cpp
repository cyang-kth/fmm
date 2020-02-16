#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "../src/network.hpp"
#include "../src/ubodt.hpp"
#include "../src/transition_graph.hpp"
#include "../src/gps.hpp"
#include "../src/reader.hpp"

using namespace MM;
using namespace MM::IO;
using namespace std;

TEST_CASE( "fmm is tested", "[fmm]" ) {
  spdlog::set_level((spdlog::level::level_enum) 0);
  spdlog::set_pattern("[%l][%s:%-3#] %v");
  Network network("../data/network.gpkg");
  network.build_rtree_index();
  int multiplier = network.get_node_count();
  TrajectoryCSVReader reader("../data/trips.csv","id","geom");
  std::vector<Trajectory> trajectories = reader.read_all_trajectories();
  SECTION( "ubodt_csv_test" ) {
    const Trajectory &trajectory = trajectories[0];
    UBODT *ubodt = read_ubodt_csv("../data/ubodt.txt",multiplier);
    Traj_Candidates traj_candidates = network.search_tr_cs_knn(
      trajectory.geom,4,0.4,0.5);
    TransitionGraph tg(
      &traj_candidates,trajectory.geom,*ubodt);
    // Optimal path inference
    O_Path o_path = tg.viterbi();
    T_Path t_path = ubodt->construct_traversed_path(o_path,network);
    LineString m_geom = network.complete_path_to_geometry(
      o_path,t_path.cpath);
    LineString expected_mgeom = wkt2linestring(
      "LINESTRING(2 0.250988700565,2 1,2 2,3 2,4 2,4 2.45776836158)");
    REQUIRE_THAT(t_path.cpath,Catch::Equals<int>({2,5,13,14,23}));
    REQUIRE(expected_mgeom==m_geom);
    delete ubodt;
  }
}
