/**
 * Fast map matching.
 *
 * Upperbounded origin destination table 
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */

#ifndef FMM_UBODT_H_
#define FMM_UBODT_H_

#include "network/type.hpp"
#include "mm/transition_graph.hpp"
#include "util/debug.hpp"

namespace FMM {
namespace MM {

/**
 * %Record type of the upper bounded origin destination table
 */
struct Record {
  NETWORK::NodeIndex source; /**< source node*/
  NETWORK::NodeIndex target; /**< target node*/
  NETWORK::NodeIndex first_n; /**< next node visited from source to target */
  NETWORK::NodeIndex prev_n; /**< last node visited before target */
  NETWORK::EdgeIndex next_e; /**< next edge visited from source to target */
  double cost; /**< distance from source to target */
  Record *next; /**< the next record stored in hashtable */
};

/**
 * Upperbounded origin destination table
 */
class UBODT {
 public:
  UBODT(const UBODT &) = delete;
  UBODT &operator=(const UBODT &) = delete;
  /**
   * Constructor of UBODT from bucket number and multiplier
   * @param buckets_arg    Bucket number
   * @param multiplier_arg A multiplier used for querying, recommended to be
   * the number of nodes in the graph.
   */
  UBODT(int buckets_arg, int multiplier_arg);
  ~UBODT();
  /**
   * Look up the row according to a source node and a target node
   * @param  source source node
   * @param  target target node
   * @return  A row in the ubodt if the od pair is found, otherwise nullptr
   * is returned.
   */
  Record *look_up(NETWORK::NodeIndex source, NETWORK::NodeIndex target) const;

  /**
   * Look up a shortest path (SP) containing edges from source to target.
   * In case that SP is not found, empty is returned.
   * @param  source source node
   * @param  target target node
   * @return  a shortest path connecting source to target
   */
  std::vector<NETWORK::EdgeIndex> look_sp_path(NETWORK::NodeIndex source,
      NETWORK::NodeIndex target) const;

  /**
   * Construct the complete path (a vector of edge ID) from an optimal path
   * (a vector of optimal nodes in the transition graph)
   *
   * @param path an optimal path
   * @param edges a vector of edges
   * @param indices the index of each optimal edge in the complete path
   * @return a complete path (topologically connected).
   * If there is a large gap in the optimal
   * path implying complete path cannot be found in UBDOT,
   * an empty path is returned
   */
  C_Path construct_complete_path(const TGOpath &path,
                                 const std::vector<NETWORK::Edge> &edges,
                                 std::vector<int> *indices) const;
  /**
   * Get the upperbound of the UBODT
   * @return upperbound value
   */
  double get_delta() const;
  /**
   * Find the bucket index for an OD pair
   * @param  source origin/source node
   * @param  target destination/target node
   * @return  bucket index
   */
  unsigned int cal_bucket_index(NETWORK::NodeIndex source,
      NETWORK::NodeIndex target) const;

  /**
   *  Insert a record into the hash table
   * @param r a record to be inserted
   */
  void insert(Record *r);

  /**
   * Read UBODT from a file.
   * The format will be infered from the file extension.
   * @param  filename   input file name
   * @param  multiplier A value used for inserting rows to the UBODT
   * @return  A shared pointer to the UBODT data.
   */
  static std::shared_ptr<UBODT> read_ubodt_file(const std::string &filename,
                                                int multiplier = 50000);
  /**
   * Read UBODT from a CSV file
   * @param  filename   input file name
   * @param  multiplier A value used for inserting rows to the UBODT
   * @return  A shared pointer to the UBODT data.
   */
  static std::shared_ptr<UBODT> read_ubodt_csv(const std::string &filename,
                                               int multiplier = 50000);

  /**
   * Read UBODT from a binary file
   * @param  filename   input file name
   * @param  multiplier A value used for inserting rows to the UBODT
   * @return  A shared pointer to the UBODT data.
   */
  static std::shared_ptr<UBODT> read_ubodt_binary(const std::string &filename,
                                                  int multiplier = 50000);
  /**
   * Estimate the number of rows in a file
   * @param  filename input file name
   * @return number of rows estimated
   */
  static long estimate_ubodt_rows(const std::string &filename);
  /**
   * Find a large prime number according to input value
   * @param  value input value
   * @return  a large prime number
   */
  static int find_prime_number(double value);
  constexpr static double LOAD_FACTOR = 2.0; /**< factor measuring the
                                              average number of elements in
                                              a bucket. */
  static const int BUFFER_LINE = 1024; /**< Number of characters to store in
                                            a line */
 private:
  const long long multiplier;   // multiplier to get a unique ID
  const int buckets;   // number of buckets
  double delta = 0.0;
  Record **hashtable;
};
}
}

#endif //FMM_SRC_FMM_FFMM_UBODT_H_
