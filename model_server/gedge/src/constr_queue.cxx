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

int constr_queue::insert(constr_descriptor* a, int pri) {
  evaluated = 0;
  return edge_queue::insert((ent) a, pri);
}

constr_descriptor* constr_queue::match
  (char* name, constr_type type, nlist* nl,int i[2], int d, int prio,
   int name_f, int type_f, int nl_f, int i_f,int d_f, int prio_f)
{
  slist_iterator_forw s(*(slist*)this);
  constr_descriptor* cd;
  while (cd = (constr_descriptor*) s()) {
    if (name_f) {
      if (strcmp(cd->description,name) != 0) continue;
    }
    if (type_f) {
      if (cd->type != type) continue;
    }
    if (nl_f) {
      if (! nl->includes(*(cd->nl))) continue;
    }
    if (i_f) {
      if ((cd->i->low != i[0]) || (cd->i->high != i[1])) continue;
    }
    if (d_f) {
      if (cd->distance != d) continue;
    }
    if (prio_f) {
      if (cd->priority != prio) continue;
    }
    break;
  }
  return cd;
}

int constr_queue::del(constr_descriptor* cd, constr_manager* manager) {

  // find slink you want to delete;
  q_link* l = (q_link*) head;
  constr_descriptor* d;
  while (l) {
    d = (constr_descriptor*) l->e;
    if (d->operator==(cd)) break;
    else l = (q_link*) l->next;
  }

  if (l == 0) return 0;

  // remember next item;
  q_link* ll = (q_link*) l->next;

  // delete constr_queue entry;
  if ((d->m == CONSISTENT) || (d->m == WAKEUP)) {
    // delete constraint from constraint network;
    d->m = SLEEP;
    (manager->*d->cf) (d,SLEEP);

    // thereby the constraint network must be updated;
    evaluated = 0;
  }
  delete d;
  slist::del_link((slink*) l);

  if (! evaluated) {

    // set all INCONSISTENT constraints after d SLEEPing
    // because they might become CONSISTENT when d is deleted;
    while (ll) {
      d = (constr_descriptor*) ll->e;
      if (d->m == INCONSISTENT) d->m = SLEEP;
      ll = (q_link*) ll->next;
    }
  }
  return 1;
}

void constr_queue::delete_all(constr_type t, constr_manager* manager, node* n) {

  // find first slink you want to delete;
  q_link* l = (q_link*) head;
  constr_descriptor* d;
  while (l) {
    d = (constr_descriptor*) l->e;
    if ((d->type == t) &&
        ((n == 0) || (n == (node*) d->nl->first()) ||
         (n == (node*) d->nl->second())))
      break;
    else l = (q_link*) l->next;
  }

  while (l) {

    // remember next item;
    q_link* ll = (q_link*) l->next;

    // delete constr_queue entry;
    if ((d->m == CONSISTENT) || (d->m == WAKEUP)) {
      // delete constraint from constraint network;
      d->m = SLEEP;
      (manager->*d->cf) (d,SLEEP);

      // thereby the constraint network must be updated;
      evaluated = 0;
    }
    delete d;
    slist::del_link((slink*) l);

    l = ll;

    // find next slink you want to delete;
    while (l) {
      d = (constr_descriptor*) l->e;
      if ((d->type == t) &&
          ((n == 0) || (n == (node*) d->nl->first()) ||
           (n == (node*) d->nl->second())))
        break;

      // not found,
      // if any CONSISTENT constraint has been deleted,
      // set all subsequent INCONSISTENT constraints SLEEPing
      // because they might become CONSISTENT;
      if ((! evaluated) && (d->m == INCONSISTENT)) d->m = SLEEP;
      l = (q_link*) l->next;
    }
  }
}


void constr_queue::output(ostream &to, int indent) {
  slist_iterator_forw q(*(slist*) this);
  constr_descriptor* cd;
  while (cd = (constr_descriptor*) q()) {
    cd->output(to, indent);
  }
}
