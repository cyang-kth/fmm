#include "util/util.hpp"

#include <iostream>
#include <string>
#include <sys/stat.h>
#include <chrono>
#include <vector>
#include <ctime>
#include "mm/mm_type.hpp"

namespace std {

std::ostream &operator<<(std::ostream &os,
                         const FMM::MM::Traj_Candidates &tr_cs) {
  os << "\nCandidate "
     << std::fixed << std::setw(4) << "step" << ";"
     << std::fixed << std::setw(6) << "index" << ";"
     << std::fixed << std::setw(8) << "offset" << ";"
     << std::fixed << std::setw(8) << "distance" << ";"
     << std::fixed << std::setw(8) << "edge_id" << '\n';
  for (auto tr_cs_iter = tr_cs.begin();
       tr_cs_iter != tr_cs.end(); ++tr_cs_iter) {
    for (auto p_cs_iter = tr_cs_iter->begin();
         p_cs_iter != tr_cs_iter->end();
         ++p_cs_iter) {
      os << "Candidate "
         << std::fixed << std::setw(4) << std::distance(tr_cs.begin(),
                                                        tr_cs_iter) << ";"
         << std::fixed << std::setw(6) << p_cs_iter->index << ";"
         << std::fixed << std::setw(8) << p_cs_iter->offset << ";"
         << std::fixed << std::setw(8) << p_cs_iter->dist << ";"
         << std::fixed << std::setw(8) << p_cs_iter->edge->id << '\n';
    }
  }
  return os;
}

std::ostream &operator<<(std::ostream &os,
                         const FMM::MM::OptCandidatePath &opath) {
  for (int i = 0; i < opath.size(); ++i) {
    // std::cout <<"Write edge "<< i <<" edge "<< opath[i]->edge->id <<"\n";
    os << opath[i]->edge->id;
    if (i != opath.size() - 1)
      os << ",";
  }
  return os;
}

std::ostream &operator<<(std::ostream &os,
                         const FMM::CORE::Point &geom) {
  os << std::setprecision(12) << boost::geometry::wkt(geom);
  return os;
}

} // namespace std

namespace FMM {

namespace UTIL {

bool file_exists(const char *filename) {
  struct stat buf;
  if (stat(filename, &buf) != -1) {
    return true;
  }
  return false;
}

bool file_exists(const std::string &filename) {
  return file_exists(filename.c_str());
}

std::vector<std::string> split_string(const std::string &str) {
  char delim = ',';
  std::vector<std::string> result;
  std::stringstream ss(str);
  std::string intermediate;
  while (getline(ss, intermediate, delim)) {
    result.push_back(intermediate);
  }
  return result;
}

bool string2bool(const std::string &str) {
  return str == "true" || str == "t" || str == "1";
}

std::chrono::time_point<std::chrono::system_clock> get_current_time() {
  return std::chrono::system_clock::now();
}

// Print a timestamp
void print_time(
    const std::chrono::time_point<std::chrono::system_clock> &start_time) {
  std::time_t start_time_t = std::chrono::system_clock::to_time_t(start_time);
  std::cout << "Time " << std::ctime(&start_time_t) << '\n';
}

double get_duration(
    const std::chrono::time_point<std::chrono::system_clock> &start_time,
    const std::chrono::time_point<std::chrono::system_clock> &end_time) {
  std::chrono::duration<double> elapsed_seconds = end_time - start_time;
  return elapsed_seconds.count();
}
bool check_file_extension(const std::string &filename,
                          const std::string &extension_list_str) {
  bool result = false;
  std::stringstream ss;
  std::string fn_extension = filename.substr(
      filename.find_last_of('.') + 1);
  std::vector<std::string> extensions =
      split_string(extension_list_str);
  for (const auto &extension:extensions) {
    if (fn_extension == extension)
      result = true;
  }
  return result;
}

bool folder_exist(const std::string &folder_name) {
  if (folder_name.empty()) return true;
  struct stat sb;
  if (stat(folder_name.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
    return true;
  }
  return false;
}

std::string get_file_directory(const std::string &fn) {
  std::size_t found = fn.find_last_of("/");
  if (found != std::string::npos) {
    return fn.substr(0, found);
  }
  return {};
};

} // Util
} // FMM
