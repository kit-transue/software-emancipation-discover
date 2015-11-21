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
#include <genError.h>
#include "constr.h"


int null_dummy::bound(int& s, mode m) {
    Initialize (dummy::bound);
  switch (m) {
  case DO :
    if (s == 0) return 1; else return 0;
  case UNDO :
  case QUERY :
    s = 0;
    return 1;
  }
  Assert(0);      // supposedly can't get here
  return 0;
}


constr_box::constr_box(constr_net* n, constr_node_list* l, node* o) : constr_node(n, o) 
{
  contents.appendlist(l);
  size = POSINF;
  left = n->add_null_dummy();
  middle = n->add_dummy();
  right = n->add_null_dummy();

  // make all nodes of l successors of left and predecessors of right;
  slist_iterator_forw s (*(slist*) l);
  constr_node* c;
  while (c = (constr_node*) s()) {
    left->order(c);
    c->cluster = this;
    c->order((constr_node*) right);
  }
}

constr_box::~constr_box() {
  slist_iterator_forw s (*(slist*) &contents);
  constr_node* c;
  while (c = (constr_node*) s()) {
    left->order(c,UNDO);
    c->order((constr_node*) right,UNDO);
  }

  delete right;
  c = (constr_node*) left; 
  constr_node* cc;
  while (c) {
    cc = c->imm_succ;
    delete c;
    c = cc;
  }
  delete middle;
}

int constr_box::increase(constr_node_list* nl) {
  contents.appendlist(nl);
  slist_iterator_forw sl(*(slist*) nl);
  constr_node* c;
  while (c = (constr_node*) sl()) {
    left->order(c);
    c->order((constr_node*) right);
  }
  return 1;
}

int constr_box::decrease(constr_node_list* nl) {
  slist_iterator_forw sl(*(slist*) nl);
  constr_node* c;
  while (c = (constr_node*) sl()) {
    contents.del(c);
    left->order(c,UNDO);
    c->order((constr_node*) right,UNDO);
  }
  return 1;
}

int constr_box::includes(constr_node_list* nl) {
  slist_iterator_forw sl(*(slist*) nl);
  constr_node* c;
  int r = 1;
  while ((r) && (c = (constr_node*) sl())) {
    r = (r && contents.find(c));  // r &= contents... doesn't work because
                                  // find returns the position number;
  }
  return r;
}

int constr_box::order(constr_node* a, mode m) {
  if (a->is_constr_box()) {
    return right->order((constr_node*)((constr_box*)a)->left, m)
      && constr_node::order(a, m);
  }
  else {
    return right->order(a,m) && constr_node::order(a,m);
  }
}

int constr_box::equal(constr_node* a, mode m) {
  if (a->is_constr_box()) {
    return right->equal((constr_node*)((constr_box*)a)->right, m) && 
      middle->equal((constr_node*)((constr_box*)a)->middle, m) && 
	left->equal((constr_node*)((constr_box*)a)->left, m) && 
	constr_node::equal(a, m);
  }
  else {
    return middle->equal(a,m) && constr_node::equal(a,m);
  }
}

int constr_box::left_neighbor(constr_node* a, mode m) {
  if (a->is_constr_box()) {
    return right->left_neighbor((constr_node*)((constr_box*)a)->left, m)
      && constr_node::left_neighbor(a, m);
  }
  else {
    return constr_node::left_neighbor(a,m);
  }
}

int constr_box::left_margin(mode m) {
  return constr_node::left_margin(m) && left->left_margin(m);
}

int constr_box::right_margin(mode m) {
  return constr_node::right_margin(m) && right->right_margin(m);
}

int constr_box::range(interval& i, mode m) {
    Initialize (constr_box::range);
  switch (m) {
  case DO :
  case UNDO :
    return constr_node::range(i,m) && 
      left->range(i,m) && middle->range(i,m) && right->range(i,m);
  case QUERY :
    return constr_node::range(i,m);
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_box::bound(int& s, mode m) {
    Initialize (constr_box::bound);
  switch (m) {
  case DO : {
    if (s < 0) return 0;  // impossible;
    if (s == size) return 1;  // already DOne;
    if (s == POSINF) return bound(size,UNDO);  // no constraint;

    if (size < POSINF) bound(size, UNDO);
    size = s;
    if (s == 0) {
      // add 'equal' constraint to contents;
      slist_iterator_forw list(*(slist*) &contents);
      constr_node* c;
      while (c = (constr_node*) list()) {
	middle->equal(c,DO);
      }
    }
    else {
      // add a neighborhood of s dummy nodes to the network;
      constr_dummy* last_dummy = (constr_dummy*) left;
      for (int d=1; d<s; d++) {
	constr_dummy* next_dummy = net->add_dummy();
	last_dummy->left_neighbor((constr_node*) next_dummy);
	last_dummy = next_dummy;
      }
      last_dummy->left_neighbor((constr_node*) right);
    }
    return 1;
  }
  case UNDO : {
    if (size == POSINF) return 1;

    if (size == 0) {
      slist_iterator_forw list(*(slist*) &contents);
      constr_node* c;
      while (c = (constr_node*) list()) {
        middle->equal(c,UNDO);
      }
    }
    else if (size == 1) {
      left->left_neighbor((constr_node*) right,UNDO);
    }
    else {
      constr_dummy* c = (constr_dummy*) left->imm_succ;
      constr_dummy* cc;
      while ((c) && (c != (constr_dummy*) right)) {
	cc = (constr_dummy*) c->imm_succ;
	delete c;
	c = cc;
      }
    }
    size = POSINF;
    return 1;
  }
  case QUERY :
    return (s == (s = size));
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

interval* constr_box::range() {
  return constr_node::range();
}

