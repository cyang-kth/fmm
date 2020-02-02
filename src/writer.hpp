/**
 * Content
 * Definition of ResultWriter Class, which contains functions for
 * writing the results.
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */

#ifndef MM_WRITER_HPP
#define MM_WRITER_HPP
#include <iostream>
#include <fstream>

#include "types.hpp"
#include "debug.h"
#include "config.hpp"
#include "network.hpp"

namespace MM
{
namespace IO {

/**
 *  The result writer is constructed with an output file name
 *  and a network pointer.
 *
 *  The network pointer is required because the complete path is actually
 *  a vector of indexes of the edges in the SHP file, which needs to be
 *  converted into a vector of IDs.
 */
class ResultWriter
{
public:
  /**
   * Constructor
   * @param result_file, the path to an output file
   * @param network, a pointer to the network
   */
  ResultWriter(const std::string &result_file,
               Network &network_arg, ResultConfig &config_arg) :
    fstream(result_file),
    network(network_arg),
    config(config_arg)
  {
    SPDLOG_INFO("Write result to file {}",result_file);
    write_header();
  };

  /**
   *  Write map matching result for a trajectory
   * @param tr_id: id of trajectory
   * @param ogeom: original geometry of the trajectory OGRLineString*
   * @param o_path: pointer to the optimal path (sequence of candidates)
   * @param c_path: pointer to the complete path (sequence of edge ids)
   * @param mgeom: the geometry of the matched path (untraversed part at the
   * first and last edge are removed.)
   */
  void write_result(int tr_id, const LineString &ogeom, const O_Path &o_path,
                    const T_Path &t_path, LineString &mgeom){
    std::stringstream buf;
    buf << tr_id;
    if (config.write_ogeom) {
      buf << ";";
      write_geometry(buf,ogeom);
    }
    if (config.write_opath) {
      buf << ";";
      write_o_path(buf,o_path);
    }
    if (config.write_error) {
      buf << ";";
      write_gps_error(buf,o_path);
    }
    if (config.write_offset) {
      buf << ";";
      write_offset(buf,o_path);
    }
    // Distance traversed which could be more complicated.
    if (config.write_spdist) {
      buf << ";";
      write_spdist(buf,o_path);
    }
    if (config.write_pgeom) {
      buf << ";";
      write_pgeom(buf,o_path);
    }
    // Write fields related with cpath
    if (config.write_cpath) {
      buf << ";";
      write_cpath(buf,t_path);
    }
    if (config.write_tpath) {
      buf << ";";
      write_tpath(buf,t_path);
    }
    if (config.write_mgeom) {
      buf << ";";
      write_geometry(buf,mgeom);
    }
    if (config.write_ep) {
      buf << ";";
      write_ep(buf,o_path);
    }
    if (config.write_tp) {
      buf << ";";
      write_tp(buf,o_path);
    }
    buf << '\n';
    // Ensure that fstream is called corrected in OpenMP
    #pragma omp critical
    fstream << buf.rdbuf();
  };

  static std::string mkString(
    const Network &network, const O_Path &o_path, const T_Path &t_path,
    const LineString &mgeom, bool return_details = false){
    std::stringstream buf;
    if (return_details) {
      write_o_path(buf,o_path);
      buf << ";";
      write_gps_error(buf,o_path);
      buf << ";";
      write_offset(buf,o_path);
      buf << ";";
      write_spdist(buf,o_path);
      buf << ";";
      write_pgeom(buf,o_path);
      buf << ";";
      write_cpath_network(buf,t_path,network);
      buf << ";";
      write_tpath_network(buf,t_path,network);
      buf << ";";
    }
    if (!mgeom.isEmpty()) {
      write_geometry(buf,mgeom);
    }
    return buf.str();
  };
  void write_header() {
    std::string header = "id";
    if (config.write_ogeom) header+=";ogeom";
    if (config.write_opath) header+=";opath";
    if (config.write_error) header+=";error";
    if (config.write_offset) header+=";offset";
    if (config.write_spdist) header+=";spdist";
    if (config.write_pgeom) header+=";pgeom";
    if (config.write_cpath) header+=";cpath";
    if (config.write_tpath) header+=";tpath";
    if (config.write_mgeom) header+=";mgeom";
    if (config.write_ep) header+=";ep";
    if (config.write_tp) header+=";tp";
    fstream << header << '\n';
  };
  static void write_geometry(std::stringstream &buf, const LineString &line){
    if (!line.isEmpty()) {
      buf << std::setprecision(12) << line.exportToWkt();
    }
  };
  // Write the optimal path
  static void write_o_path(std::stringstream &buf,const O_Path &o_path)
  {
    if (o_path.empty()) {
      return;
    };
    int N = o_path.size();
    for (int i = 0; i < N - 1; ++i)
    {
      buf << o_path[i]->edge->id << ",";
    }
    buf << o_path[N - 1]->edge->id;
  };

  static void write_offset(std::stringstream &buf,const O_Path &o_path)
  {
    if (o_path.empty()) {
      return;
    };
    int N = o_path.size();
    for (int i = 0; i < N - 1; ++i)
    {
      buf << o_path[i]->offset<< ",";
    }
    buf << o_path[N - 1]->offset;
  };

  static void write_spdist(std::stringstream &buf,const O_Path &o_path)
  {
    if (o_path.empty()) {
      return;
    };
    int N = o_path.size();
    for (int i = 0; i < N - 1; ++i)
    {
      buf << o_path[i]->sp_dist<< ",";
    }
    buf << o_path[N - 1]->sp_dist;
  };

  static void write_ep(std::stringstream &buf,const O_Path &o_path)
  {
    if (o_path.empty()) {
      return;
    };
    int N = o_path.size();
    for (int i = 0; i < N - 1; ++i)
    {
      buf << o_path[i]->obs_prob<< ",";
    }
    buf << o_path[N - 1]->obs_prob;
  };

  static void write_tp(std::stringstream &buf,const O_Path &o_path)
  {
    if (o_path.empty()) return;
    int N = o_path.size();
    for (int i = 0; i < N - 1; ++i)
    {
      float ca = o_path[i]->cumu_prob;
      float cb = o_path[i+1]->cumu_prob;
      float tp = (cb - ca)/(o_path[i+1]->obs_prob+1e-7);
      if (i==N-2) {
        buf << tp;
      } else {
        buf << tp <<",";
      }
    }
  };

  static void write_gps_error(std::stringstream &buf,const O_Path &o_path)
  {
    if (o_path.empty()) {
      return;
    };
    int N = o_path.size();
    for (int i = 0; i < N - 1; ++i)
    {
      buf << o_path[i]->dist<< ",";
    }
    buf << o_path[N - 1]->dist;
  };

  // Write the optimal path
  static void write_pgeom(std::stringstream &buf,const O_Path &o_path)
  {
    if (o_path.empty()) {
      return;
    };
    int N = o_path.size();
    LineString pline;
    // Create a linestring from matched point
    for (int i = 0; i < N; ++i)
    {
      LineString &edge_geom = o_path[i]->edge->geom;
      double px = 0;
      double py = 0;
      ALGORITHM::locate_point_by_offset(
        edge_geom,o_path[i]->offset,&px,&py);
      pline.addPoint(px,py);
    }
    if (!pline.isEmpty()) {
      write_geometry(buf,pline);
    }
  };

  // Write the complete path
  void write_cpath(std::stringstream &buf,const T_Path &t_path) {
    if (t_path.cpath.empty()) return;
    const C_Path &c_path = t_path.cpath;
    int N = c_path.size();
    for (int i = 0; i < N - 1; ++i)
    {
      buf << c_path[i] << ",";
    }
    buf << c_path[N - 1];
  };



  // Write the traversed path separated by vertical bar
  void write_tpath(std::stringstream &buf,const T_Path &t_path) {
    if (t_path.cpath.empty()) return;
    // Iterate through consecutive indexes and write the traversed path
    int J = t_path.indices.size();
    for (int j=0; j<J-1; ++j) {
      int a = t_path.indices[j];
      int b = t_path.indices[j+1];
      for (int i=a; i<b; ++i) {
        buf << t_path.cpath[i];
        buf << ",";
      }
      buf << t_path.cpath[b];
      if (j<J-2) {
        // Last element should not have a bar
        buf << "|";
      }
    }
  };

  static void write_cpath_network(
    std::stringstream &buf, const T_Path &t_path, const Network &network) {
    if (t_path.cpath.empty()) return;
    const C_Path &c_path = t_path.cpath;
    int N = c_path.size();
    for (int i = 0; i < N - 1; ++i)
    {
      buf << network.get_edge_id(c_path[i]) << ",";
    }
    buf << network.get_edge_id(c_path[N - 1]);
  };

  // Write the traversed path separated by vertical bar
  static void write_tpath_network(
    std::stringstream &buf,const T_Path &t_path,const Network &network) {
    if (t_path.cpath.empty()) return;
    // Iterate through consecutive indexes and write the traversed path
    int J = t_path.indices.size();
    for (int j=0; j<J-1; ++j) {
      int a = t_path.indices[j];
      int b = t_path.indices[j+1];
      for (int i=a; i<b; ++i) {
        buf << network.get_edge_id(t_path.cpath[i]);
        buf << ",";
      }
      buf << network.get_edge_id(t_path.cpath[b]);
      if (j<J-2) {
        // Last element should not have a bar
        buf << "|";
      }
    }
  };
private:
  std::ofstream fstream;
  Network &network;
  ResultConfig &config;
}; // ResultWriter
}; //IO
} //MM
#endif // MM_WRITER_HPP
