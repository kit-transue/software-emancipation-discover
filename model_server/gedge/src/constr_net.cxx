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

constr_net::constr_net() : limit() {
  first_constr = 0;
  size = 0;
  saved_size = 0;

  neginf = 0;
  posinf = 0;
  neginf = new null_dummy(this);
  posinf = new null_dummy(this);
  leftmargin = new null_dummy(this);
  rightmargin = new null_dummy(this);
  leftmargin->left_margin();
  rightmargin->right_margin();

  consistent = 0;
  var = 0;
  debug = 0;

  consistency(DO);
}

constr_net::~constr_net() {
  // first delete constraint boxes;
  constr_node* c = first_constr; 
  constr_node* cc;
  while (c) {
    cc = c->next;
    if (c->is_constr_box()){
        if (c == first_constr)
	    first_constr = 0;
	delete c;
    }
    c = cc;
  }

  // then delete the rest;
  while (first_constr) delete first_constr;
  first_constr = 0;
  delete var;
}

void constr_net::set_dimension(dimension d) {
  dim = d;
}

int constr_net::eval() {
  clear();

  constr_node* c;

  // construct the 'neighborhood' classes;
  for (c = first_constr; c; c = c->next) {
    c->make_neighborhood();
  }

  // construct the 'position' classes;
  for (c = first_constr; c; c = c->next) {
    saved_size += c->make_positions();
  }

  // construct the 'environment' classes;
  for (c = first_constr; c; c = c->next) {
    c->make_environment();
  }
  
  neginf->range(limit, DO);
  leftmargin->range(limit, DO);
  rightmargin->range(limit, DO);
  posinf->range(limit, DO);

  // construct the predecessor sets;
  consistent = 1;
  var = new set_variables(size - saved_size);
  set l(size - saved_size ,EMPTY);
  neginf->touch(&l);
  consistent &= posinf->is_evaluated();

  // compute range limits;
  consistent &= order_ranges();

  if (! consistent) return 0;

  slist_iterator_forw small(*(slist*) &linear_order);
  while ((c = (constr_node*) small()) && (c != (constr_node*) leftmargin)) {
    if ((c != (constr_node*) neginf) &&
	(c->order((constr_node*) leftmargin,QUERY))) 
      return 0;
  }

  if (! consistent) return 0;

  slist_iterator_back great(*(slist*) &linear_order);
  while ((c = (constr_node*) great()) && (c != (constr_node*) rightmargin)) {
    if ((c != (constr_node*) posinf) && (rightmargin->order(c,QUERY)))
      return 0;
  }

  return consistent;
}

set* constr_net::give_var() {
  return var->give();
}

constr_dummy* constr_net::add_dummy() {
  return new constr_dummy(this);
}

null_dummy* constr_net::add_null_dummy() {
  return new null_dummy(this);
}

void constr_net::clear() {
  saved_size = 0;
  for (constr_node* c = first_constr; c; c = c->next) {
    c->clear();
  }
  interval dummy;             // not used in UNDO case at all
  neginf->range(dummy,UNDO);
  neginf->range(limit,DO);
  posinf->range(dummy,UNDO);
  posinf->range(limit,DO);
  leftmargin->range(dummy,UNDO);
  leftmargin->range(limit,DO);
  rightmargin->range(dummy,UNDO);
  rightmargin->range(limit,DO);
  consistent = 0;
  linear_order.clear();
  delete var; var = 0;
}

int constr_net::exist(constr_node** a, node* owner, mode m) {
    Initialize (constr_net::exist);
  switch (m) {
  case DO :
    if (*a == 0) {
      *a = new constr_node(this, owner);
      return 1;
    }
    else return 0;
  case UNDO :
    if (*a != 0) {
      delete *a;
      return 1;
    }
    else return 0;
  case QUERY :
    return (*a != 0);
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_net::strong_order(constr_node* a, constr_node* b, mode m) {
    Initialize (constr_net::strong_order);
    switch (m) {
    case DO :
    case UNDO :
      return (a->strong_order(b,m));
    case QUERY :
      if (consistency(DO))
	return (a->strong_order(b,m));
      else return 0;
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_net::order(constr_node* a, constr_node* b, mode m) {
    Initialize (constr_net::order);
  switch (m) {
  case DO :
  case UNDO:
    return (a->order(b,m));
  case QUERY:
    if (consistency(DO)) 
      return (a->order(b,m));
    else return 0;
 }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_net::equal(constr_node* a, constr_node* b, mode m) {
    Initialize (constr_net::equal);
  switch (m) {
  case DO :
  case UNDO :
    return (a->equal(b,m));
  case QUERY :
    if (consistency(DO)) 
      return (a->equal(b,m));
    else return 0;
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_net::neighbors(constr_node* a, constr_node* b, mode m) {
    Initialize (constr_net::neighbors);
  switch (m) {
  case DO :
  case UNDO :
    return (a->left_neighbor(b,m));
  case QUERY :
    if (consistency(DO))
      return (a->left_neighbor(b,m));
    else return 0;
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_net::left_margin(constr_node* a, mode m) {
    Initialize (constr_net::left_margin);
  switch (m) {
  case DO :
  case UNDO :
    return (a->left_margin(m) && leftmargin->equal(a,m));
  case QUERY :
    if (consistency(DO)) 
      return (a->left_margin(m) && leftmargin->equal(a,m));
    else return 0;
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_net::right_margin(constr_node* a, mode m) {
    Initialize (constr_net::right_margin);
  switch (m) {
  case DO :
  case UNDO :
    return (a->right_margin(m) && rightmargin->equal(a,m));
  case QUERY :
    if (consistency(DO)) 
      return (a->right_margin(m) && rightmargin->equal(a,m));
    else return 0;
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_net::box(constr_node* n, constr_node_list* nl, mode m) {
    Initialize (constr_net::box);
  switch (m) {
  case DO :
    if (n->is_constr_box()) 
      return ((constr_box*)n)->increase(nl);
    else {
      constr_node *c;
      c=n->node2box(nl);
      if ( c != n) n=c;
      return 1;
    }
  case UNDO :
    if (n->is_constr_box())
      return ((constr_box*)n)->decrease(nl);
    else return 0;
  case QUERY :
    if ((consistency(DO)) && (n->is_constr_box()))
      return ((constr_box*)n)->includes(nl);
    else return 0;
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_net::bound(constr_node* n, int& distance, mode m) {
    Initialize (constr_net::bound);
  switch (m) {
  case DO :
  case UNDO :
    return n->bound(distance, m);
  case QUERY :
    if (consistency(DO))
      return n->bound(distance, m);
    else return 0;

  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_net::consistency(mode m) {
    Initialize (constr_net::consistency);
  switch (m) {
  case DO :
  case QUERY :
    if (consistent == 0) return eval(); else return 1;
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_net::range(constr_node* a, interval& i, mode m) {
    Initialize (constr_net::range);
  switch (m) {
  case DO :
  case UNDO :
    return (a->range(i,m));
  case QUERY :
    if (consistency(DO))
      return (a->range(i,m));
    else return 0;
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_net::range(interval& i, mode m) {
    Initialize (constr_net::range);
  switch (m) {
  case DO :
    return limit.intersect(&i);
  case UNDO :
    limit.low = NEGINF;
    limit.high = POSINF;
    return 1;
  case QUERY :
    if (consistency()) {
      i.low = limit.low;
      i.high = limit.high;
      return (!limit.empty());
    }
    else return 0;
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_net::order_ranges() {
  int consistent = 1;
  for (int i=0; i<2; i++) {
    slist_iterator_forw forw(*(slist*) &linear_order);
    constr_node* c;
    while (c = (constr_node*) forw()) {
      consistent &= c->order_ranges_forw(i);
    }
    if (! consistent) return 0;
    
    slist_iterator_back backw(*(slist*) &linear_order);
    while (c = (constr_node*) backw()) {
      consistent &= c->order_ranges_backw(i);
    }
    if (! consistent) return 0;

  }
  return consistent;
}



