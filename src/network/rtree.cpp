#include "network/rtree.hpp"

#include <boost/geometry/index/rtree.hpp>

using namespace FMM;
using namespace FMM::CORE;
using namespace FMM::NETWORK;

unsigned int NodeTree::insert_point(Point &p){
  unsigned int id = size;
  rtree.insert(std::make_pair(p,id));
  size+=1;
  return id;
};

// Id is the node returned within distance of radius
int NodeTree::query_point_radius(Point &p,double radius,
                                 unsigned int *id){
  std::vector<NodeItem> returned_values;
  rtree.query(boost::geometry::index::nearest(p,1),
              std::back_inserter(returned_values));
  if (returned_values.empty()) {
    return -1;
  }
  if (boost::geometry::distance(returned_values[0].first,p)<=radius) {
    *id = returned_values[0].second;
    return 0;
  }
  return -1;
};

int NodeTree::getSize(){
  return size;
};
