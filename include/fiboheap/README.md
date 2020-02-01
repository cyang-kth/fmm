fiboheap
========

C++ STL-like Fibonacci heap and queue for fast priority queues with mutable keys.

This is a header-only implementation of:
* Fibonacci Heap: a fast heap with mutable keys;
  Implementation follows Cormen et al. (2009) "Fibonacci Heaps," in Introduction to Algorithms, 3rd ed. Cambridge: MIT Press, pp. 505-530.
* Fibonacci Queue: a priority queue based on Fibonacci heap. This is basically a Fibonacci heap with an added fast store for retrieving nodes, and decrease their key as needed. Useful for search algorithms (e.g. Dijkstra, heuristic, ...).

The heap and queues are targeted at projects that are relunctant to rely on Boost for a simple Fibonacci heap or queue.

Compile test exe with
```
g++ -g -std=c++11 test_fiboheap.cc -o tf
```
