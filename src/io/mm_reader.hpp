/**
 * Content
 * Definition of MatchResultWriter Class, which contains functions for
 * writing the results.
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef MM_MM_READER_HPP
#define MM_MM_READER_HPP

#include <iostream>
#include <fstream>

#include "mm/mm_interface.hpp"
#include "network/type.hpp"
#include "util/util.hpp"
#include "util/debug.hpp"
#include "network/network.hpp"

namespace MM
{

namespace IO {

class MatchResultReader {
public:
  virtual bool has_next_feature() = 0;
  virtual void close() = 0;
  virtual MatchResult read_next_trajectory() = 0;
};

class GDALMatchResultReader : public MatchResultReader {
public:
  GDALMatchResultReader(const std::string & filename,
                        const std::string & id_name="id",
                        const std::string & opath_name="opath",
                        const std::string & cpath_name="cpath");
  int get_num_trajectories();
  bool has_next_feature();
  void close();
  MatchResult read_next_trajectory();
private:
  int NUM_FEATURES=0;
  int id_idx = -1;
  int opath_idx = -1;
  int cpath_idx = -1;
  // Keep record of current features read
  int _cursor = 0;
  GDALDataset *poDS;
  OGRLayer  *ogrlayer;
}; // GDALMatchResultWriter

};     //IO
} //MM
#endif // MM_MM_READER_HPP
