/**
 * Content
 * FMM application (by stream)
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#include"mm/fmm/fmm.hpp"
#include<iostream>

using namespace std;
using namespace MM;
using namespace MM::IO;

void run(int argc, char **argv)
{

  UTIL::TimePoint start_time = std::chrono::steady_clock::now();
  std::shared_ptr<MMInterface> mm_model;
  switch (config_.algorithm_id) {
    case (MM_ALGOR_TAG::CWRMM):
      SPDLOG_INFO("Create CWRMM model")
      mm_model = std::make_shared<CWRMM>(network_,ng_);
      break;
    case (MM_ALGOR_TAG::FCDMM):
      SPDLOG_INFO("Create FCDMM model")
      mm_model = std::make_shared<FCDMM>(network_,ng_);
      break;
    case (MM_ALGOR_TAG::STMATCH):
      SPDLOG_INFO("Create STMATCH model")
      mm_model = std::make_shared<STMATCH>(network_,ng_);
      break;
    default:
      SPDLOG_CRITICAL("MM Algorithm not found")
      break;
  }
  if (!config_.mm_log_file.empty()){
    SPDLOG_INFO("Write log of mm to file {}", config_.mm_log_file)
    auto mm_logger = spdlog::basic_logger_mt("mm_logger",
                                             config_.mm_log_file,true);
    mm_logger->set_level(static_cast<spdlog::level::level_enum>
                         (config_.mm_log_level));
    //mm_logger->set_pattern("[%^%l%$][%s:%-3#] %v");
    mm_logger->set_pattern("%v");
    mm_model->set_logger(mm_logger);
  }
  MMConfig mm_config = config_.get_mm_config();
  // Create trajectory reader and result writer
  std::shared_ptr<IO::TemporalGPSReader> reader;
  if (config_.gps_point){
    reader = std::make_shared<IO::CSVTemporalPointReader>(
        config_.gps_file,config_.gps_id,
        config_.gps_x,config_.gps_y, config_.gps_timestamp
        );
  } else {
    reader = std::make_shared<IO::CSVTemporalTrajectoryReader>(
        config_.gps_file,config_.gps_id,
        config_.gps_geom,config_.gps_timestamp
    );
  }
  OutputConfig result_config = config_.get_result_config();
  IO::CSVMatchResultWriter writer(config_.result_file,result_config);
  int progress=0;
  int points_matched=0;
  int total_points=0;
  int step_size = 100;
  if (config_.step>0) step_size = config_.step;
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
    if (reader->has_time_stamp()){
      TemporalTrajectory traj = reader->read_next_temporal_trajectory();
      int points_in_tr = traj.geom.get_num_points();
      MM::MatchResult result = mm_model->match_temporal_traj(
          traj, mm_config);
      writer.write_result(result);
      if (!result.cpath.empty()) {
        points_matched+=points_in_tr;
      }
      total_points+=points_in_tr;
    } else {
      Trajectory traj = reader->read_next_trajectory();
      int points_in_tr = traj.geom.get_num_points();
      MM::MatchResult result = mm_model->match_traj(
          traj, mm_config);
      writer.write_result(result);
      if (!result.cpath.empty()) {
        points_matched+=points_in_tr;
      }
      total_points+=points_in_tr;
    }
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
