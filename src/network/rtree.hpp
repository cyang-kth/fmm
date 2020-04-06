/**
 * Fast map matching.
 *
 * Node rtree
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef FMM_RTREE_HPP /* Currently not used */
#define FMM_RTREE_HPP

#include <boost/geometry/index/rtree.hpp>

#include "core/geometry.hpp"

namespace FMM {
namespace NETWORK{
/**
 * Node Rtree item class
 */
typedef std::pair<FMM::CORE::Point, unsigned int> NodeItem;
/**
 * Boost node Rtree
 */
typedef boost::geometry::index::rtree<
    NodeItem,boost::geometry::index::quadratic<16> > BoostNodeRtree;

/**
 * NodeRtree wrapper for the boost node rtree.
 */
class NodeTree {
public:
  /**
   * Insert a point into the rtree
   * @param p point to be inserted
   * @return index of the point
   */
  unsigned int insert_point(FMM::CORE::Point &p);
  /**
   * Query a point within distance of r to a given point p in the rtree
   * @param p the queried point
   * @param radius search radius
   * @param id the id will be updated as the id of the point found in the
   * rtree
   * @return 0 if a node is found, otherwise -1 is returned
   */
  int query_point_radius(FMM::CORE::Point &p,double radius,
                         unsigned int *id);
  /**
   * Get the number of nodes in the rtree
   * @return number of nodes
   */
  int getSize();
private:
  BoostNodeRtree rtree;
  unsigned int size=0;
};
};
};


#endif // FMM_RTREE_HPP
