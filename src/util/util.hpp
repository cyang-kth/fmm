/**
 * Fast map matching.
 *
 * Utility functions
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef FMM_UTIL_HPP
#define FMM_UTIL_HPP

#include "mm/mm_type.hpp"

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

/**
 * Print a vector of values
 * @tparam T vector element type
 * @param os stream to write result
 * @param vec input vector
 * @return the stream with comma separated values of the vector written
 */
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

/**
 * Write trajectory candidate to a stream
 * @param os stream to write
 * @param tr_cs trajectory candidate
 * @return the stream with trajectory candidate information written
 */
std::ostream &operator<<(std::ostream &os,
    const FMM::MM::Traj_Candidates &tr_cs);

/**
 * Write optimal candidate path into a stream
 * @param os stream to write
 * @param opath optimal candidate path
 * @return the stream with candidate path information written
 */
std::ostream &operator<<(std::ostream &os,
    const FMM::MM::OptCandidatePath &opath);

/**
 * Write a point into a stream
 * @param os stream to write
 * @param geom point
 * @return the stream with wkt point written.
 */
std::ostream &operator<<(std::ostream &os,
    const FMM::CORE::Point &geom);

} // namespace std

namespace FMM {

/**
 * Utility functions
 */
namespace UTIL {

/**
 * Time point
 */
typedef std::chrono::steady_clock::time_point TimePoint;

/**
 * Check if file exist or not
 * @param  filename file name
 * @return true if file exists
 */
bool file_exists(const char *filename);
/**
 * Check if file exist or not
 * @param  filename file name
 * @return true if file exists
 */
bool file_exists(const std::string &filename);
/**
 * Check if folder exists or not
 * @param  folder_name folder name
 * @return true if folder exists
 */
bool folder_exist(const std::string &folder_name);
/**
 * Get folder path from file path
 * @param  fn File path
 * @return Folder path to a file
 */
std::string get_file_directory(const std::string &fn);

/**
 * Convert string to bool
 * @param  str input string
 * @return true if str equals "true", "t" or "1";
 */
bool string2bool(const std::string &str);

/**
 * Convert bool to string
 * @param  value bool value
 * @return  "true" if value is true
 */
inline std::string bool2string(bool value) {
  return (value ? "true" : "false");
}

/**
 * Check if the filename has an extension in the list
 * @param filename
 * @param extension_list_str a list of string separated by ,
 * @return true if file extension is in the list
 */
bool check_file_extension(const std::string &filename,
                          const std::string &extension_list_str);

/**
 * Convert a vector of type into a string with delimiter of ,
 * @param  vec input vector
 * @return  a string of values in the vector delimited by ,
 */
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

/**
 * Convert string to vector
 * @param  str a string containing a list of values separated by ,
 * @return  a vector of values
 */
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

/**
 * Split a string containing string separated by , into a vector of string
 * @param str input string
 * @return a vector of strings
 */
std::vector<std::string> split_string(const std::string &str);

/**
 * Get current timestamp
 * @return a timestamp point
 */
std::chrono::time_point<std::chrono::system_clock> get_current_time();

/**
 * Print a timestamp
 * @param start_time timestamp point
 */
void print_time(
    const std::chrono::time_point<std::chrono::system_clock> &start_time);

/**
 * Calculate the duration between two time points
 * @param start_time start time point
 * @param end_time end time point
 * @return the duration between two time points
 */
double get_duration(
    const std::chrono::time_point<std::chrono::system_clock> &start_time,
    const std::chrono::time_point<std::chrono::system_clock> &end_time);

} // Util
} // FMM
#endif /* FMM_UTIL_HPP */
