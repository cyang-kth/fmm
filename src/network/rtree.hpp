#ifndef FMM_RTREE_HPP /* Currently not used */
#define FMM_RTREE_HPP

#include <boost/geometry/index/rtree.hpp>

#include "core/geometry.hpp"

namespace FMM {
namespace NETWORK{
typedef std::pair<Point, unsigned int> NodeItem; // Item stored in rtree
typedef boost::geometry::index::rtree<
    NodeItem,boost::geometry::index::quadratic<16> > BoostNodeRtree;

class NodeTree {
public:
  unsigned int insert_point(Point &p);
  int query_point_radius(Point &p,double radius,
                         unsigned int *id);
  int getSize();
private:
  BoostNodeRtree rtree;
  unsigned int size=0;
};
};
};


#endif // FMM_RTREE_HPP
