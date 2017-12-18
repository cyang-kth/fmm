/**
 * Content
 * Definition of ResultWriter Class, which contains functions for 
 * writing the results.
 *      
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef MM_WRITER_HPP /* Currently not used */
#define MM_WRITER_HPP
#include <iostream>
#include <fstream>
#include "types.hpp"
#include "multilevel_debug.h"
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
     * @param network_ptr, a pointer to the network 
     */
    ResultWriter(const std::string &result_file,Network *network_ptr):
    m_fstream_ptr(new std::ofstream(result_file)),
    m_fstream(*m_fstream_ptr),m_network_ptr(network_ptr)
    {
        std::cout<<"Write result to file: "<<result_file<<std::endl;
    };
    // Destructor
    ~ResultWriter(){
        delete m_fstream_ptr;
    };
    /**
     * Write the results to a CSV file storing:
     *     trajectory id;optimal path;complete path;geometry(WKT)
     */
    void write_map_matched_result_wkt(int tr_id,O_Path *o_path_ptr,C_Path *c_path_ptr, OGRLineString *m_geom){
        DEBUG(2) std::cout<<__FILE__<<"    Line"<<__LINE__<<":    "<<__FUNCTION__<<std::endl;
        m_fstream<<tr_id;
        m_fstream<<";";
        write_o_path(o_path_ptr);
        m_fstream<<";";
        write_complete_path(c_path_ptr);
        m_fstream<<";";
        if (m_geom!=nullptr){
            char *wkt;
            m_geom->exportToWkt(&wkt);
            m_fstream<<wkt<<std::endl;
            CPLFree(wkt);
        } else {
            m_fstream<<std::endl;
        }
    };
    /**
     * Write the results to a CSV file storing:
     *     trajectory id;optimal path;complete path;geometry(WKB)
     */
    void write_map_matched_result_wkb(int tr_id,O_Path *o_path_ptr,C_Path *c_path_ptr, OGRLineString *m_geom){
        DEBUG(2) std::cout<<__FILE__<<"    Line"<<__LINE__<<":    "<<__FUNCTION__<<std::endl;
        m_fstream<<tr_id;
        m_fstream<<";";
        write_o_path(o_path_ptr);
        m_fstream<<";";
        write_complete_path(c_path_ptr);
        m_fstream<<";";
        if (m_geom!=nullptr){
            int binary_size = m_geom->WkbSize();
            unsigned char wkb[binary_size];
            // http://www.gdal.org/ogr__core_8h.html#a36cc1f4d807ba8f6fb8951f3adf251e2
            m_geom->exportToWkb(wkbNDR,wkb);
            // http://www.gdal.org/cpl__string_8h.html
            char *hex_wkb = CPLBinaryToHex(binary_size,wkb);
            m_fstream<<hex_wkb<<std::endl;
            CPLFree(hex_wkb);
            // CPLBinaryToHex()
        } else {
            m_fstream<<std::endl;
        }
    };
    // write a header text to the csv file
    void write_header(const std::string &header="id;m_path;geom"){
        m_fstream<<header<<std::endl;
    };
    // write opitmal path and complete path to the csv file
    void write_opath_cpath(int tr_id,O_Path *o_path_ptr,C_Path *c_path_ptr){
        DEBUG(2) std::cout<<__FILE__<<"    Line"<<__LINE__<<":    "<<__FUNCTION__<<std::endl;
        m_fstream<<tr_id;
        m_fstream<<";";
        write_o_path(o_path_ptr);
        m_fstream<<";";
        write_complete_path(c_path_ptr);
        m_fstream<<std::endl;
    };
    void write_opath_cpath_offset(int tr_id,O_Path *o_path_ptr,C_Path *c_path_ptr){
        DEBUG(2) std::cout<<__FILE__<<"    Line"<<__LINE__<<":    "<<__FUNCTION__<<std::endl;
        m_fstream<<tr_id;
        m_fstream<<";";
        write_o_path(o_path_ptr);
        m_fstream<<";";
        write_offset(o_path_ptr);
        m_fstream<<";";
        write_complete_path(c_path_ptr);
        m_fstream<<std::endl;
    };
private:
    // Write the optimal path
    void write_o_path(O_Path *o_path_ptr)
    {
        DEBUG(2) std::cout<<__FILE__<<"    Line"<<__LINE__<<":    "<<__FUNCTION__<<std::endl;
        if (o_path_ptr==nullptr) {
            DEBUG(2) std::cout<<"Matched path NULL"<<std::endl;
            return;
        };
        int N = o_path_ptr->size();
        for(int i=0; i<N-1; ++i)
        {
            m_fstream<< (*o_path_ptr)[i]->edge->id_attr<<",";
        }
        m_fstream<< (*o_path_ptr)[N-1]->edge->id_attr;
        DEBUG(3) std::cout<<"Finish writing Optimal path"<<std::endl;
    };
    void write_offset(O_Path *o_path_ptr){
        if (o_path_ptr==nullptr) {
            DEBUG(2) std::cout<<"Matched path NULL"<<std::endl;
            return;
        };
        int N = o_path_ptr->size();
        for(int i=0; i<N-1; ++i)
        {
            m_fstream<<(*o_path_ptr)[i]->edge->length - (*o_path_ptr)[i]->offset<<",";
        }
        m_fstream<<(*o_path_ptr)[N-1]->edge->length - (*o_path_ptr)[N-1]->offset;
    };
    // Write the complete path
    void write_complete_path(C_Path *c_path_ptr){
        if (c_path_ptr==nullptr) return;
        int N = c_path_ptr->size();
        for(int i=0; i<N-1; ++i)
        {
            m_fstream<<m_network_ptr->get_edge_id_attr((*c_path_ptr)[i])<<",";
        }
        m_fstream<<m_network_ptr->get_edge_id_attr((*c_path_ptr)[N-1]);
    };
    std::ostream *m_fstream_ptr;
    std::ostream &m_fstream;
    Network *m_network_ptr;
}; // ResultWriter
} //IO
} //MM
#endif // MM_WRITER_HPP
