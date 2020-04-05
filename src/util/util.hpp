/**
 * Content
 * Utility functions
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef FMM_UTIL_HPP
#define FMM_UTIL_HPP

#include "network/type.hpp"

#include <ogrsf_frmts.h> // C++ API for GDAL
#include <cfloat>
#include <iostream>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include <cmath>
#include <chrono>
#include <vector>
#include <ctime>

/**
 * Utility functions for writing data to std stream
 */
namespace std {

template<typename T>
std::ostream &operator<<(std::ostream &os,
                         const std::vector<T> &vec) {
  if (!vec.empty()) {
    std::copy(vec.begin(), vec.end() - 1,
              std::ostream_iterator<T>(os, ","));
    os << vec.back();
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const FMM::Traj_Candidates &tr_cs);

std::ostream &operator<<(std::ostream &os, const FMM::OptCandidatePath &opath);

std::ostream &operator<<(std::ostream &os, const FMM::Point &geom);

} // namespace std

namespace FMM {

inline double meter2degree(double dist_meter);

/**
 * Utility functions
 */
namespace UTIL {

typedef std::chrono::steady_clock::time_point TimePoint;

double meter2degree(double dist_meter);

/**
 * Check if the file exists or not
 */
bool file_exists(const char *filename);
bool file_exists(const std::string &filename);
bool folder_exist(const std::string &folder_name);
std::string get_file_directory(const std::string &fn);


bool string2bool(const std::string &str);

inline std::string bool2string(bool value) {
  return (value ? "true" : "false");
}

/**
 * Check if the filename has an extension in the list
 * @param filename
 * @param extension_list_str a list of string separated by ,
 * @return
 */
bool check_file_extension(const std::string &filename,
                          const std::string &extension_list_str);

template<typename T>
std::string vec2string(
    const std::vector<T> &vec) {
  std::ostringstream vts;
  if (!vec.empty()) {
    std::copy(vec.begin(), vec.end() - 1,
              std::ostream_iterator<T>(vts, ","));
    vts << vec.back();
  }
  return vts.str();
}

template<typename T>
std::vector<T> string2vec(
    const std::string &str) {
  std::vector<T> vec;
  std::stringstream ss(str);
  T i;
  while (ss >> i) {
    vec.push_back(i);
    if (ss.peek() == ',')
      ss.ignore();
  }
  return vec;
}

std::vector<std::string> split_string(const std::string &str);

/**
 *  Print the candidates of trajectory in a table with header of
 *  step;offset;distance;edge_id
 */
void print_traj_candidates(Traj_Candidates &tr_cs);

void print_traj_candidates_summary(Traj_Candidates &tr_cs);

/**
 * Print the number of candidates for each point of trajectory
 */
void print_traj_candidates_count(Traj_Candidates &tr_cs);

/**
 * Print the OGRLineString in WKT format
 */
void print_geometry(LineString &geom);

// Get current timestamp
std::chrono::time_point<std::chrono::system_clock> get_current_time();

// Print a timestamp
void print_time(
    const std::chrono::time_point<std::chrono::system_clock> &start_time);

/**
 * Calculate the duration between two timestamps in unit of seconds
 */
double get_duration(
    const std::chrono::time_point<std::chrono::system_clock> &start_time,
    const std::chrono::time_point<std::chrono::system_clock> &end_time);

} // Util
} // FMM
#endif /* FMM_UTIL_HPP */
