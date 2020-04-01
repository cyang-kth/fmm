#ifndef MM_SRC_MM_FMM_FMM_APP_H_
#define MM_SRC_MM_FMM_FMM_APP_H_

#include "fmm_app_config.hpp"
#include "fmm_algorithm.hpp"

namespace MM {
class FMMApp {
 public:
  FMMApp(const FMMAppConfig &config) :
      config_(config),
      network_(config_.network_config.file,
               config_.network_config.id,
               config_.network_config.source,
               config_.network_config.target),
      ng_(network_),
      ubodt_(UBODT::read_ubodt_file(config_.ubodt_file)){};
  void run() {
    UTIL::TimePoint start_time = std::chrono::steady_clock::now();
    FMM mm_model(network_, ng_, *ubodt_);
    const FMMConfig &fmm_config = config.fmm_config;
    GPSReader reader(config.gps_config);
    OutputConfig result_config = config.get_result_config();
    IO::CSVMatchResultWriter writer(result_config.file,
                                    result_config.output_config);

    // Start map matching
    int progress = 0;
    int points_matched = 0;
    int total_points = 0;
    int step_size = 100;
    if (config.step > 0) step_size = config.step;
    SPDLOG_INFO("Progress report step {}", step_size)
    UTIL::TimePoint corrected_begin = std::chrono::steady_clock::now();
    SPDLOG_INFO("Start to match trajectories")
    // The header is moved to constructor of result writer
    // rw.write_header();
    while (reader->has_next_feature()) {
      if (progress % step_size == 0) {
        SPDLOG_INFO("Progress {}", progress)
      }
      TemporalTrajectory traj = reader->read_next_temporal_trajectory();
      int points_in_tr = traj.geom.get_num_points();
      MM::MatchResult result = mm_model.match_temporal_traj(
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
 private:
  const FMMAppConfig &config_;
  Network network_;
  NetworkGraph ng_;
  std::shared_ptr<UBODT> ubodt_;
};
}

#endif
