//
// Created by Can Yang on 2020/4/1.
//

#include "mm/stmatch/stmatch_app.hpp"

using namespace FMM;
using namespace FMM::CORE;
using namespace FMM::NETWORK;
using namespace FMM::MM;

void STMATCHApp::run() {
  auto start_time = UTIL::get_current_time();
  STMATCH mm_model(network_, ng_);
  const STMATCHConfig &stmatch_config =
      config_.stmatch_config;
  IO::GPSReader reader(config_.gps_config);
  IO::CSVMatchResultWriter writer(config_.result_config.file,
                                  config_.result_config.output_config);
  // Start map matching
  int progress = 0;
  int points_matched = 0;
  int total_points = 0;
  int step_size = 100;
  if (config_.step > 0) step_size = config_.step;
  SPDLOG_INFO("Progress report step {}", step_size);
  auto corrected_begin = UTIL::get_current_time();
  SPDLOG_INFO("Start to match trajectories");
  if (config_.use_omp){
    SPDLOG_INFO("Run map matching parallelly");
    int buffer_trajectories_size = 100000;
    while (reader.has_next_trajectory()) {
      std::vector<Trajectory> trajectories =
        reader.read_next_N_trajectories(buffer_trajectories_size);
      int trajectories_fetched = trajectories.size();
      #pragma omp parallel for
      for (int i = 0; i < trajectories_fetched; ++i) {
        Trajectory &trajectory = trajectories[i];
        int points_in_tr = trajectory.geom.get_num_points();
        MM::MatchResult result = mm_model.match_traj(
            trajectory, stmatch_config);
        writer.write_result(trajectory,result);
        #pragma omp critical
        if (!result.cpath.empty()) {
          points_matched += points_in_tr;
        }
        total_points += points_in_tr;
        ++progress;
        if (progress % step_size == 0) {
          std::stringstream buf;
          buf << "Progress " << progress << '\n';
          std::cout << buf.rdbuf();
        }
      }
    }
  } else {
    SPDLOG_INFO("Run map matching in single thread");
    while (reader.has_next_trajectory()) {
      if (progress % step_size == 0) {
        SPDLOG_INFO("Progress {}", progress);
      }
      Trajectory trajectory = reader.read_next_trajectory();
      int points_in_tr = trajectory.geom.get_num_points();
      MM::MatchResult result = mm_model.match_traj(
          trajectory, stmatch_config);
      writer.write_result(trajectory,result);
      if (!result.cpath.empty()) {
        points_matched += points_in_tr;
      }
      total_points += points_in_tr;
      ++progress;
    }
  }
  SPDLOG_INFO("MM process finished");
  auto end_time = UTIL::get_current_time();
  double time_spent = UTIL::get_duration(start_time,end_time);
  double time_spent_exclude_input = UTIL::get_duration(corrected_begin,end_time);
  SPDLOG_INFO("Time takes {}", time_spent);
  SPDLOG_INFO("Time takes excluding input {}", time_spent_exclude_input);
  SPDLOG_INFO("Finish map match total points {} matched {}",
              total_points, points_matched);
  SPDLOG_INFO("Matched percentage: {}", points_matched / (double) total_points);
  SPDLOG_INFO("Point match speed: {}", points_matched / time_spent);
  SPDLOG_INFO("Point match speed (excluding input): {}",
              points_matched / time_spent_exclude_input);
  SPDLOG_INFO("Time takes {}", time_spent);
};
