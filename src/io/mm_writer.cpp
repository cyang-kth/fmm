/**
 * Content
 * Definition of MatchResultWriter Class, which contains functions for
 * writing the results.
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#include "io/mm_writer.hpp"
#include "util/util.hpp"
#include "util/debug.hpp"
#include "config/result_config.hpp"

#include <sstream>

namespace FMM {

namespace IO {

CSVMatchResultWriter::CSVMatchResultWriter(
  const std::string &result_file, const CONFIG::OutputConfig &config_arg) :
  m_fstream(result_file), config_(config_arg) {
  write_header();
}

void CSVMatchResultWriter::write_header() {
  std::string header = "id";
  if (config_.write_opath) header += ";opath";
  if (config_.write_error) header += ";error";
  if (config_.write_offset) header += ";offset";
  if (config_.write_spdist) header += ";spdist";
  if (config_.write_pgeom) header += ";pgeom";
  if (config_.write_cpath) header += ";cpath";
  if (config_.write_tpath) header += ";tpath";
  if (config_.write_mgeom) header += ";mgeom";
  if (config_.write_ep) header += ";ep";
  if (config_.write_tp) header += ";tp";
  if (config_.write_length) header += ";length";
  if (config_.write_duration) header += ";duration";
  if (config_.write_speed) header += ";speed";
  m_fstream << header << '\n';
}

void CSVMatchResultWriter::write_result(
  const FMM::CORE::Trajectory &traj,
  const FMM::MM::MatchResult &result){
  std::stringstream buf;
  write_result_(buf,traj,result.opt_candidate_path,
                result.opath, result.cpath, result.indices);
  write_linestring_(buf,result.mgeom);
  buf << '\n';
  // Ensure that fstream is called corrected in OpenMP
  #pragma omp critical
  m_fstream << buf.rdbuf();
};

void CSVMatchResultWriter::write_result(
  const FMM::CORE::Trajectory &traj,
  const FMM::MM::PartialMatchResult &result){
  std::stringstream buf;
  write_result_(buf,traj,result.opt_candidate_path,
                result.opath, result.cpath, result.indices);
  write_multilinestring_(buf,result.mgeom);
  buf << '\n';
  // Ensure that fstream is called corrected in OpenMP
  #pragma omp critical
  m_fstream << buf.rdbuf();
};

void CSVMatchResultWriter::write_linestring_(
  std::stringstream &buf,const FMM::CORE::LineString &line){
  if (config_.write_mgeom) {
    buf << ";" << line;
  }
};

void CSVMatchResultWriter::write_multilinestring_(
  std::stringstream &buf,const FMM::CORE::MultiLineString &mline){
  if (config_.write_mgeom) {
    buf << ";" << mline;
  }
};

void CSVMatchResultWriter::write_result_(
  std::stringstream &buf,
  const FMM::CORE::Trajectory &traj,
  const FMM::MM::MatchedCandidatePath &opt_candidate_path,
  const FMM::MM::O_Path &opath,
  const FMM::MM::C_Path &cpath,
  const std::vector<int> &indices) {
  buf << traj.id;
  if (config_.write_opath) {
    buf << ";" << opath;
  }
  if (config_.write_error) {
    buf << ";";
    if (!opt_candidate_path.empty()) {
      int N = opt_candidate_path.size();
      for (int i = 0; i < N - 1; ++i) {
        buf << opt_candidate_path[i].c.dist << ",";
      }
      buf << opt_candidate_path[N - 1].c.dist;
    }
  }
  if (config_.write_offset) {
    buf << ";";
    if (!opt_candidate_path.empty()) {
      int N = opt_candidate_path.size();
      for (int i = 0; i < N - 1; ++i) {
        buf << opt_candidate_path[i].c.offset << ",";
      }
      buf << opt_candidate_path[N - 1].c.offset;
    }
  }
  if (config_.write_spdist) {
    buf << ";";
    if (!opt_candidate_path.empty()) {
      int N = opt_candidate_path.size();
      for (int i = 1; i < N; ++i) {
        buf << opt_candidate_path[i].sp_dist
            << (i==N-1 ? "" : ",");
      }
    }
  }
  if (config_.write_pgeom) {
    buf << ";";
    if (!opt_candidate_path.empty()) {
      int N = opt_candidate_path.size();
      FMM::CORE::LineString pline;
      for (int i = 0; i < N; ++i) {
        const FMM::CORE::Point &point = opt_candidate_path[i].c.point;
        pline.add_point(point);
      }
      buf << pline;
    }
  }
  // Write fields related with cpath
  if (config_.write_cpath) {
    buf << ";" << cpath;
  }
  if (config_.write_tpath) {
    buf << ";";
    if (!cpath.empty()) {
      // Iterate through consecutive indexes and write the traversed path
      int J = indices.size();
      for (int j = 0; j < J - 1; ++j) {
        int a = indices[j];
        int b = indices[j + 1];
        for (int i = a; i < b; ++i) {
          buf << cpath[i];
          buf << ",";
        }
        buf << cpath[b];
        if (j < J - 2) {
          // Last element should not have a bar
          buf << "|";
        }
      }
    }
  }
  if (config_.write_ep) {
    buf << ";";
    if (!opt_candidate_path.empty()) {
      int N = opt_candidate_path.size();
      for (int i = 0; i < N - 1; ++i) {
        buf << opt_candidate_path[i].ep << ",";
      }
      buf << opt_candidate_path[N - 1].ep;
    }
  }
  if (config_.write_tp) {
    buf << ";";
    if (!opt_candidate_path.empty()) {
      int N = opt_candidate_path.size();
      for (int i = 0; i < N - 1; ++i) {
        buf << opt_candidate_path[i].tp << ",";
      }
      buf << opt_candidate_path[N - 1].tp;
    }
  }
  if (config_.write_length) {
    buf << ";";
    if (!opt_candidate_path.empty()) {
      int N = opt_candidate_path.size();
      SPDLOG_TRACE("Write length for {} edges",N);
      for (int i = 0; i < N - 1; ++i) {
        // SPDLOG_TRACE("Write length {}",i);
        buf << opt_candidate_path[i].c.edge->length << ",";
      }
      // SPDLOG_TRACE("Write length {}",N-1);
      buf << opt_candidate_path[N - 1].c.edge->length;
    }
  }
  if (config_.write_duration) {
    buf << ";";
    if (!traj.timestamps.empty()) {
      int N = traj.timestamps.size();
      SPDLOG_TRACE("Write duration for {} points",N);
      for (int i = 1; i < N; ++i) {
        // SPDLOG_TRACE("Write length {}",i);
        buf << traj.timestamps[i] - traj.timestamps[i-1]
            << (i==N-1 ? "" : ",");
      }
    }
  }
  if (config_.write_speed) {
    buf << ";";
    if (!opt_candidate_path.empty() && !traj.timestamps.empty()) {
      int N = traj.timestamps.size();
      for (int i = 1; i < N; ++i) {
        double duration = traj.timestamps[i] - traj.timestamps[i-1];
        buf << (duration>0 ? (opt_candidate_path[i].sp_dist/duration) : 0)
            << (i==N-1 ? "" : ",");
      }
    }
  }
}

} //IO
} //MM
