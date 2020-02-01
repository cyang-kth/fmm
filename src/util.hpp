/**
 * Content
 * Utility functions
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */

#ifndef MM_UTIL_HPP
#define MM_UTIL_HPP

#include <ogrsf_frmts.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <cmath>
#include <cfloat>
#include <chrono>
#include <ctime>

#include "types.hpp"
#include "debug.h"

namespace MM
{
namespace UTIL
{
/**
 * Check if the file exists or not
 */
bool fileExists(const char *filename)
{
  struct stat buf;
  if (stat(filename, &buf) != -1)
  {
    return true;
  }
  return false;
};

bool fileExists(std::string &filename)
{
  return fileExists(filename.c_str());
};

bool folderExists(std::string &folder_name)
{
  if (folder_name.empty()) return true;
  struct stat sb;
  if (stat(folder_name.c_str(),&sb) == 0 && S_ISDIR(sb.st_mode)) {
    return true;
  }
  return false;
}

// Check extension of the file, 0 for CSV and 1 for Binary
int get_file_extension(std::string &fn) {
  std::string fn_extension = fn.substr(fn.find_last_of(".") + 1);
  if (fn_extension == "csv" || fn_extension == "txt") {
    return 0;
  } else if (fn_extension == "bin" || fn_extension == "binary") {
    return 1;
  }
  return 2;
};

std::string get_file_directory(std::string &fn){
  std::size_t found = fn.find_last_of("/");
  if (found!=std::string::npos){
    return fn.substr(0,found);
  }
  return {};
};

/**
 *  Print the candidates of trajectory in a table with header of
 *  step;offset;distance;edge_id
 */
void print_traj_candidates(Traj_Candidates &tr_cs) {
  std::cout << "step;offset;distance;id;edge_id_attr" << '\n';
  Traj_Candidates::iterator tr_cs_iter;
  Point_Candidates::iterator p_cs_iter;
  for (tr_cs_iter = tr_cs.begin(); tr_cs_iter != tr_cs.end(); ++tr_cs_iter) {
    for (p_cs_iter = tr_cs_iter->begin();
         p_cs_iter != tr_cs_iter->end(); ++p_cs_iter) {
      std::cout << std::distance(tr_cs.begin(), tr_cs_iter) << ";"
                << p_cs_iter->offset << ";"
                << p_cs_iter->dist << ";"
                <<  p_cs_iter->edge->index
                << ";" << p_cs_iter->edge->id<< '\n';
    }
  }
}

void print_traj_candidates_summary(Traj_Candidates &tr_cs) {
  std::cout << "point_idx;candidate_count" << '\n';
  Traj_Candidates::iterator tr_cs_iter;
  Point_Candidates::iterator p_cs_iter;
  for (tr_cs_iter = tr_cs.begin(); tr_cs_iter != tr_cs.end(); ++tr_cs_iter) {
    std::cout << std::distance(tr_cs.begin(), tr_cs_iter) << ";"
              << tr_cs_iter->size() << '\n';
  }
}

// Print a complete path
void print_c_path(C_Path *c_path_ptr) {
  std::cout<<"Complete path elements:";
  int N = c_path_ptr->size();
  for(int i=0; i<N; ++i)
  {
    std::cout<< (*c_path_ptr)[i]<<" ";
  }
  std::cout<<'\n';
}

/**
 * Print the number of candidates for each point of trajectory
 */
void print_traj_candidates_count(Traj_Candidates &tr_cs) {
  Traj_Candidates::iterator tr_cs_iter;
  std::cout<<"Summary of transition graph: "<<'\n';
  std::cout<<"Count of candidates"<<'\n';
  for (tr_cs_iter = tr_cs.begin(); tr_cs_iter != tr_cs.end(); ++tr_cs_iter) {
    std::cout<< tr_cs_iter->size() <<" ";
  }
  std::cout<< '\n';
};
/**
 * Print the OGRLineString in WKT format
 */
void print_geometry(LineString *geom){
  if (geom==nullptr) {
    std::cout<<"Geometry in WKT: NULL"<<'\n';
    return;
  }
  std::cout<< geom->exportToWkt()<<'\n';
};

// Get current timestamp
std::chrono::time_point<std::chrono::system_clock> get_current_time(){
  return std::chrono::system_clock::now();
};

// Print a timestamp
void print_time(
  const std::chrono::time_point<std::chrono::system_clock> &start_time){
  std::time_t start_time_t = std::chrono::system_clock::to_time_t(start_time);
  std::cout<<"Time "<<std::ctime(&start_time_t)<<'\n';
};

/**
 * Calculate the duration between two timestamps in unit of seconds
 */
double get_duration(
  const std::chrono::time_point<std::chrono::system_clock> &start_time,
  const std::chrono::time_point<std::chrono::system_clock> &end_time){
  std::chrono::duration<double> elapsed_seconds = end_time-start_time;
  return elapsed_seconds.count();
};

} // Util
} // MM
#endif /* MM_UTIL_HPP */
