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

extern dimension H_direction;
extern dimension V_direction;

extern void unset_stable(node*);

/*
 * find_stable_levels
 *
 * find all levels beginning from head 
 * if all nodes have a fixed position (range of length 0) mark it as stable
 * and bring the nodes into the right order
 * returns 1 if all levels are stable
 */ 
int find_stable_levels(level* head) {
  graph* bg = head->getbackgraph();
  int INCR = bg->getlayoutpar(5);
  int r = 1;
  for (level* l = head; l; l = l->getnextlevel()) {
    l->setstable(1);
    for(int x=0; x < l->getcitems(); x++) {
      node* n = l->get_item(x)->getnode();
      if (! n->stable) {
	l->setstable(0);
	
	// delete STABILITY constraints of some nodes on this level;
	// this is only necessary for efficiency reasons, the layout
	// certainly would look better if we did not do this;
	for (levelitem* li = l->getfirstitem(); li; li=li->getnextitem()) {
	  if (INCR == 3) continue;           // very stable layout,
	                                     // do not delete any constraints;
	  node* nn = li->getnode();
	  if (INCR == 2) {
	                                     // quite stable layout,
	     if (! nn->isdummy()) continue; 
		  // do not delete constaints from
	                                     // none-dummies;
	  }
	  bg->constraints->delete_constraints(H_direction,STABILITY,nn);
	  unset_stable(nn);
	}
	r = 0;
	break;
      }
    }
    if (l->isstable()) {
      l->constr_order();  
      if (debug) printf("level %d is stable\n", l->getclevel());
    }
  }
  return r;
}

void make_stability(level* head) {
  graph* backgraph = head->getbackgraph();
  node* n; 

  // first delete old stability constraints;
  backgraph->constraints->delete_constraints(STABILITY);
  for (n = backgraph->getheadnode(); n; n=n->getnextnode()) {
    unset_stable(n);
  }

  // add new stability constraints;  
  char s[20];
  node* prev_l_dummy = 0;  // points to previous level;

  for (level* l = head; l; l = l->getnextlevel()) {

    sprintf(s,"level-dummy-%d", l->getclevel());
    node* l_dummy ;

    node* prev_n = 0;
    for (int x = 0; x < l->getcitems(); x++) {
      n = l->get_item(x)->getnode();
      if (n->constr[H_direction]) n->stable = 1; 
      else n->stable = ONLY_STABILITY;
      
      // y coordinate;
      backgraph->constraints->
	equal(STABILITY, V_direction, l_dummy, n, DO, STABILITY_PRI);

      // x coordinate;
      if (prev_n) {
	// there are already nodes on this level;
	backgraph->constraints->
	  smaller(STABILITY, H_direction, prev_n, n, DO, STABILITY_PRI);
      }
      else { 
	// n is the first node on this level;
	if (prev_l_dummy) {
	  backgraph->constraints->
	    smaller(STABILITY, V_direction, prev_l_dummy, l_dummy, 
		    DO, STABILITY_PRI);
	}
      }
      prev_n = n;  // remember previous node on this level;
    }
    prev_l_dummy = l_dummy;  // remember node from previous level;
  }
}

// before calling find_neighbors, all nodes must be initialised 'unreached';
nlist* find_neighbors(node* n, int distance) {
  nlist* nl = new nlist;
  if (distance > 0) {
    if (! n->isreached()) {
      nl->append(n);
      n->setreached(1);
      slist_iterator_forw s(*(slist*) n->getsucclist());
      node* nn;
      while (nn = (node*) s()) {
	nl->appendlist(find_neighbors(nn,distance-1));
      }
      slist_iterator_forw p(*(slist*) n->getpredlist());
      while (nn = (node*) p()) {
	nl->appendlist(find_neighbors(nn,distance-1));
      }
    }    
  }
  return nl;
}

void release_stability(node* n, int distance) {
  graph* bg = n->getbackgraph();
  
  // find neighbor nodes;
  bg->set_nodes_unreached();
  nlist* nl = find_neighbors(n, distance);
  
  // delete their constraints;
  slist_iterator_forw neighbors(*(slist*) nl);
  node* nn;
  while (nn = (node*) neighbors()) {
    bg->constraints->delete_constraints(STABILITY,nn);
    unset_stable(nn);
  
    // make dummy edges of these nodes improper;
    edge* e;
    elist* el = nn->find_edges_from_preds();
    slist_iterator_forw ep(*(slist*) el);
    while (e = (edge*) ep()) {
      if (e->isdummy()) {
	// we have found a dummy edge which must be made improper;
	edge* dummy_e = e;
	node* s = dummy_e->getsource();
	node* t = dummy_e->gettarget();    
	if (t != nn) printf("illegal error A\n");
	for (;;) {
	  if (t->isdummy()) bg->delete_node(t);
	  if (! s->isdummy()) break;
	  
	  t = s;
	  s = s->getpredlist()->first();  // dummy nodes have only one
	}
      }
    }
    
    el = nn->find_edges_to_succs();
    slist_iterator_forw es(*(slist*) el);
    while (e = (edge*) es()) {
      if (e->isdummy()) {
	// we have found a dummy edge which must be made improper;
	edge* dummy_e = e;
	node* s = dummy_e->getsource();
	node* t = dummy_e->gettarget();
	if (s != nn) printf("illegal error B\n");
	for(;;) {
	  if (s->isdummy()) bg->delete_node(s);
	  if (! t->isdummy()) break;
	  
	  s = t;
	  t = t->getsucclist()->first();  // dummy nodes have only one
	    // successor;
	}
      }
    }
  }
}

void unset_stable(node* n) {
  if ((n->stable == ONLY_STABILITY) && (n->constr[H_direction])) {
    delete n->constr[H_direction];
    n->constr[H_direction] = 0;
  }
  n->stable = 0;
}
