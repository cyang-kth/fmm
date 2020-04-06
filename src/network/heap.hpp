/**
 * Definition of heap types
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */


#ifndef FMM_HEAP_HPP
#define FMM_HEAP_HPP

#include "network/type.hpp"
#include "fiboheap/fiboheap.h"

namespace FMM {
namespace NETWORK{
/**
 * Node in the heap structure
 */
struct HeapNode
{
  NodeIndex index;
  double dist;
  bool operator<(const HeapNode &rhs) const {
    if (dist == rhs.dist) return (index < rhs.index);
    return dist < rhs.dist;
  };
};

/**
 * Heap data structure used in the routing query
 */
class Heap{
public:
  inline void push(NodeIndex index, double dist){
    HeapNodeHandle handle = heap.push({index,dist});
    handle_data.insert({index,handle});
  };

  inline void pop(){
    HeapNode &node = heap.top();
    handle_data.erase(node.index);
    heap.pop();
  };

  inline HeapNode top(){
    return heap.top();
  };

  inline bool empty(){
    return heap.empty();
  };

  inline unsigned int size(){
    return heap.size();
  };

  inline bool contain_node(NodeIndex index){
    return handle_data.find(index)!=handle_data.end();
  };

  inline void decrease_key(NodeIndex index,double dist){
    HeapNodeHandle handle = handle_data[index];
    heap.decrease_key(handle,{index,dist});
  };

private:
  typedef FibHeap<HeapNode>::FibNode *HeapNodeHandle;
  FibHeap<HeapNode> heap;
  std::unordered_map<NodeIndex,HeapNodeHandle> handle_data;
}; // Heap
}; // NETWORK
}; //FMM

#endif
