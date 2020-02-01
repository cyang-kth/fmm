/**
 * Fibonacci Queue
 * Copyright (c) 2014, Emmanuel Benazera beniz@droidnik.fr, All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */

/**
 * This is basically a Fibonacci heap with an added fast store for retrieving
 * nodes, and decrease their key as needed. Useful for search algorithms (e.g.
 * Dijstra, heuristic, ...).
 */

#ifndef FIBOQUEUE_H
#define FIBOQUEUE_H

#include "fiboheap.h"
#include <unordered_map>
#include <algorithm>

template<class T, class Comp = std::less<T>>
class FibQueue : public FibHeap<T, Comp>
{
 public:
  using Heap = FibHeap<T, Comp>;
  using Node = typename Heap::FibNode;
  using KeyNodeIter = typename std::unordered_map<T, Node*>::iterator;

  FibQueue()
    : Heap()
    {
    }

  FibQueue(Comp comp)
      : Heap(comp)
  {
  }

  ~FibQueue()
    {
    }

  void decrease_key(Node *x, T k)
  {
    KeyNodeIter mit = find(x->key);
    fstore.erase(mit);
    fstore.insert({ k, x });
    Heap::decrease_key(x,std::move(k));
  }

  Node* push(T k, void *pl)
  {
    Node *x = Heap::push(std::move(k),pl);
    fstore.insert({ k, x });
    return x;
  }

  Node* push(T k)
  {
    return push(std::move(k),NULL);
  }

  KeyNodeIter find(const T& k)
  {
    KeyNodeIter mit = fstore.find(k);
    return mit;
  }

  int count(const T& k)
  {
      KeyNodeIter mit = fstore.find(k);
      return mit != fstore.end();
  }

  Node* findNode(const T& k)
  {
    KeyNodeIter mit = find(k);
    return mit->second;
  }

  void pop()
  {
    if (Heap::empty())
      return;
    Node *x = Heap::extract_min();
    if (!x)
      return; // should not happen.
    auto range = fstore.equal_range(x->key);
    auto mit = std::find_if(range.first, range.second,
                            [x](const std::pair<T, Node*> &ele){
                                return ele.second == x;
                            }
    );
    if (mit != range.second)
      fstore.erase(mit);
    else std::cerr << "[Error]: key " << x->key << " cannot be found in FiboQueue fast store\n";
    delete x;
  }

  void clear() {
      Heap::clear();
      fstore.clear();
  }

  std::unordered_multimap<T, Node*> fstore;
};

#endif
