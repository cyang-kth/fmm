/**
 * Content
 * Utility functions 
 *      
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef MM_UTIL_HPP
#define MM_UTIL_HPP
#include "types.hpp"
#include "gdal/ogrsf_frmts.h" // C++ API for GDAL
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <cmath>
#include <chrono>
#include <ctime>
#include "multilevel_debug.h"
#include "float.h"
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

/**
 *  Print the candidates of trajectory in a table with header of
 *  step;offset;distance;edge_id
 */
void print_traj_candidates(Traj_Candidates &tr_cs) {
    std::cout << "step;offset;distance;edge_id" << std::endl;
    Traj_Candidates::iterator tr_cs_iter;
    Point_Candidates::iterator p_cs_iter;
    for (tr_cs_iter = tr_cs.begin(); tr_cs_iter != tr_cs.end(); ++tr_cs_iter) {
        for (p_cs_iter = tr_cs_iter->begin(); p_cs_iter != tr_cs_iter->end(); ++p_cs_iter) {
            std::cout << std::distance(tr_cs.begin(), tr_cs_iter) << ";" << p_cs_iter->offset << ";" << p_cs_iter->dist << ";" << p_cs_iter->edge->id_attr<< std::endl;
        }
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
    std::cout<<std::endl;
}

/** 
 * Print the number of candidates for each point of trajectory
 */
void print_traj_candidates_count(Traj_Candidates &tr_cs) {
    Traj_Candidates::iterator tr_cs_iter;
    std::cout<<"Summary of transition graph: "<<std::endl;
    std::cout<<"Count of candidates"<<std::endl;
    for (tr_cs_iter = tr_cs.begin(); tr_cs_iter != tr_cs.end(); ++tr_cs_iter) {
        std::cout<< tr_cs_iter->size() <<" ";
    }
    std::cout<< std::endl;
};
/**
 * Print the OGRLineString in WKT format
 */
void print_geometry(OGRLineString *geom){
    if (geom==nullptr) {
        std::cout<<"Geometry in WKT: NULL"<<std::endl;
        return;
    }
    char *wkt;
    geom->exportToWkt(&wkt);
    std::cout<<"Geometry in WKT: "<<wkt<<std::endl;
    CPLFree(wkt);
};


// Get current timestamp
std::chrono::time_point<std::chrono::system_clock> get_current_time(){
    return std::chrono::system_clock::now();
};

// Print a timestamp
void print_time(const std::chrono::time_point<std::chrono::system_clock> &start_time){
    std::time_t start_time_t = std::chrono::system_clock::to_time_t(start_time);
    std::cout<<"Time "<<std::ctime(&start_time_t)<<std::endl;
};

/**
 * Calculate the duration between two timestamps in unit of seconds
 */
double get_duration(const std::chrono::time_point<std::chrono::system_clock> &start_time,
const std::chrono::time_point<std::chrono::system_clock> &end_time){
    std::chrono::duration<double> elapsed_seconds = end_time-start_time;
    return elapsed_seconds.count();
};

} // Util
} // MM
#endif /* MM_UTIL_HPP */
