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
// 06/15/92 S. Spivakovsky - fixed memory damage problem in multiple clear() calls.
// 08/06/92 S. Spivakovsky - corrected clear() - added zeroing positions of all equal
//                           nodes to avoid double delete and crash in clear().

#include "all.h"
#include <genError.h>
#include "constr.h"

void neighbor_info::make() {
  constr_node* c;
  constr_node* cc;
  for (c = head; c; c = c->imm_succ) {
    if (c == head) break;  // avoid endless loop among inconsistent neighbors;
    slist_iterator_forw s (*(slist*) &(c->strong_succs));
    while (cc = (constr_node*) s()) {
      if (cc->neighborhood != this) {
	strong_succs.append(cc);
      }
      else {
	strong_in_degree -= 1;
      }
    }
  }
}

 equal_info::~equal_info()
 { 
      if (nr)
         delete nr; 

      if (preds)
          delete preds;

      if (environment){
          delete environment;           
      }
}

void equal_info::make_environment() {
  offset = 0;
  environment = new environment_info();
  int i = 1;
  
  for (equal_info* e = next; e; e = e->next) {
    e->environment = environment;
    e->offset = i++;
  }
  environment->size = i;
}


constr_node::constr_node(constr_net* n, node* o) {
  owner = o;
  cluster = 0;
  net = n;
  if (owner) owner->constr[net->dim] = this;
  next = n->first_constr;
  n->first_constr = this;
  n->size += 1;
  n->consistent = 0;
  imm_pred = 0;
  imm_succ = 0;
  leftmargin = 0;
  rightmargin = 0;
  neighborhood = 0;
  position = 0;
  own_position = true;
  strong_in_degree = 0;
  in_degree = 0;
  
  if (n->neginf) n->neginf->order(this,DO);
  if (n->posinf) order((constr_node*) n->posinf,DO);
}

constr_node::~constr_node() {
  if (owner) owner->constr[net->dim] = 0;
  net->consistent = 0;
  if (this == net->first_constr) 
    net->first_constr = net->first_constr->next;
    
  else {
    constr_node* n = net->first_constr;
    constr_node* l = 0;
    while (n != this) {
      l = n;
      n = n->next;
    }
    l->next = n->next;
  }
  net->linear_order.del(this);
  net->size -= 1;
  if ( neighborhood )
       delete neighborhood;
  neighborhood = 0;

  if (own_position && position)
      delete position;
  position = 0;

  if (cluster) {
    constr_node_list cl(this);
    cluster->decrease(&cl);
  }
  if (imm_pred) imm_pred->left_neighbor(this,UNDO);
  if (imm_succ) left_neighbor(imm_succ,UNDO);
  if (leftmargin) left_margin(UNDO);
  if (rightmargin) right_margin(UNDO);
  constr_node* c;
  while (c = equals.first()) equal(c,UNDO);
  slist_iterator_forw sts (*(slist*) &strong_succs);
  while (c = strong_succs.first()) strong_order(c,UNDO);
  while (c = succs.first()) order(c,UNDO);
  while (c = strong_preds.first()) c->strong_order(this,UNDO);
  while (c = preds.first()) c->order(this,UNDO);
}

void constr_node::make_neighborhood() {
  if (neighborhood) return;

  neighborhood = new neighbor_info();
  neighborhood->strong_in_degree = strong_in_degree;

  constr_node* c = this;
  while (c->imm_pred) {
    if (c == this) break;  // avoid loop among inconsistent neighbors;
    c = c->imm_pred;
    c->neighborhood = neighborhood;
    neighborhood->strong_in_degree += c->strong_in_degree;
  }
  neighborhood->head = c;

  c = this;
  while (c->imm_succ) {
    if (c == this) break;  // avoid loop among inconsistent neighbors;
    c = c->imm_succ;
    c->neighborhood = neighborhood;
    neighborhood->strong_in_degree += c->strong_in_degree;
  }
  neighborhood->tail = c;

  neighborhood->make();
}

int constr_node::make_positions(equal_info* p, equal_info* l, equal_info* r) {
  if (position) {
    // already visited;
    if ((p != 0) && (p != position)) {
      // inconsistency discovered;
      net->consistent = 0;
    } 
    return 0;
  }
  
  int count = 0;
  if (p) {
    position = p;
    own_position = false;
    count += 1;
  }
  else {
    position = new equal_info();  // in constructor nr, preds .. = 0.
    own_position = true;
  }
  if (l) position->prev = l;
  if (r) position->next = r;
  position->equals.append(this);
  position->succs.appendlist(&succs);
  position->in_degree += in_degree;

  slist_iterator_forw e(*(slist*) &equals);
  constr_node* c;
  while(c = (constr_node*) e()) {
    count += c->make_positions(position);
  }
  if ((imm_pred) && (imm_pred->position == 0)) {
    count += imm_pred->make_positions(position->prev,0,position);
    position->prev = imm_pred->position;
  }
  if ((imm_succ) && (imm_succ->position == 0)) {
    count += imm_succ->make_positions(position->next,position,0);
    position->next = imm_succ->position;
  }

  return count;
} 

void constr_node::make_environment() {
  if (position->environment == 0) {
    equal_info* e = position;
    while (e->prev) {
      e = e->prev;
    }
    e->make_environment();
  }
  interval i(&limit);
  i.shift(- position->offset);
  position->environment->range.intersect(&i);
}

int constr_node::order_ranges_forw(int fixed) {
  int consistent = 1;
  position->environment->range.low_fixed = fixed;

  // visit successors including immediate successor;
  slist_iterator_forw s (*(slist*) &(position->succs));
  constr_node* c;
  int low; 
  int d;
  while (c = (constr_node*) s()) {
    d = min(delta(), c->delta());
    low = shift_range(position->environment->range.low,
		      position->offset - c->position->offset + d);
    consistent &= c->position->environment->range.lower_bound(low);
  }

  if (neighborhood->tail == this) {
    // visit strong successors;
    slist_iterator_forw s (*(slist*) &(neighborhood->strong_succs));
    while (c = (constr_node*) s()) {
      d = min(delta(), c->delta());
      low = shift_range(position->environment->range.low,
			position->offset - c->position->offset + d);
      consistent &= c->position->environment->range.lower_bound(low);
    }
  }
  
  return consistent;
}

int constr_node::order_ranges_backw(int fixed) {
  int consistent = 1;

  // visit successors including immediate successor;
  slist_iterator_forw s (*(slist*) &(position->succs));
  constr_node* c;
  int high;
  int d;
  while (c = (constr_node*) s()) {
    d = min(delta(), c->delta());
    high = shift_range(c->position->environment->range.high,
		       c->position->offset - position->offset - d);
    consistent &= position->environment->range.upper_bound(high);
  }

  if (neighborhood->tail == this) {
    // visit strong successors;
    slist_iterator_forw s (*(slist*) &(neighborhood->strong_succs));
    while (c = (constr_node*) s()) {
      d = min(delta(), c->delta());
      high = shift_range(c->position->environment->range.high,
			 c->position->offset - position->offset - d);
      consistent &= position->environment->range.upper_bound(high);
    }
  }

  position->environment->range.high_fixed = fixed;
  return consistent;
}

void constr_node::touch(set* preds) {
  if (position->preds == 0) {
    position->preds = new set(preds);
  }
  else {
    position->preds->op_or(preds);
  }
  

  if ((position->touched >= position->in_degree) &&
      (neighborhood->touched >= neighborhood->strong_in_degree)) {
    // all predecessors of this node are already touched;
    net->linear_order.appendlist(&position->equals);

    // give node a new number;
    position->nr = net->give_var();
    
    
    // compute new predecessor set;
    set new_preds(position->preds);
    new_preds.op_or(position->nr);

    // visit successors including immediate successor;
    slist_iterator_forw s (*(slist*) &(position->succs));
    constr_node* c;
    while (c = (constr_node*) s()) {
      c->position->touched += 1;
      c->touch(&new_preds);
    }
    
    if (neighborhood->tail == this) {
      // visit strong successors;
      slist_iterator_forw s (*(slist*) &(neighborhood->strong_succs));
      while (c = (constr_node*) s()) {
	c->neighborhood->touched += 1;
	c->neighborhood->head->touch(&new_preds);
      }
    }
  }
}

int constr_node::is_evaluated() {
  return ((position->touched >= position->in_degree) &&
	  (neighborhood->touched >= neighborhood->strong_in_degree));
}


// sergey 08/25/92 - modified: zeroed positions in equal nodes + environment

void constr_node::clear() {  
     constr_node* n = net->first_constr;

     if ( neighborhood != 0){
       delete neighborhood; neighborhood = 0;
     }
     if (position != 0){

//     clear common environment to be deleted only once 

       while (n = n->next){      // clear for all but this
           if ( n == this || n->position == NULL )
	       continue;
	   if (n->position != position){    // but envir. can be the same
               if (n->position->environment == this->position->environment)
                   n->position->environment = 0;
           }
           else
               n->position = 0;
       }
       delete position; position = 0;
     }
}

int constr_node::strong_order(constr_node* a, mode m) {
    Initialize (constr_node::strong_order);
  if (a->is_constr_box()) {sorry; return 0;}
  switch (m) {
  case DO :
    net->consistent = 0;
    strong_succs.append(a);
    a->strong_preds.append(this);
    a->strong_in_degree += 1;
    return 1;
  case UNDO :
    net->consistent = 0;
    if (strong_succs.find(a)) a->strong_in_degree -= 1;
    strong_succs.del(a);
    a->strong_preds.del(this);
    return 1;
  case QUERY :
    sorry;
    return 0;
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_node::order(constr_node* a, mode m) {
    Initialize (constr_node::order);
  switch (m) {
  case DO :
    if (a->is_constr_box()) order((constr_node*)((constr_box*)a)->left, DO);
    net->consistent = 0;
    succs.append(a);
    a->preds.append(this);
    a->in_degree += 1;
    return 1;
  case UNDO :
    if (a->is_constr_box()) order((constr_node*)((constr_box*)a)->left, UNDO);
    net->consistent = 0;
    if (succs.find(a)) a->in_degree -= 1;
    succs.del(a);
    a->preds.del(this);
    return 1;
  case QUERY :
    {
    int answer = 1;
    if (a->is_constr_box()) 
      answer &= order((constr_node*)((constr_box*)a)->left, QUERY); 
    answer &= a->position->preds->includes(position->nr) ||
      (position->environment->range.high + position->offset <
       a->position->environment->range.low + a->position->offset);
    return answer;
    }
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_node::equal(constr_node* a, mode m) {
    Initialize (constr_node::equal);
  switch (m) {
  case DO :
    if (a->is_constr_box()) 
      equal((constr_node*)((constr_box*)a)->middle, DO);
    net->consistent = 0;
    equals.append(a);
    a->equals.append(this);
    return 1;
  case UNDO :
    if (a->is_constr_box()) 
      equal((constr_node*)((constr_box*)a)->middle, UNDO);
    net->consistent = 0;
    equals.del(a);
    a->equals.del(this);
    return 1;
  case QUERY :
    {
    int answer = 1;
    if (a->is_constr_box())
      answer &= equal((constr_node*)((constr_box*)a)->middle, QUERY);
    answer &= (position == a->position) || 
      ((position->environment->range.low == 
	position->environment->range.high) &&
       (position->environment->range.high + position->offset ==
	a->position->environment->range.low + a->position->offset) &&
       (a->position->environment->range.low ==
	a->position->environment->range.high));
    return answer;
    }
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_node::left_neighbor(constr_node* a, mode m) {
    Initialize (constr_node::left_neighbor);
  switch (m) {
  case DO :
    net->consistent = 0;
    if ((!imm_succ) && (!a->imm_pred)) {
#if 0   // unused
      constr_node* c = a;
#endif
      imm_succ = a;
      a->imm_pred = this;
      return order(a,m);
    }
    else {
      if ((imm_succ == a) && (a->imm_pred == this)) return 1;
      else return 0;
    }
  case UNDO :
    net->consistent = 0;
    if ((imm_succ == a) && (a->imm_pred == this)) {
      imm_succ = 0;
      a->imm_pred = 0;
      order(a,m);
    }
    return 1;
  case QUERY :
    return ((imm_succ == a) && (a->imm_pred == this));
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_node::left_margin(mode m) {
    Initialize (constr_node::left_margin);
  switch (m) {
  case DO :
    net->consistent = 0;
    leftmargin = 1;
    return equal((constr_node*) net->leftmargin,m);
  case UNDO :
    net->consistent = 0;
    leftmargin = 0;
    return equal((constr_node*) net->leftmargin,m);
  case QUERY :
    return leftmargin && ((constr_node*) net->leftmargin,m);
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_node::right_margin(mode m) {
    Initialize (constr_node::right_margin);
  switch (m) {
  case DO :
    net->consistent = 0;
    rightmargin = 1;
    return equal((constr_node*) net->rightmargin,m);
  case UNDO :
    net->consistent = 0;
    rightmargin = 0;
    return equal((constr_node*) net->rightmargin,m);
  case QUERY :
    return rightmargin && equal((constr_node*) net->rightmargin,m);
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_node::range(interval& i, mode m) {
    Initialize (constr_node::range);
  switch (m) {
  case DO :
    net->consistent = 0;
    return (limit.intersect(&i));
  case UNDO :
    net->consistent = 0;
    limit.low = NEGINF;
    limit.high = POSINF;
    return (!limit.empty());
  case QUERY :
    i = *range();
    return (!i.empty());
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

int constr_node::bound(int& s, mode m) {
    Initialize (constr_node::bound);
  switch (m) {
  case DO :
    if (s == 1) return 1; else return 0;
  case UNDO :
  case QUERY :
    s = 1;
    return 1;
  }
  Assert(0);         // supposedly can't get here
  return 0;
}

constr_node *constr_node::node2box(constr_node_list* nl) {
  if (is_constr_box()) return this;
  constr_box* cb = new constr_box(net,nl,owner);
  cb->owner = owner;
  if (imm_pred) {
    imm_pred->left_neighbor(this,UNDO);
    imm_pred->left_neighbor((constr_node*) cb,DO);
  }
  if (imm_succ) {
    left_neighbor(imm_pred,UNDO);
    cb->left_neighbor(imm_pred,DO);
  }
  cb->range(limit, DO);
  if (leftmargin) {
    cb->left_margin(DO);
    left_margin(UNDO);
  }
  if (rightmargin) {
    cb->right_margin(DO);
    right_margin(UNDO);
  }
  constr_node* c;
  slist_iterator_forw e(*(slist*) &equals);
  while (c = (constr_node*) e()) {
    c->equal((constr_node*) cb,DO);
  }
  slist_iterator_forw ss(*(slist*) &strong_succs);
  while (c = (constr_node*) ss()) {
    cb->strong_order(c,DO);
  }
  slist_iterator_forw s(*(slist*) &succs);
  while (c = (constr_node*) s()) {
    cb->order(c,DO);
  }
  slist_iterator_forw sp(*(slist*) &strong_preds);
  while (c = (constr_node*) sp()) {
    c->strong_order((constr_node*) cb,DO);
  }
  slist_iterator_forw p(*(slist*) &preds);
  while (c = (constr_node*) p()) {
    c->order((constr_node*) cb,DO);
  }
  if (owner) {
    owner->constr[net->dim] = (constr_node*) cb;
    owner = 0;
  }
 return ((constr_node*) cb);
}

int constr_node::is_upper_bound() {
  return position->nr->limits(position->preds);
}


