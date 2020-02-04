/**
 * Content
 * Definition of UBODT, which is a hashtable containing the precomputed
 * shortest path routing results.
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */

#ifndef MM_UBODT_HPP
#define MM_UBODT_HPP
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <fstream>
#include <boost/archive/binary_iarchive.hpp>

#include "types.hpp"
#include "network.hpp"
#include "debug.h"

namespace MM
{

class UBODT
{
public:
  /**
   *  Constructor of UBODT
   *  @param  buckets: the number of buckets in the hashtable
   *  @param  multipler: multiplier to calculate hashcode of an OD pair as
   *  n_o * multiplier + n_d
   */
  UBODT(int buckets_arg,int multiplier_arg) :
    buckets(buckets_arg),multiplier(multiplier_arg) {
    SPDLOG_TRACE("Intialization UBODT with buckets {} multiplier {}",
                 buckets, multiplier);
    hashtable = (Record **) malloc(sizeof(Record*)*buckets);
    for (int i = 0; i < buckets; i++) {
      hashtable[i] = NULL;
    }
    SPDLOG_TRACE("Intialization UBODT finished");
  };

  ~UBODT(){
    /* Clean hashtable */
    SPDLOG_TRACE("Clean UBODT");
    int i;
    for (i=0; i<buckets; ++i) {
      Record* head = hashtable[i];
      Record* curr;
      while ((curr = head) != NULL) {
        head = head->next;
        free(curr);
      }
    }
    // Destory hash table pointer
    free(hashtable);
    SPDLOG_TRACE("Clean UBODT finished");
  };

  Record *look_up(NodeIndex source,NodeIndex target)
  {
    unsigned int h = cal_bucket_index(source,target);
    Record *r = hashtable[h];
    while (r != NULL)
    {
      if (r->source==source && r->target==target)
      {
        return r;
      }
      else
      {
        r=r->next;
      }
    }
    return r;
  };

  /**
   *  Return a shortest path (SP) containing edges from source to target.
   *  In case that SP is not found, empty is returned.
   */
  C_PathIndex look_sp_path(NodeIndex source,NodeIndex target){
    C_PathIndex edges;
    if (source==target) {return edges;}
    Record *r=look_up(source,target);
    // No transition exist from source to target
    if (r==NULL) {return edges;}
    while(r->first_n!=target) {
      edges.push_back(r->next_e);
      r=look_up(r->first_n,target);
    }
    edges.push_back(r->next_e);
    return edges;
  };

  /**
   * Construct the complete path (a vector of edge ID) from an optimal path
   * (a vector of candidates)
   *
   * @param  path, an optimal path
   * @param  edges, a vector of edges
   * @return  a complete path (spatially contiguous).
   * If there is a large gap in the optimal
   * path implying complete path cannot be found in UBDOT,
   * an empty path is returned
   */
  C_Path construct_complete_path(O_Path &path, Network &network){
    T_Path tpath = construct_traversed_path(path,network);
    return tpath.cpath;
  };

  /**
   * Construct a traversed path from the optimal path.
   * It is different with cpath in that the edges traversed between
   * consecutive GPS observations are Recorded.
   * It returns a traversed path including the cpath and the index of
   * matched edge for each point in the GPS trajectory.
   */
  T_Path construct_traversed_path(O_Path &path, Network &network){
    SPDLOG_TRACE("Opath {}", opath2string(path));
    if (path.empty()) return T_Path();
    std::vector<Edge> &edges = network.get_edges();
    int N = path.size();
    // T_Path *edges= new T_Path();
    T_Path t_path;
    int current_idx = -1;
    // if (path[0]==nullptr){
    //   t_path.cpath.push_back(-1);
    // } else {
    //   t_path.cpath.push_back(path[0]->edge->id);
    // }
    // int current_idx = 0;
    // t_path.indices.push_back(current_idx);
    bool prev_connected = false;
    for(int i=0; i<N-1; ++i) {
      SPDLOG_TRACE("Iterate i {}",i);
      Candidate* a = path[i];
      Candidate* b = path[i+1];
      if (a==nullptr && b==nullptr ) {
        SPDLOG_TRACE("a null b null");
        t_path.cpath.push_back(-1);
        ++current_idx;
        SPDLOG_TRACE("Push back current index {}",current_idx);
        t_path.indices.push_back(current_idx);
        if (i==N-2) {
          t_path.cpath.push_back(-1);
          ++current_idx;
          SPDLOG_TRACE("Push back current index {}",current_idx);
          t_path.indices.push_back(current_idx);
        }
        prev_connected = false;
      }
      if (a==nullptr && b!=nullptr ) {
        SPDLOG_TRACE("a null b {}",b->edge->id);
        t_path.cpath.push_back(-1);
        ++current_idx;
        SPDLOG_TRACE("Push back current index {}",current_idx);
        t_path.indices.push_back(current_idx);
        if (i==N-2) {
          t_path.cpath.push_back(b->edge->id);
          ++current_idx;
          SPDLOG_TRACE("Push back current index {}",current_idx);
          t_path.indices.push_back(current_idx);
        }
        prev_connected = false;
      }
      if (a!=nullptr && b==nullptr) {
        SPDLOG_TRACE("a {} b nullptr",a->edge->id);
        if (!prev_connected) {
          t_path.cpath.push_back(a->edge->id);
          ++current_idx;
          SPDLOG_TRACE("Push back current index {}",current_idx);
          t_path.indices.push_back(current_idx);
        }
        if (i==N-2) {
          t_path.cpath.push_back(-1);
          ++current_idx;
          SPDLOG_TRACE("Push back current index {}",current_idx);
          t_path.indices.push_back(current_idx);
        }
        prev_connected = false;
      }
      if (a!=nullptr && b !=nullptr) {
        SPDLOG_TRACE("a {} b {}",a->edge->id,b->edge->id);
        if (!prev_connected) {
          t_path.cpath.push_back(a->edge->id);
          ++current_idx;
          SPDLOG_TRACE("Push back current index {}",current_idx);
          t_path.indices.push_back(current_idx);
        }
        if ((a->edge->id!=b->edge->id) || (a->offset>b->offset)) {
          // segs stores edge index
          auto segs = look_sp_path(a->edge->target,b->edge->source);
          // No transition exist in UBODT
          if (segs.empty() &&  a->edge->target!=b->edge->source) {
            t_path.cpath.push_back(-1);
            ++current_idx;
            prev_connected = false;
          } else {
            for (int e:segs) {
              t_path.cpath.push_back(edges[e].id);
              ++current_idx;
            }
            t_path.cpath.push_back(b->edge->id);
            ++current_idx;
            SPDLOG_TRACE("Push back current index {}",current_idx);
            t_path.indices.push_back(current_idx);
            prev_connected = true;
          }
        } else {
          // b stays on the same edge
          SPDLOG_TRACE("Push back current index {}",current_idx);
          t_path.indices.push_back(current_idx);
          prev_connected = true;
        }
      }
      SPDLOG_TRACE("Iterate i {} done",i);
    }
    return t_path;
  };

  /**
   *  Print statistics of the hashtable to a file
   */
  void print_statictics(const char*filename){
    std::map<int,int> statistics;
    for (int i=0; i<buckets; ++i) {
      int count=0;
      Record *r=hashtable[i];
      while (r!=NULL) {
        r=r->next;
        ++count;
      }
      statistics[count]=statistics[count]+1;
    }
    std::ofstream outputfile(filename);
    if (outputfile.is_open())
    {
      outputfile<<"BucketElements;Counts\n";
      for (std::map<int,int>::iterator it=statistics.begin();
           it!=statistics.end(); ++it)
        outputfile<< it->first << ";" << it->second<<"\n";
      outputfile.close();
    } else {
      SPDLOG_INFO("Unable to write statistics to file");
    }
  };
  double get_delta(){
    return delta;
  };
  inline unsigned int cal_bucket_index(NodeIndex source,NodeIndex target){
    return (source*multiplier+target)%buckets;
  };

  // Insert a Record into the hash table
  void insert(Record *r)
  {
    //int h = (r->source*multiplier+r->target)%buckets ;
    int h = cal_bucket_index(r->source,r->target);
    r->next = hashtable[h];
    hashtable[h]=r;
    if (r->cost > delta) delta = r->cost;
  };

private:
  const long long multiplier;   // multiplier to get a unique ID
  const int buckets;   // number of buckets
  double delta = 0.0;
  //int maxnode=0;
  Record** hashtable;
};

// Constant values used in UBODT.

double LOAD_FACTOR = 2.0;
int BUFFER_LINE = 1024;

/**
 *  Estimate the number of rows in a UBODT file from its size in byte
 *  @Returns the number of rows
 */
int estimate_ubodt_rows(const std::string &filename){
  struct stat stat_buf;
  int rc = stat(filename.c_str(), &stat_buf);
  if (rc==0) {
    int file_bytes = stat_buf.st_size;
    SPDLOG_TRACE("UBODT file size is {} bytes",file_bytes);
    std::string fn_extension = filename.substr(filename.find_last_of(".") + 1);
    std::transform(fn_extension.begin(),
                   fn_extension.end(),
                   fn_extension.begin(),
                   [](unsigned char c){
        return std::tolower(c);
      });
    if (fn_extension == "csv" || fn_extension == "txt") {
      int row_size = 36;
      return file_bytes/row_size;
    } else if (fn_extension == "bin" || fn_extension == "binary") {
      Record r;
      // When exporting to a file using boost binary writer,
      // the padding is removed.
      int row_size = 28;
      return file_bytes/row_size;
    }
  }
  return -1;
};

int find_prime_number(double value){
  std::vector<int> prime_numbers = {
    5003,10039,20029,50047,100669,200003,500000,
    1000039,2000083,5000101,10000103,20000033};
  int N = prime_numbers.size();
  for (int i=0; i<N; ++i) {
    if (value<=prime_numbers[i]) {
      return prime_numbers[i];
    }
  }
  return prime_numbers[N-1];
};
/**
 * Read ubodt from a csv file, the caller takes the ownership.
 * The ubodt is stored on heap memory.
 */
UBODT *read_ubodt_csv(const std::string &filename, int multiplier=50000)
{
  SPDLOG_INFO("Reading UBODT file (CSV format) from {}",filename);
  int rows = estimate_ubodt_rows(filename);
  int buckets = find_prime_number(rows/LOAD_FACTOR);
  SPDLOG_TRACE("Estimated buckets {}",buckets);
  int progress_step = 1000000;
  UBODT *table = new UBODT(buckets, multiplier);
  FILE* stream = fopen(filename.c_str(), "r");
  unsigned int NUM_ROWS = 0;
  char line[BUFFER_LINE];
  if(fgets(line, BUFFER_LINE, stream)) {
    SPDLOG_TRACE("Header line skipped.");
  };
  while (fgets(line, BUFFER_LINE, stream))
  {
    ++NUM_ROWS;
    Record *r =(Record *) malloc(sizeof(Record));
    /* Parse line into a Record */
    sscanf(
      line,"%d;%d;%d;%d;%d;%lf",
      &r->source,
      &r->target,
      &r->first_n,
      &r->prev_n,
      &r->next_e,
      &r->cost
      );
    r->next=NULL;
    table->insert(r);
    if (NUM_ROWS%progress_step==0)
      SPDLOG_INFO("Read rows {}",NUM_ROWS);
  };
  fclose(stream);
  double lf = NUM_ROWS/(double)buckets;
  SPDLOG_TRACE("Estimated load factor #elements/#tablebuckets {}",lf);
  if (lf>10)
    SPDLOG_WARN("Load factor is too large.");
  SPDLOG_INFO("Finish reading UBODT with rows {}",NUM_ROWS);
  return table;
};

/**
 * Read ubodt from a binary file, the caller takes the ownership.
 */
UBODT *read_ubodt_binary(const std::string &filename, int multiplier=50000)
{
  SPDLOG_INFO("Reading UBODT file (binary format) from {}",filename);
  int rows = estimate_ubodt_rows(filename);
  int progress_step = 1000000;
  SPDLOG_TRACE("Estimated rows is {}",rows);
  int buckets = find_prime_number(rows/LOAD_FACTOR);
  UBODT *table = new UBODT(buckets,multiplier);
  unsigned int NUM_ROWS = 0;
  std::ifstream ifs(filename.c_str());
  // Check byte offset
  std::streampos archiveOffset = ifs.tellg();
  std::streampos streamEnd = ifs.seekg(0, std::ios_base::end).tellg();
  ifs.seekg(archiveOffset);
  boost::archive::binary_iarchive ia(ifs);
  while (ifs.tellg() < streamEnd)
  {
    ++NUM_ROWS;
    Record *r =(Record *) malloc(sizeof(Record));
    ia >> r->source;
    ia >> r->target;
    ia >> r->first_n;
    ia >> r->prev_n;
    ia >> r->next_e;
    ia >> r->cost;
    r->next=NULL;
    table->insert(r);
    if (NUM_ROWS%progress_step==0)
      SPDLOG_INFO("Read rows {}", NUM_ROWS);
  }
  ifs.close();
  double lf = NUM_ROWS/(double)buckets;
  SPDLOG_TRACE("Estimated load factor #elements/#tablebuckets {}",lf);
  if (lf>10)
    SPDLOG_WARN("Load factor is too large.");
  SPDLOG_INFO("Finish reading UBODT with rows {}", NUM_ROWS);
  return table;
};

}
#endif /* MM_UBODT_HPP */
