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
#include "constr.h"

constr_descriptor::constr_descriptor
  (constr_function cf_, constr_type type_, char* description_, dimension d_,
   nlist* nl_, interval* i_, int distance_, int priority_) {
  type = type_;
  cf = cf_;
  description = description_;
  d = d_;
  nl = new nlist;
  if (nl_) nl->appendlist(nl_); else nl = 0;
  if (i_) i = new interval(i_); else i = 0;
  distance = distance_;
  m = SLEEP;
  priority = priority_;
  net = 0; // this information cannot be provided when called from constr_3D,
           // it must be done by the constraint manager later;
}

constr_node* constr_descriptor::node1() {
  node* n1 = (node*) nl->first();
  if (n1) {
    net->exist(&(n1->constr[d]), n1, DO);
    return n1->constr[d];
  }
  else return 0;
}

constr_node* constr_descriptor::node2() {
  node* n2 = (node*) nl->second();

  if (n2) {
    net->exist(&(n2->constr[d]), n2, DO);
    return n2->constr[d];
  }
  else return 0;
}

constr_node_list* constr_descriptor::nodelist() {
  slist_iterator_forw nn(*(slist*) nl);
  node* n;
  constr_node_list* cl = new constr_node_list;
  while (n = (node*) nn()) {
    net->exist(&(n->constr[d]), n, DO);
    constr_node* c = n->constr[d];
    cl->append(c);
  }
  return cl;
}

constr_descriptor::~constr_descriptor() {
 delete nl;
 delete i;
}

int constr_descriptor::operator==(constr_descriptor* cd) {
  // compare constr_type;
  if (type != cd->type) return 0;
  // compare constr_function;
  if (cf != cd->cf) return 0;
  // compare dimension;
  if ((d != -1) && (cd->d != -1) && (d != cd->d)) return 0;
  // compare nlist;
  if ((nl) && (cd->nl) && (! nl->equal(*(cd->nl)))) return 0;
  // compare interval;
  if ((i) && (cd->i) &&
      ((i->low != cd->i->low) || (i->high != cd->i->high))) return 0;
  // compare distance;
  if ((distance != -1) && (cd->distance != -1) &&
      (distance != cd->distance)) return 0;
  // compare priority; 
  if ((priority != 0) && (cd->priority != 0) && (priority != cd->priority))
    return 0;

  return 1;
}


void constr_descriptor::output(ostream &to, int indent) {
  space (to, indent);
  to << "constraint: {";
  if (m == CONSISTENT) to << " /* active */\n";
  else if (m == INCONSISTENT) to << " /* */\n";

  space (to, indent + 2);
  to << "name: " << description << "\n";

  slist_iterator_forw s(*(slist*) nl);
  if (cf == (constr_function) &constr_manager::cluster) {
    node* n = (node*) s();
    if (n) {
      space (to, indent + 2);
    }
  }

  if (d >= 0) {
    char* c = new char[2];
    sprintf(c,"%c", 'x' + d);
    space (to, indent + 2);
    to << "dimension: " << c << "\n";
  }

  if (nl) {
    space (to, indent + 2);
    to <<  "nodes: {";
    node* n = (node*) s();
    to << "}\n";
  }

  if (i) {
    space (to, indent + 2);
    to << "interval: {" << i->low << "," << i->high << "}\n";
  }

  if (distance >= 0) {
    space (to, indent + 2);
    to << "distance: " << distance << "\n";
  }

  space (to, indent + 2);
  to << "priority: " <<  priority << "\n";

//  if (priority == STABILITY_PRI) {
//    space (to, indent + 2);
//    to << "constrainttype: stability\n";
//  }

  space (to, indent);
  to << "}\n";
}

