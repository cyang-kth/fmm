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
    const FMM::MM::MatchResult &result) {
  std::stringstream buf;
  buf << result.id;
  if (config_.write_opath) {
    buf << ";" << result.opath;
  }
  if (config_.write_error) {
    buf << ";";
    if (!result.opt_candidate_path.empty()) {
      int N = result.opt_candidate_path.size();
      for (int i = 0; i < N - 1; ++i) {
        buf << result.opt_candidate_path[i].c.dist << ",";
      }
      buf << result.opt_candidate_path[N - 1].c.dist;
    }
  }
  if (config_.write_offset) {
    buf << ";";
    if (!result.opt_candidate_path.empty()) {
      int N = result.opt_candidate_path.size();
      for (int i = 0; i < N - 1; ++i) {
        buf << result.opt_candidate_path[i].c.offset << ",";
      }
      buf << result.opt_candidate_path[N - 1].c.offset;
    }
  }
  if (config_.write_spdist) {
    buf << ";";
    if (!result.opt_candidate_path.empty()) {
      int N = result.opt_candidate_path.size();
      for (int i = 1; i < N; ++i) {
        buf << result.opt_candidate_path[i].sp_dist
            << (i==N-1?"":",");
      }
    }
  }
  if (config_.write_pgeom) {
    buf << ";";
    if (!result.opt_candidate_path.empty()) {
      int N = result.opt_candidate_path.size();
      FMM::CORE::LineString pline;
      for (int i = 0; i < N; ++i) {
        const FMM::CORE::Point &point = result.opt_candidate_path[i].c.point;
        pline.add_point(point);
      }
      buf << pline;
    }
  }
  // Write fields related with cpath
  if (config_.write_cpath) {
    buf << ";" << result.cpath;
  }
  if (config_.write_tpath) {
    buf << ";";
    if (!result.cpath.empty()) {
      // Iterate through consecutive indexes and write the traversed path
      int J = result.indices.size();
      for (int j = 0; j < J - 1; ++j) {
        int a = result.indices[j];
        int b = result.indices[j + 1];
        for (int i = a; i < b; ++i) {
          buf << result.cpath[i];
          buf << ",";
        }
        buf << result.cpath[b];
        if (j < J - 2) {
          // Last element should not have a bar
          buf << "|";
        }
      }
    }
  }
  if (config_.write_mgeom) {
    buf << ";" << result.mgeom;
  }
  if (config_.write_ep) {
    buf << ";";
    if (!result.opt_candidate_path.empty()) {
      int N = result.opt_candidate_path.size();
      for (int i = 0; i < N - 1; ++i) {
        buf << result.opt_candidate_path[i].ep << ",";
      }
      buf << result.opt_candidate_path[N - 1].ep;
    }
  }
  if (config_.write_tp) {
    buf << ";";
    if (!result.opt_candidate_path.empty()) {
      int N = result.opt_candidate_path.size();
      for (int i = 0; i < N - 1; ++i) {
        buf << result.opt_candidate_path[i].tp << ",";
      }
      buf << result.opt_candidate_path[N - 1].tp;
    }
  }
  if (config_.write_length) {
    buf << ";";
    if (!result.opt_candidate_path.empty()) {
      int N = result.opt_candidate_path.size();
      SPDLOG_TRACE("Write length for {} edges",N);
      for (int i = 0; i < N - 1; ++i) {
        // SPDLOG_TRACE("Write length {}",i);
        buf << result.opt_candidate_path[i].c.edge->length << ",";
      }
      // SPDLOG_TRACE("Write length {}",N-1);
      buf << result.opt_candidate_path[N - 1].c.edge->length;
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
            << (i==N-1?"":",");
      }
    }
  }
  if (config_.write_speed) {
    buf << ";";
    if (!result.opt_candidate_path.empty() && !traj.timestamps.empty()) {
      int N = traj.timestamps.size();
      for (int i = 1; i < N; ++i) {
        double duration = traj.timestamps[i] - traj.timestamps[i-1];
        buf << (duration>0?(result.opt_candidate_path[i].sp_dist/duration):0)
            << (i==N-1?"":",");
      }
    }
  }
  buf << '\n';
  // Ensure that fstream is called corrected in OpenMP
  #pragma omp critical
  m_fstream << buf.rdbuf();
}

} //IO
} //MM
