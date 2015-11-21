/*************************************************************************
* Copyright (c) 2015, Synopsys, Inc.                                     *
* All rights reserved.                                                   *
*                                                                        *
* Redistribution and use in source and binary forms, with or without     *
* modification, are permitted provided that the following conditions are *
* met:                                                                   *
*                                                                        *
* 1. Redistributions of source code must retain the above copyright      *
* notice, this list of conditions and the following disclaimer.          *
*                                                                        *
* 2. Redistributions in binary form must reproduce the above copyright   *
* notice, this list of conditions and the following disclaimer in the    *
* documentation and/or other materials provided with the distribution.   *
*                                                                        *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    *
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      *
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  *
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   *
* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  *
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  *
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  *
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   *
*************************************************************************/
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include "slist.h"
#include "queue.h"


int edge_queue::insert(ent a, int pri) {
  q_link* q = (q_link*) head;
  while ((q) && (q->priority >= pri)) {
    q = (q_link*) q->next;
  }
  
  // q == 0  ||  q->priority < pri;
  if (q == 0) {
    // tail->priority > pri;
    // simulate slist::append(a);
    if (tail) {
      q_link* f = new q_link(a, 0, (q_link*) tail, pri);
      tail->next = (slink*) f;
      tail = (slink*) f;
    } else {
      head = (slink*) new q_link(a, 0, 0, pri);
      tail = head;
    }
    return 0;
  }

  if (q == (q_link*) head) {
    // head->priority < pri;
    // simulate slist::insert(a), head != 0;
    q_link* f = new q_link(a, (q_link*) head, 0, pri);
    head->prev = (slink*) f;
    head = (slink*) f;
    return 0;
  }
  
  // q->priority < pri;
  q_link* new_q = new q_link(a, q, (q_link*) q->prev, pri);
  q->prev->next = (slink*) new_q;
  q->prev = (slink*) new_q;
  return 0;
}

int edge_queue::del(ent a) { return slist::del(a); };




















