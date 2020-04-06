//
// Created by Can Yang on 2020/4/1.
//

#include "mm/fmm/ubodt_gen_app.hpp"
#include "mm/fmm/ubodt.hpp"
#include "network/network.hpp"
#include "network/network_graph.hpp"
#include <boost/archive/binary_oarchive.hpp>
#include "util/debug.hpp"
#include <omp.h>

using namespace FMM;
using namespace FMM::CORE;
using namespace FMM::NETWORK;
using namespace FMM::MM;
void UBODTGenApp::run() const {
  if (!config_.validate()) {
    SPDLOG_CRITICAL("Validation fail, program stop");
    return;
  };
  config_.print();
  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();
  SPDLOG_INFO("Write UBODT to file {}", config_.result_file);
  bool binary = config_.is_binary_output();
  if (config_.use_omp){
    precompute_ubodt_omp(config_.result_file, config_.delta, binary);
  } else {
    precompute_ubodt(config_.result_file, config_.delta, binary);
  }
  std::chrono::steady_clock::time_point end =
      std::chrono::steady_clock::now();
  double time_spent =
      std::chrono::duration_cast<std::chrono::milliseconds>
          (end - begin).count() / 1000.;
  SPDLOG_INFO("Time takes {}", time_spent);
}

void UBODTGenApp::precompute_ubodt(
    const std::string &filename, double delta, bool binary) const {
  int num_vertices = graph_.get_num_vertices();
  int step_size = num_vertices / 10;
  if (step_size < 10) step_size = 10;
  std::ofstream myfile(filename);
  SPDLOG_INFO("Start to generate UBODT with delta {}", delta);
  SPDLOG_INFO("Output format {}", (binary ? "binary" : "csv"));
  if (binary) {
    boost::archive::binary_oarchive oa(myfile);
    for (NodeIndex source = 0; source < num_vertices; ++source) {
      if (source % step_size == 0)
        SPDLOG_INFO("Progress {} / {}", source, num_vertices);
      PredecessorMap pmap;
      DistanceMap dmap;
      graph_.single_source_upperbound_dijkstra(source, delta, &pmap, &dmap);
      write_result_binary(oa, source, pmap, dmap);
    }
  } else {
    myfile << "source;target;next_n;prev_n;next_e;distance\n";
    for (NodeIndex source = 0; source < num_vertices; ++source) {
      if (source % step_size == 0)
        SPDLOG_INFO("Progress {} / {}", source, num_vertices);
      PredecessorMap pmap;
      DistanceMap dmap;
      graph_.single_source_upperbound_dijkstra(source, delta, &pmap, &dmap);
      write_result_csv(myfile, source, pmap, dmap);
    }
  }
  myfile.close();
}

// Parallelly generate ubodt using OpenMP
void UBODTGenApp::precompute_ubodt_omp(
    const std::string &filename, double delta,
    bool binary) const {
  int num_vertices = graph_.get_num_vertices();
  int step_size = num_vertices / 10;
  if (step_size < 10) step_size = 10;
  std::ofstream myfile(filename);
  SPDLOG_INFO("Start to generate UBODT with delta {}", delta);
  SPDLOG_INFO("Output format {}", (binary ? "binary" : "csv"));
  if (binary) {
    boost::archive::binary_oarchive oa(myfile);
    int progress = 0;
#pragma omp parallel
    {
#pragma omp for
      for (int source = 0; source < num_vertices; ++source) {
        ++progress;
        if (progress % step_size == 0) {
          SPDLOG_INFO("Progress {} / {}", progress, num_vertices);
        }
        PredecessorMap pmap;
        DistanceMap dmap;
        std::stringstream node_output_buf;
        graph_.single_source_upperbound_dijkstra(source, delta, &pmap, &dmap);
        write_result_binary(oa, source, pmap, dmap);
      }
    }
  } else {
    myfile << "source;target;next_n;prev_n;next_e;distance\n";
    int progress = 0;
#pragma omp parallel
    {
#pragma omp for
      for (int source = 0; source < num_vertices; ++source) {
        ++progress;
        if (progress % step_size == 0) {
          SPDLOG_INFO("Progress {} / {}", progress, num_vertices);
        }
        PredecessorMap pmap;
        DistanceMap dmap;
        std::stringstream node_output_buf;
        graph_.single_source_upperbound_dijkstra(source, delta, &pmap, &dmap);
        write_result_csv(myfile, source, pmap, dmap);
      }
    }
  }
  myfile.close();
}

/**
   * Write the result of routing from a single source node
   * @param stream output stream
   * @param s      source node
   * @param pmap   predecessor map
   * @param dmap   distance map
   */
void UBODTGenApp::write_result_csv(
    std::ostream &stream, NodeIndex s,
                                   PredecessorMap &pmap, DistanceMap &dmap) const {
  std::vector<Record> source_map;
  for (auto iter = pmap.begin(); iter != pmap.end(); ++iter) {
    NodeIndex cur_node = iter->first;
    if (cur_node != s) {
      NodeIndex prev_node = iter->second;
      NodeIndex v = cur_node;
      NodeIndex u;
      while ((u = pmap[v]) != s) {
        v = u;
      }
      NodeIndex successor = v;
      // Write the result to source map
      double cost = dmap[successor];
      EdgeIndex edge_index = graph_.get_edge_index(s, successor, cost);
      source_map.push_back(
          {s,
           cur_node,
           successor,
           prev_node,
           edge_index,
           dmap[cur_node],
           nullptr});
    }
  }
#pragma omp critical
  for (Record &r:source_map) {
    stream << r.source << ";"
           << r.target << ";"
           << r.first_n << ";"
           << r.prev_n << ";"
           << r.next_e << ";"
           << r.cost << "\n";
  }
}

/**
 * Write the result of routing from a single source node in
 * binary format
 *
 * @param stream output stream
 * @param s      source node
 * @param pmap   predecessor map
 * @param dmap   distance map
 */
void UBODTGenApp::write_result_binary(boost::archive::binary_oarchive &stream,
                                      NodeIndex s,
                                      PredecessorMap &pmap,
                                      DistanceMap &dmap) const {
  std::vector<Record> source_map;
  for (auto iter = pmap.begin(); iter != pmap.end(); ++iter) {
    NodeIndex cur_node = iter->first;
    if (cur_node != s) {
      NodeIndex prev_node = iter->second;
      NodeIndex v = cur_node;
      NodeIndex u;
      // When u=s, v is the next node visited
      while ((u = pmap[v]) != s) {
        v = u;
      }
      NodeIndex successor = v;
      // Write the result
      double cost = dmap[successor];
      EdgeIndex edge_index = graph_.get_edge_index(s, successor, cost);
      source_map.push_back(
          {s,
           cur_node,
           successor,
           prev_node,
           edge_index,
           dmap[cur_node],
           nullptr});
    }
  }
#pragma omp critical
  for (Record &r:source_map) {
    stream << r.source << r.target
           << r.first_n << r.prev_n << r.next_e << r.cost;
  }
}
