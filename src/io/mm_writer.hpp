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

#include "mm/mm_interface.hpp"
#include "network/type.hpp"
#include "util/util.hpp"
#include "util/debug.hpp"
#include "network/network.hpp"

#include <iostream>
#include <fstream>

namespace MM
{

namespace IO {

class MatchResultWriter {
public:
  virtual void write_result(int id, const O_Path &opath,
                            const C_Path &cpath, const LineString &geom) = 0;
  inline void write_result(const MatchResult &result){
    write_result(result.id, result.opath, result.cpath,result.mgeom);
  };
};

class CSVMatchResultWriter : public MatchResultWriter
{
public:
  using MatchResultWriter::write_result;
  /**
   * Constructor
   * @param result_file, the path to an output file
   * @param network_ptr, a pointer to the network
   */
  CSVMatchResultWriter(const std::string &result_file, OutputConfig &config_arg);

  void write_result(int id, const O_Path &opath,
                    const C_Path &cpath, const LineString &geom);

  void write_header();

  static void write_geometry(std::stringstream &buf, const LineString &line);

  static std::string match_result2geojson(const MatchResult &result);

private:
  std::ofstream m_fstream;
  OutputConfig config;
}; // CSVMatchResultWriter

class GDALMatchResultWriter : public MatchResultWriter
{
public:
  using MatchResultWriter::write_result;
  /**
   * Constructor
   * @param result_file, the path to an output file
   * @param network_ptr, a pointer to the network
   */
  GDALMatchResultWriter(const std::string &result_file, OutputConfig &config_arg);

  void write_result(int id, O_Path &opath, C_Path &cpath,LineString &geom);

private:
  GDALDataset *poDS;
  OGRLayer *poLayer;
  int id_idx = -1;
  int opath_idx = -1;
  int cpath_idx = -1;
  OutputConfig config;
}; // CSVMatchResultWriter

};     //IO
} //MM
#endif // MM_MM_WRITER_HPP
