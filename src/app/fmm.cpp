/**
 * Content
 * FMM application (by stream)
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#include "mm/fmm/fmm.hpp"
#include <iostream>

using namespace std;
using namespace MM;
using namespace MM::IO;

void run(int argc, char **argv)
{

  UTIL::TimePoint start_time = std::chrono::steady_clock::now();

  // Read configurations

  ArgConfig config(argc,argv);

  NetworkConfig network_config = config.get_network_config();
  GPSConfig gps_config = config.get_gps_config();
  ResultConfig result_config = config.get_result_config();
  UBODTConfig ubodt_config = config.get_ubodt_config();

  // Create network data and FMM model, reader and writer

  Network network(network_config.file, network_config.id,
                  network_config.source, network_config.target);
  NetworkGraph graph(network);
  auto ubodt = UBODT::read_file(ubodt_config.file);
  FMM model(network, graph, ubodt);
  MMConfig fmm_config = config.get_mm_config();
  GPSReader reader(config_.gps_file,config_.gps_id,
                   config_.gps_geom,config_.gps_timestamp);
  OutputConfig result_config = config_.get_result_config();
  IO::CSVMatchResultWriter writer(result_config.file,
                                  result_config.output_config);

  // Start map matching

  int progress=0;
  int points_matched=0;
  int total_points=0;
  int step_size = 100;
  if (config_.step>0) step_size = config.step;
  SPDLOG_INFO("Progress report step {}",step_size)
  UTIL::TimePoint corrected_begin = std::chrono::steady_clock::now();
  SPDLOG_INFO("Start to match trajectories")
  // The header is moved to constructor of result writer
  // rw.write_header();
  while (reader->has_next_feature())
  {
    if (progress%step_size==0) {
      SPDLOG_INFO("Progress {}",progress)
    }
    TemporalTrajectory traj = reader->read_next_temporal_trajectory();
    int points_in_tr = traj.geom.get_num_points();
    MM::MatchResult result = mm_model->match_temporal_traj(
      traj, mm_config);
    writer.write_result(result);
    if (!result.cpath.empty()) {
      points_matched+=points_in_tr;
    }
    total_points+=points_in_tr;
    ++progress;
  }
  SPDLOG_INFO("MM process finished")
  UTIL::TimePoint end_time = std::chrono::steady_clock::now();
  double time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(
    end_time - start_time).count()/1000.;
  double time_spent_exclude_input = std::chrono::duration_cast<
    std::chrono::milliseconds>(end_time - corrected_begin).count()/1000.;
  SPDLOG_INFO("Time takes {}",time_spent)
  SPDLOG_INFO("Time takes excluding input {}",time_spent_exclude_input)
  SPDLOG_INFO("Finish map match total points {} matched {}",
              total_points,points_matched)
  SPDLOG_INFO("Matched percentage: {}",points_matched/(double)total_points)
  SPDLOG_INFO("Point match speed: {}",points_matched/time_spent)
  SPDLOG_INFO("Point match speed (excluding input): {}",
              points_matched/time_spent_exclude_input)
  SPDLOG_INFO("Time takes {}",time_spent)
};

int main(int argc, char **argv){
  std::cout<<"------------ Fast map matching (FMM) ------------\n";
  std::cout<<"------------     Author: Can Yang    ------------\n";
  std::cout<<"------------   Version: 2020.01.31   ------------\n";
  std::cout<<"------------     Applicaton: fmm     ------------\n";
  run(argc,argv);
  std::cout<<"------------    Program finished     ------------\n";
  return 0;
};
