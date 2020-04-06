/**
 * Fast map matching.
 *
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
  NodeIndex index; /**< Index of a node in the heap */
  double value; /**< Value of a node in the heap */
  /**
   * Compare two nodes in the heap
   * @param rhs a node to be compared
   * @return true if the left operand has a lower value
   */
  bool operator<(const HeapNode &rhs) const {
    if (value == rhs.value) return (index < rhs.index);
    return value < rhs.value;
  };
};

/**
 * %Heap data structure used in the routing query
 */
class Heap{
public:
  /**
   * Push a node into the heap
   * @param index index of node
   * @param value value of the node
   */
  inline void push(NodeIndex index, double value){
    HeapNodeHandle handle = heap.push({index,value});
    handle_data.insert({index,handle});
  };
  /**
   * Pop a node from the heap
   */
  inline void pop(){
    HeapNode &node = heap.top();
    handle_data.erase(node.index);
    heap.pop();
  };
  /**
   * Get a copy of the top node in the heap
   * @return A heap node
   */
  inline HeapNode top(){
    return heap.top();
  };
  /**
   * Check if the heap is empty
   * @return true if the heap is empty
   */
  inline bool empty(){
    return heap.empty();
  };
  /**
   * Get the size of the heap
   * @return the number of nodes in a heap
   */
  inline unsigned int size(){
    return heap.size();
  };
  /**
   * Check if the heap contains a node
   * @param index the node index to be queried
   * @return true if a node is contained
   */
  inline bool contain_node(NodeIndex index){
    return handle_data.find(index)!=handle_data.end();
  };
  /**
   * Decrease a node in the heap
   * @param index node index
   * @param value new value of the node
   */
  inline void decrease_key(NodeIndex index,double value){
    HeapNodeHandle handle = handle_data[index];
    heap.decrease_key(handle,{index,value});
  };

private:
  typedef FibHeap<HeapNode>::FibNode *HeapNodeHandle;
  FibHeap<HeapNode> heap;
  std::unordered_map<NodeIndex,HeapNodeHandle> handle_data;
}; // Heap
}; // NETWORK
}; //FMM

#endif
