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
#include "all.h"
#include "sugilevel.h"
 
#include "globdef.h"
#include "constr.h"


/* declarations of external C functions */

#ifdef __cplusplus
extern "C" { // Allows linkage to non-C++ objects
#endif /* __cplusplus */

extern void EdgeResizeWidget(long, int, int);

#ifdef __cplusplus
}
#endif /* __cplusplus */

extern level* NewLevelStruct(graph*);
extern void MakeImproper(graph*);
extern void MakeProper(graph*);
extern void ConstrTopSort(graph* g);

extern dimension H_direction;
extern dimension V_direction;
extern int find_stable_levels(level*);
extern void make_stability(level*);

class item {
 public:
  item* next;
  item* prev;
  int pos;
  int old_pos;
  int low, high;
  node* n;
  item(node* nn, int p, int old_p, int mn, int mx) 
    {next=0; prev=0; pos=p; old_pos=old_p; n=nn; low=mn; high=mx;}
  ~item();
  void insert_after(item* i)  // insert i after this item;
  {
    i->next=next; i->prev=this;
    if (next) next->prev=i;
    next=i;
  }
  void insert_before(item* i)  // insert i before this item;
  {
    i->next=this; i->prev=prev;
    if (prev) prev->next=i;
    prev=i;
  }
};
  
   item::~item() {
      item *tmp = next;   // in case of loop of items
      next = 0;
      if (tmp)
         delete tmp;      // this call will delete the tail
   }

/*
 * constr_correction
 */
int level::constr_correction(permutation& p) {
  // remember: item_permutation gives the order of the items
  // item_permutation[0] gives the first item;
  // p is proposed reordering of item_permutation:
  // item_permutation[i] <- item_permutation[p[i]],
  // i.e. element from position p[i] moves to position i;

  node* n;
  constr_node* cn;
  constr_node* cni;
  constr_node* cnl;
  int pos;
  int high, low;
  int newpos;
  interval* range;
  int x;
  int stop;

  // initialize list of items;
  newpos = 0;
  pos = p[newpos];
  n = get_item(pos)->p;
  // n should be at position newpos instead of pos;
  cn = n->constr[H_direction];
  if (cn) {
    range = cn->range();
    low = range->low;
    high = range->high;
    if (low > newpos) newpos = low;
    else if (high < newpos) newpos = high;
    // now newpos gives the allowed position of this node;
  }
  else {
    low = NEGINF;
    high = POSINF;
  }

  item* first = new item(n,newpos,pos,low,high);
  item* last = first;

  item* i;
  item* new_i;
  item* il;

  for(x=1; x<c_items; x++) {
    newpos = x;
    pos = p[newpos];
    n = get_item(pos)->p;
    // node n should be at position newpos instead of pos;

    cn = n->constr[H_direction];
    if (cn) {
      range = cn->range();
      low = range->low;
      high = range->high;
      if ((low > newpos) || (cn->left_margin(QUERY))) newpos = low;
      else if ((high < newpos) || (cn->right_margin(QUERY))) newpos = high;
      // now newpos gives the allowed position of this node;
    }
    else {
      low = NEGINF; 
      high = POSINF;
    }

    // find the right position for node n. Besides the position pos,
    // take the order constraints into account;
    if (cn) {
      stop = 0;
      for (i=last; i; i=i->prev) {
	if ((i->pos < newpos) ||
	    ((i->pos == newpos) && (i->high < high))) break;
	cni = i->n->constr[H_direction];
	if ((cni) &&
	    ((cni->order(cn,QUERY)) || (cni->equal(cn,QUERY)))) {
	  stop = 1;
	  break;
	}
      }
      if ((! stop) && (i)) {
	// there might be a node cnl left to the actual position with the
        // constraint cn->order(cnl,QUERY)
        // test all nodes between first and i;
	for (il=first; il != i->next; il=il->next) {
	  cnl = il->n->constr[H_direction];
	  if ((cnl) && (cn->order(cnl,QUERY))) {
	    i = il->prev;
	    break;
	  }
	}
      }
    }
    else {
      // no constraints on node n;
      for (i=last ;i; i=i->prev) {
	if (i->pos <= newpos) break;
      }
    }

    // insert new_i into item list;
    new_i = new item(n,newpos,pos,low,high);
    if (i) {
      i->insert_after(new_i); 
      if (i == last) last = new_i;
    }
    else {
      first->insert_before(new_i);
      first = new_i;
    }
  }
  
  // now first-> ... -> last gives the corrected ordering;
  // compute the permutation;
  x=0;
  for (i=first; i; i=i->next) {
    p[x] = i->old_pos;
    x++;
  }

  // did anything change ?
  int ret = 0;
  for (x=0; x<c_items; x++) {
    ret |= (p[x] != x);
  }

  if (first)
      delete first;

  return ret;
}


