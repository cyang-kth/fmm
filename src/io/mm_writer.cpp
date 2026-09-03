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
#include <omp.h>

#include <charconv>
#include <sstream>
#include <string>
#include <vector>

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

namespace {

// The helpers below produce exactly the text a std::ostream produces for the
// same values (integers in decimal, floating point values as %g with the
// given number of significant digits), without the cost of a string stream.

inline void append_int(std::string &out, long long value) {
  char buf[24];
  auto r = std::to_chars(buf, buf + sizeof(buf), value);
  out.append(buf, r.ptr);
}

inline void append_double(std::string &out, double value, int precision) {
  char buf[64];
  auto r = std::to_chars(buf, buf + sizeof(buf), value,
                         std::chars_format::general, precision);
  out.append(buf, r.ptr);
}

template<typename T>
void append_ids(std::string &out, const std::vector<T> &values) {
  for (std::size_t i = 0; i < values.size(); ++i) {
    if (i > 0) out += ',';
    append_int(out, values[i]);
  }
}

// Same text as `os << line` (FMM::CORE::operator<<, i.e. the WKT written by
// boost with 12 significant digits) for a non empty linestring. The empty
// case is left to boost so that its representation stays whatever boost
// produces.
void append_wkt(std::string &out, const FMM::CORE::LineString &line) {
  int N = line.get_num_points();
  if (N == 0) {
    std::ostringstream os;
    os << line;
    out += os.str();
    return;
  }
  out += "LINESTRING(";
  for (int i = 0; i < N; ++i) {
    if (i > 0) out += ',';
    append_double(out, line.get_x(i), 12);
    out += ' ';
    append_double(out, line.get_y(i), 12);
  }
  out += ')';
}

} // namespace

void CSVMatchResultWriter::write_result(
    const FMM::CORE::Trajectory &traj,
    const FMM::MM::MatchResult &result) {
  std::string buf;
  buf.reserve(4096);
  // FMM::CORE::operator<< sets the precision of the stream to 12 when a
  // linestring is written, and that setting persists for the fields written
  // afterwards in the same line. The same behaviour is reproduced here.
  int precision = 6;
  append_int(buf, result.id);
  if (config_.write_opath) {
    buf += ';';
    append_ids(buf, result.opath);
  }
  if (config_.write_error) {
    buf += ';';
    if (!result.opt_candidate_path.empty()) {
      int N = result.opt_candidate_path.size();
      for (int i = 0; i < N - 1; ++i) {
        append_double(buf, result.opt_candidate_path[i].c.dist, precision);
        buf += ',';
      }
      append_double(buf, result.opt_candidate_path[N - 1].c.dist, precision);
    }
  }
  if (config_.write_offset) {
    buf += ';';
    if (!result.opt_candidate_path.empty()) {
      int N = result.opt_candidate_path.size();
      for (int i = 0; i < N - 1; ++i) {
        append_double(buf, result.opt_candidate_path[i].c.offset, precision);
        buf += ',';
      }
      append_double(buf, result.opt_candidate_path[N - 1].c.offset, precision);
    }
  }
  if (config_.write_spdist) {
    buf += ';';
    if (!result.opt_candidate_path.empty()) {
      int N = result.opt_candidate_path.size();
      for (int i = 1; i < N; ++i) {
        append_double(buf, result.opt_candidate_path[i].sp_dist, precision);
        if (i != N - 1) buf += ',';
      }
    }
  }
  if (config_.write_pgeom) {
    buf += ';';
    if (!result.opt_candidate_path.empty()) {
      int N = result.opt_candidate_path.size();
      FMM::CORE::LineString pline;
      for (int i = 0; i < N; ++i) {
        const FMM::CORE::Point &point = result.opt_candidate_path[i].c.point;
        pline.add_point(point);
      }
      append_wkt(buf, pline);
      precision = 12;
    }
  }
  // Write fields related with cpath
  if (config_.write_cpath) {
    buf += ';';
    append_ids(buf, result.cpath);
  }
  if (config_.write_tpath) {
    buf += ';';
    if (!result.cpath.empty()) {
      // Iterate through consecutive indexes and write the traversed path
      int J = result.indices.size();
      for (int j = 0; j < J - 1; ++j) {
        int a = result.indices[j];
        int b = result.indices[j + 1];
        for (int i = a; i < b; ++i) {
          append_int(buf, result.cpath[i]);
          buf += ',';
        }
        append_int(buf, result.cpath[b]);
        if (j < J - 2) {
          // Last element should not have a bar
          buf += '|';
        }
      }
    }
  }
  if (config_.write_mgeom) {
    buf += ';';
    append_wkt(buf, result.mgeom);
    precision = 12;
  }
  if (config_.write_ep) {
    buf += ';';
    if (!result.opt_candidate_path.empty()) {
      int N = result.opt_candidate_path.size();
      for (int i = 0; i < N - 1; ++i) {
        append_double(buf, result.opt_candidate_path[i].ep, precision);
        buf += ',';
      }
      append_double(buf, result.opt_candidate_path[N - 1].ep, precision);
    }
  }
  if (config_.write_tp) {
    buf += ';';
    if (!result.opt_candidate_path.empty()) {
      int N = result.opt_candidate_path.size();
      for (int i = 0; i < N - 1; ++i) {
        append_double(buf, result.opt_candidate_path[i].tp, precision);
        buf += ',';
      }
      append_double(buf, result.opt_candidate_path[N - 1].tp, precision);
    }
  }
  if (config_.write_length) {
    buf += ';';
    if (!result.opt_candidate_path.empty()) {
      int N = result.opt_candidate_path.size();
      SPDLOG_TRACE("Write length for {} edges",N);
      for (int i = 0; i < N - 1; ++i) {
        append_double(buf, result.opt_candidate_path[i].c.edge->length,
                      precision);
        buf += ',';
      }
      append_double(buf, result.opt_candidate_path[N - 1].c.edge->length,
                    precision);
    }
  }
  if (config_.write_duration) {
    buf += ';';
    if (!traj.timestamps.empty()) {
      int N = traj.timestamps.size();
      SPDLOG_TRACE("Write duration for {} points",N);
      for (int i = 1; i < N; ++i) {
        append_double(buf, traj.timestamps[i] - traj.timestamps[i-1],
                      precision);
        if (i != N - 1) buf += ',';
      }
    }
  }
  if (config_.write_speed) {
    buf += ';';
    if (!result.opt_candidate_path.empty() && !traj.timestamps.empty()) {
      int N = traj.timestamps.size();
      for (int i = 1; i < N; ++i) {
        double duration = traj.timestamps[i] - traj.timestamps[i-1];
        append_double(buf, duration > 0 ?
                      (result.opt_candidate_path[i].sp_dist / duration) : 0,
                      precision);
        if (i != N - 1) buf += ',';
      }
    }
  }
  buf += '\n';
  // Ensure that fstream is called corrected in OpenMP
  #pragma omp critical
  m_fstream.write(buf.data(), buf.size());
}

} //IO
} //MM
