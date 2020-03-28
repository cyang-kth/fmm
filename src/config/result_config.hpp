//
// Created by Can Yang on 2020/3/26.
//

#ifndef FMM_SRC_CONFIG_RESULT_CONFIG_HPP_
#define FMM_SRC_CONFIG_RESULT_CONFIG_HPP_

#include<string>

namespace MM{

struct OutputConfig {
  // Original geometry
  bool write_ogeom = false;
  // Optimal path, the edge matched to each point
  bool write_opath = false;
  // The distance from the source node of an edge to the matched point
  bool write_offset = false;
  // The distance from a raw GPS point to a matched GPS point
  bool write_error = false;
  // Complete path, a path traversed by the trajectory
  bool write_cpath = true;
  // Traversed path, the path traversed between
  // each two consecutive observations
  bool write_tpath = false;
  // The geometry of the complete path
  bool write_mgeom = true;
  // The distance travelled between two GPS observations
  bool write_spdist = false;
  // A linestring connecting the point matched for each edge.
  bool write_pgeom = false;
  // The emission probability of each matched point.
  bool write_ep = false;
  // The transition probability of each matched point.
  bool write_tp = false;
  // The length of the matched edge
  bool write_length = false;
};

struct ResultConfig{
  std::string file;
  OutputConfig output_config;
};

}

#endif //FMM_SRC_CONFIG_RESULT_CONFIG_HPP_
