#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "../src/network.hpp"

using namespace MM;
using namespace MM::ALGORITHM;
using namespace std;

TEST_CASE( "Network is tested", "[network]" ) {
  spdlog::set_level((spdlog::level::level_enum) 0);
  spdlog::set_pattern("[%l][%s:%-3#] %v");
  Network network("../data/network.gpkg");
  network.build_rtree_index();

  SECTION( "node edge getter test" ) {
    NodeID nid = 6;
    NodeIndex nidx = network.get_node_index(nid);
    NodeID nid2 = network.get_node_id(nidx);
    REQUIRE(nid == nid2);
    boost_point p = network.get_node_geom_from_idx(nidx);
    REQUIRE(bg::get<0>(p) == 3.0);
    REQUIRE(bg::get<1>(p) == 2.0);
    EdgeID edge_id = 5;
    EdgeIndex e_idx = network.get_edge_index(edge_id);
    EdgeID edge_id_2 = network.get_edge_id(e_idx);
    REQUIRE(edge_id == edge_id_2);
    int nodes = network.get_node_count();
    REQUIRE(nodes==17);
    LineString &line = network.get_edge_geom(edge_id);
    LineString expected = wkt2linestring("LineString(2 1,2 2)");
    REQUIRE(line==expected);
  }

  SECTION("search_point_candidate_edges"){
    double px = 2.1, py = 2.8;
    Point_Candidates pcs = network.search_point_candidate_edges(
      px,py, 3, 1.0);
    Point_Candidates pcs2 = network.search_point_candidate_edges(
      px,py, 3, 0.15);
    REQUIRE(pcs2.size()==2);
  }

  SECTION( "search_tr_cs_knn" ) {
    LineString line = wkt2linestring("LineString(2.1 1.9,2.1 2.8)");
    Traj_Candidates trcs = network.search_tr_cs_knn(line,3,0.15,50);
    REQUIRE(trcs.size()==2);
    REQUIRE(trcs[0].size()==3);
    REQUIRE(trcs[1].size()==2);
    trcs = network.search_tr_cs_knn(line,3,0.05,50);
    REQUIRE(trcs.size()==0);

    line = wkt2linestring("LineString(2.0 1.0,2.1 2.8)");
    trcs = network.search_tr_cs_knn(line,3,0.05,50);
    REQUIRE(trcs.size()==0);
  }
}
