//
// Created by Can Yang on 2020/4/1.
//

#include "mm/fmm/fmm_app.hpp"
#include "io/gps_reader.hpp"
#include "io/mm_writer.hpp"

namespace MM{

void FMMApp::run() {
  UTIL::TimePoint start_time = std::chrono::steady_clock::now();
  FMM mm_model(network_, ng_, *ubodt_);
  const FMMConfig &fmm_config = config_.fmm_config;
  IO::GPSReader reader(config_.gps_config);
  IO::CSVMatchResultWriter writer(config_.result_config.file,
                                  config_.result_config.output_config);

  // Start map matching
  int progress = 0;
  int points_matched = 0;
  int total_points = 0;
  int step_size = 100;
  if (config_.step > 0) step_size = config_.step;
  SPDLOG_INFO("Progress report step {}", step_size)
  UTIL::TimePoint corrected_begin = std::chrono::steady_clock::now();
  SPDLOG_INFO("Start to match trajectories")
  // The header is moved to constructor of result writer
  // rw.write_header();
  while (reader.has_next_feature()) {
    if (progress % step_size == 0) {
      SPDLOG_INFO("Progress {}", progress)
    }
    Trajectory traj = reader.read_next_trajectory();
    int points_in_tr = traj.geom.get_num_points();
    MM::MatchResult result = mm_model.match_traj(
        traj, fmm_config);
    writer.write_result(result);
    if (!result.cpath.empty()) {
      points_matched += points_in_tr;
    }
    total_points += points_in_tr;
    ++progress;
  }
  SPDLOG_INFO("MM process finished")
  UTIL::TimePoint end_time = std::chrono::steady_clock::now();
  double time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time).count() / 1000.;
  double time_spent_exclude_input = std::chrono::duration_cast<
      std::chrono::milliseconds>(end_time - corrected_begin).count() / 1000.;
  SPDLOG_INFO("Time takes {}", time_spent)
  SPDLOG_INFO("Time takes excluding input {}", time_spent_exclude_input)
  SPDLOG_INFO("Finish map match total points {} matched {}",
              total_points, points_matched)
  SPDLOG_INFO("Matched percentage: {}", points_matched / (double) total_points)
  SPDLOG_INFO("Point match speed: {}", points_matched / time_spent)
  SPDLOG_INFO("Point match speed (excluding input): {}",
              points_matched / time_spent_exclude_input)
  SPDLOG_INFO("Time takes {}", time_spent)
};

}