/**
 * Fast map matching.
 *
 * Definition of MatchResultWriter Class, which contains functions for
 * writing the results.
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef FMM_MM_WRITER_HPP
#define FMM_MM_WRITER_HPP

#include "mm/mm_type.hpp"
#include "network/type.hpp"
#include "util/util.hpp"
#include "util/debug.hpp"
#include "network/network.hpp"
#include "config/result_config.hpp"

#include <iostream>
#include <fstream>
#include <omp.h>

namespace FMM {

namespace IO {

/**
 * An interface defined for writing the map match result
 */
class MatchResultWriter {
public:
  /**
   * Write the match result to a file
   * @param result the match result of a trajectory
   */
  virtual void write_result(
    const FMM::CORE::Trajectory &traj,
    const FMM::MM::MatchResult &result) = 0;
};

/**
 * A writer class for writing matche result to a CSV file.
 */
class CSVMatchResultWriter : public MatchResultWriter {
public:
  /**
   * Constructor
   *
   * The output fields are defined only once and later all the match result
   * will be exported according to that configuration.
   *
   * @param result_file the filename to write result
   * @param config_arg the fields that will be exported
   *
   */
  CSVMatchResultWriter(const std::string &result_file,
                       const CONFIG::OutputConfig &config_arg);
  /**
   * Write a header line for the fields exported
   */
  void write_header();
  /**
   * Write match result
   * @param traj Input trajectory
   * @param result Map match result
   */
  void write_result(const FMM::CORE::Trajectory &traj,
                    const FMM::MM::MatchResult &result);
private:
  std::ofstream m_fstream;
  const CONFIG::OutputConfig &config_;
}; // CSVMatchResultWriter

};     //IO
} //FMM
#endif // FMM_MM_WRITER_HPP
