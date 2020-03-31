/**
 * Content
 * Definition of MatchResultWriter Class, which contains functions for
 * writing the results.
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef MM_MM_WRITER_HPP
#define MM_MM_WRITER_HPP

#include "mm/mm_result.hpp"
#include "network/type.hpp"
#include "util/util.hpp"
#include "util/debug.hpp"
#include "network/network.hpp"
#include "config/result_config.hpp"

#include <iostream>
#include <fstream>

namespace MM {

namespace IO {

class MatchResultWriter {
 public:
  virtual void write_result(const MatchResult &result) = 0;
};

class CSVMatchResultWriter : public MatchResultWriter {
 public:
  /**
   * Constructor
   * @param result_file, the path to an output file
   * @param network_ptr, a pointer to the network
   */
  CSVMatchResultWriter(const std::string &result_file,
                       const OutputConfig &config_arg);
  void write_header();
  void write_result(const MatchResult &result);
 private:
  std::ofstream m_fstream;
  const OutputConfig &config_;
}; // CSVMatchResultWriter

};     //IO
} //MM
#endif // MM_MM_WRITER_HPP
