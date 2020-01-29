#include "fiboheap/fiboheap.h"

namespace MM {

struct HeapNode
{
  NodeIndex index;
  double dist;
  bool operator<(const HeapNode &rhs) const {
    if (dist == rhs.dist) return (index < rhs.index);
    return dist < rhs.dist;
  }
};

// This class is a wrapper of the fiboheap to facilitate calling of
// insert, pop and update functions.
class Heap {
public:
  inline void push(NodeIndex index, double cost){
    HeapNodeHandle handle = heap.push({index,cost});
    handle_data.insert({index,handle});
  }

  inline void pop(){
    HeapNode &node = heap.top();
    handle_data.erase(node.index);
    heap.pop();
  }

  inline HeapNode &top(){
    return heap.top();
  }

  inline bool empty(){
    return heap.empty();
  }

  inline void decrease_key(NodeIndex index,double cost){
    HeapNodeHandle handle = handle_data[index];
    heap.decrease_key(handle,{index,cost});
  }

private:
  typedef FibHeap<HeapNode>::FibNode *HeapNodeHandle;
  FibHeap<HeapNode> heap;
  std::unordered_map<NodeIndex,HeapNodeHandle> handle_data;
};

};
