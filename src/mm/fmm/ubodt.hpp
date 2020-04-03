//
// Created by Can Yang on 2020/3/22.
//

#ifndef MM_SRC_MM_FMM_UBODT_H_
#define MM_SRC_MM_FMM_UBODT_H_

#include "network/type.hpp"
#include "mm/transition_graph.hpp"
#include "util/debug.hpp"

namespace MM {

struct Record {
  NodeIndex source;
  NodeIndex target;
  NodeIndex first_n;   // next_n in the paper
  NodeIndex prev_n;
  EdgeIndex next_e;
  double cost;
  Record *next;   // the next Record used in Hashtable
};

class UBODT {
 public:
  UBODT(const UBODT &) = delete;
  UBODT &operator=(const UBODT &) = delete;
  UBODT(int buckets_arg, int multiplier_arg);
  ~UBODT();

  Record *look_up(NodeIndex source, NodeIndex target) const;

  /**
   *  Return a shortest path (SP) containing edges from source to target.
   *  In case that SP is not found, empty is returned.
   */
  std::vector<EdgeIndex> look_sp_path(NodeIndex source, NodeIndex target) const;

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
  C_Path construct_complete_path(const TGOpath &path,
                                 const std::vector<Edge> &edges,
                                 std::vector<int> *indices) const;
  double get_delta() const;

  unsigned int cal_bucket_index(NodeIndex source, NodeIndex target) const;

  // Insert a Record into the hash table
  void insert(Record *r);

  static std::shared_ptr<UBODT> read_ubodt_file(const std::string &filename,
                                                int multiplier = 50000);
  /**
   * Read ubodt from a csv file, the caller takes the ownership.
   * The ubodt is stored on heap memory.
   */
  static std::shared_ptr<UBODT> read_ubodt_csv(const std::string &filename,
                                               int multiplier = 50000);

  /**
   * Read ubodt from a binary file, the caller takes the ownership.
   */
  static std::shared_ptr<UBODT> read_ubodt_binary(const std::string &filename,
                                                  int multiplier = 50000);
  static long estimate_ubodt_rows(const std::string &filename);
  static int find_prime_number(double value);
  constexpr static double LOAD_FACTOR = 2.0;
  static const int BUFFER_LINE = 1024;
 private:
  const long long multiplier;   // multiplier to get a unique ID
  const int buckets;   // number of buckets
  double delta = 0.0;
  Record **hashtable;
};

}

#endif //MM_SRC_MM_FMM_UBODT_H_
