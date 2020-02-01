/**
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

#include "fiboheap.h"
#include "fiboqueue.h"
#include <stdlib.h>
#include <assert.h>
#include <queue>

struct lowerI
{
  bool operator()(const int &d1,const int &d2)
  {
    return d1>d2;
  }
};

void fill_heaps(FibHeap<int> &fh,
		std::priority_queue<int,std::vector<int>,lowerI> &pqueue,
		const int &n)
{
  for (int i=0;i<n;i++)
    {
      int r = rand();
      fh.push(r);
      pqueue.push(r);
    }
  assert(fh.size()==n);
  assert(pqueue.size()==n);
}

bool match_heaps(FibHeap<int> &fh,
		 std::priority_queue<int,std::vector<int>,lowerI> &pqueue)
{
  while(!pqueue.empty())
    {
      int i1 = pqueue.top();
      int i2 = fh.top();
      //std::cerr << "i1: " << i1 << " -- i2: " << i2 << std::endl;
      assert(i1 == i2);
      pqueue.pop();
      fh.pop();
    }
  assert(fh.empty());
}

void fill_queues(FibQueue<int> &fh,
		 std::priority_queue<int,std::vector<int>,lowerI> &pqueue,
		 const int &n)
{
  for (int i=0;i<n;i++)
    {
      int r = rand();
      fh.push(r);
      pqueue.push(r);
    }
  assert(fh.size()==n);
  assert(pqueue.size()==n);
}

bool match_queues(FibQueue<int> &fh,
		  std::priority_queue<int,std::vector<int>,lowerI> &pqueue)
{
  while(!pqueue.empty())
    {
      int i1 = pqueue.top();
      int i2 = fh.top();
      //std::cerr << "i1: " << i1 << " -- i2: " << i2 << std::endl;
      assert(i1 == i2);
      pqueue.pop();
      fh.pop();
    }
  assert(fh.empty());
}

int main(int argc, char *argv[])
{
  FibHeap<int> fh;
  int n = 10;
  std::priority_queue<int,std::vector<int>,lowerI> pqueue;

  //srand(time(0));
  
  fill_heaps(fh,pqueue,n);
  assert(match_heaps(fh,pqueue));
      
  fill_heaps(fh,pqueue,n);
  //std::cerr << "top pqueue: " << pqueue.top() << " -- top fh: " << fh.top() << std::endl;
  int r = pqueue.top()-1;
  //std::cerr << "old val: " << pqueue.top() << " -- new val: " << r << std::endl;
  pqueue.pop();
  pqueue.push(r);
  std::make_heap(const_cast<int*>(&pqueue.top()),const_cast<int*>(&pqueue.top())+pqueue.size(),lowerI());
  fh.decrease_key(fh.topNode(),r);
  assert(match_heaps(fh,pqueue));

  FibQueue<int> fq;
  fill_queues(fq,pqueue,n);
  match_queues(fq,pqueue);

  fill_queues(fq,pqueue,n);
  r = rand();
  fq.push(r);
  FibHeap<int>::FibNode *x = fq.findNode(r);
  assert(x!=NULL);
  int nr = r-rand()/2;
  fq.decrease_key(x,nr);
  pqueue.push(nr);
  match_queues(fq,pqueue);
}
