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

extern void reset_values(level* , level* =0);
extern dimension H_direction;
extern dimension V_direction;

/*
 * Constraintfinetuning
 * the finetuning phase (phase 4) of the Constraint layout algorithm
 */
void 
graph::Constraintfinetuning(int PR, int START_LEVEL) {
  if (headlevel) {
    //////////////////////////////////////////// begin priority layout phase;
    constraints->consistency();
    headlevel->constr_init_finetuning();
    reset_values(headlevel);
    level* l = headlevel;
    l->constr_2halfD();
    for (int count_PR=0; count_PR < PR; count_PR++) {
      while (l->getnextlevel()) {
	l = l->getnextlevel();
	l->constr_adjust_down();
	l->constr_2halfD();
      }
      reset_values(headlevel,l);
      while (l->getprevlevel()) {
	l = l->getprevlevel();
	l->constr_adjust_up();
	l->constr_2halfD();
      }
      reset_values(headlevel,l);
    };
    // looking for widest level while going down;
    int wide = -1;
    level* widest_level = 0;
    while (l->getnextlevel()) {
      l = l->getnextlevel();
      l->constr_adjust_down(); 
      l->constr_2halfD();
      edge_vector* p = l->getposition();
      int b = (*p)[l->getcitems()-1] - (*p)[0];
      if (b > wide) {
	wide = b;
	widest_level = l;
      }
    };
    // found if widest_level != 0 , may happen when only one level exists;
    if (l = widest_level) {
      // again looking for widest level while going up;
      reset_values(headlevel,l);
      while (l->getprevlevel()) {
	l=l->getprevlevel();
	l->constr_adjust_careful_up();
	l->constr_2halfD();
	edge_vector* p = l->getposition();
	int b = (*p)[l->getcitems()-1] - (*p)[0];
	if (b >= wide) {
	  wide = b;
	  widest_level = l;
	  reset_values(headlevel,l);
	}
      };
      if (START_LEVEL != -1) {
	if (debug) printf("starting from specified level %d\n", START_LEVEL);
        for (l = headlevel; l->getclevel() != START_LEVEL; l=l->getnextlevel(
));
       } else {
	if (debug) printf("starting from widest level %d\n", widest_level->getclevel());
          l = widest_level;
       }

      // going down from widest_level for the last time;
      if (l = widest_level) {
	while (l->getnextlevel()) {
	  l=l->getnextlevel();
	  l->constr_adjust_careful_down();
	  l->constr_2halfD();
	}
      }
    }
    headlevel->last_adjust();
    headlevel->constr_Z();
    reset_values(headlevel);
    //////////////////////////////////////////// end priority layout phase;
    
  };
  
  headlevel->set_display();

  for (node* p = headnode; p != 0; p = p->getnextnode())
    p->movenode(p->getx(), p->gety());
  for (edge* e = headedge; e != 0; e=e->getnextedge()) 
    e->layout();
  dirty = 0;
}

void
level::constr_adjust_down()
{
  for (int c = c_items - 1; c >= 0; c--) {
    int nr = (*upper_priority)[c];
    constr_node* cn = get_item(nr)->getnode()->constr[H_direction];
    int new_pos;
    if ((cn == 0) || (! cn->get_value(new_pos))) {
      new_pos = prev_level->interconnection->
	barycenter_column(nr,prev_level->position);
    }
    shift(nr, new_pos - (*position)[nr], (*upper_connectivity)[nr],
	  upper_connectivity);
    if (cn) cn->set_value(new_pos);
  };
}

void
level::constr_adjust_up()
{
  for (int c = c_items - 1; c >= 0; c--) {
    int nr = (*lower_priority)[c];
    constr_node* cn = get_item(nr)->getnode()->constr[H_direction];
    int new_pos;
    if ((cn == 0) || (! cn->get_value(new_pos))) {
      new_pos = interconnection->barycenter_row(nr,next_level->position);
    }
    shift(nr, new_pos - (*position)[nr], (*lower_connectivity)[nr],
	  lower_connectivity);
    if (cn) cn->set_value(new_pos);
  };
}

void
level::constr_adjust_careful_down()
{
  for (int c = c_items - 1; c >= 0; c--) {
    int nr = (*upper_priority)[c];
    constr_node* cn  = get_item(nr)->getnode()->constr[H_direction];
    int new_pos;
    if ((cn == 0) || (! cn->get_value(new_pos))) {
      new_pos = prev_level->interconnection->
	barycenter_column(nr,prev_level->position);
    }
    shift(nr, new_pos - (*position)[nr], (*upper_connectivity)[nr]-1,
	  upper_connectivity);
    if (cn) cn->set_value(new_pos);
  };
}


void
level::constr_adjust_careful_up()
{
  for (int c = c_items - 1; c >= 0; c--) {
    int nr = (*lower_priority)[c];
    constr_node* cn = get_item(nr)->getnode()->constr[H_direction];
    int new_pos;
    if ((cn == 0) || (! cn->get_value(new_pos))) {
      new_pos = interconnection->barycenter_row(nr,next_level->position);
    }
    shift(nr, new_pos - (*position)[nr], (*lower_connectivity)[nr]-1,
	  lower_connectivity);
    if (cn) cn->set_value(new_pos);
  };
}

#define DUMMYPRI 1000
#define CONSTRAINTPRI 10000
void
level::constr_init_finetuning()
{
  half_x_width = new edge_vector(c_items);
  depth = new edge_vector(c_items);
  int c;
  for (c=0; c<c_items; c++) {
    levelitem* li = get_item(c);
    
    if ((backgraph->getorientation() == left_to_right) ||
	(backgraph->getorientation() == right_to_left)) {
      (*half_x_width)[c] = li->p->getheight()/2 + li->p->getborderwidth();
      int y = li->p->getwidth() + 2*li->p->getborderwidth();
      if (y_height < y) y_height = y;
    } 
    
    else {        
      // normal top-to-bottom orientation;
      (*half_x_width)[c] = li->p->getwidth()/2 + li->p->getborderwidth();
      int y = li->p->getheight() + 2*li->p->getborderwidth();
      if (y_height < y) y_height = y;
    }
    (*depth)[c] = 0;
  };

  position = new edge_vector(c_items);
  (*position)[0] = (*half_x_width)[0];
  for (c=1; c<c_items; c++) {
    (*position)[c] = (*position)[c-1] + (*half_x_width)[c-1]
      + backgraph->getxspace() + (*half_x_width)[c];
  };
  if (prev_level) {
    upper_connectivity = new edge_vector(c_items);
    for (int i=0; i<c_items; i++) {
      // set upper priority for normal node;
      (*upper_connectivity)[i] =
        prev_level->interconnection->connectivity_column(i);
      // increase priorities for dummy nodes and nodes with layout constraints;
      levelitem* li = get_item(i);
      if (li->p->isdummy()) {
        // dummy node;
        int dummypri;
        if (i < (c_items/2)) dummypri = i; else dummypri = c_items - i;
        (*upper_connectivity)[i] += prev_level->c_items + DUMMYPRI + dummypri;
      };
      if (li->p->constr[H_direction]) {
        // layout constraint;
        int constraintpri;
        if (i < (c_items/2)) constraintpri = i; 
	else constraintpri = c_items - i;
        (*upper_connectivity)[i] += CONSTRAINTPRI + constraintpri;
      };
    };
    upper_priority = new permutation(c_items);
    upper_priority->sort(upper_connectivity);
  };
  if (next_level) {
    lower_connectivity = new edge_vector(c_items);
    for (int i=0; i<c_items; i++) {
      // set lower priority for normal node;
      (*lower_connectivity)[i] = interconnection->connectivity_row(i);
      // increase priorities for dummy nodes and nodes with layout constraints;
      levelitem* li = get_item(i);
      if (li->p->isdummy()) {
        int dummypri;
        if (i < (c_items/2)) dummypri = i; else dummypri = c_items - i;
        (*lower_connectivity)[i] += next_level->c_items + DUMMYPRI + dummypri;
      };
      if (li->p->constr[H_direction]) {
        // layout constraint;
        int constraintpri;
        if (i < (c_items/2)) constraintpri = i; 
	else constraintpri = c_items - i;
        (*lower_connectivity)[i] += 
	  next_level->c_items + CONSTRAINTPRI + constraintpri;
      }
    };
    lower_priority = new permutation(c_items);
    lower_priority->sort(lower_connectivity);
    next_level->constr_init_finetuning();
  }
}

void reset_values(level* ll, level* l) {
  while (ll) {
    for (int i=0; i<ll->getcitems(); i++) {
      constr_node* cn = ll->get_item(i)->getnode()->constr[H_direction];
      if (cn) cn->reset_value();
    }
    ll = ll->getnextlevel();
  }
  
  if (l) {
    for (int i=0; i<l->getcitems(); i++) {
      constr_node* cn = l->get_item(i)->getnode()->constr[H_direction];
      if (cn) cn->set_value((*(l->getposition()))[i]);
    }
  }
}

void level::constr_2halfD() {
  constr_node* cn1 = 0;
  constr_node* cn2 = 0;

  for (int i = 0; i < c_items; i++) {
    cn1 = get_item(i)->p->constr[H_direction];
    if (cn1) {
      // node i has a constraint in X direction;
      int k;
      for (k = i+1; k < c_items; k++) {
	constr_node* cn2 = get_item(k)->p->constr[H_direction];
	if ((! cn2) || (! cn1->equal(cn2,QUERY))) break;
      }
      // nodes i .. k-1 should have the same X position;

      if (i < k-1) {
	// there are at least two nodes;
	// shift them together;
	int middle = ((*position)[i] + (*position)[k-1]) / 2;
	for (int l = i; l < k; l++) {
	  (*position)[l] = middle;
	}
	i = k-1;
      }
    }
  }
}

void level::constr_Z() {
  constr_node* cn1 = 0;
  constr_node* cn2 = 0;
  int pos = 0;
  
  int maxdepth = 0;

  int i;
  for (i = 0; i < c_items; i++) {
    pos = (*position)[i];
    // find nodes with same position;
    int k = i+1;
    while ((k < c_items) && ((*position)[k] == pos)) k++;
    // nodes i .. k-1 have same position;

    int d = k-1 - i;
    if (d > 0) {
      maxdepth = max(maxdepth, d);
      
      // compute depth of the nodes;
      for (int l = i; l < k; l++) {
	// nodes i .. l-1 already have depth 0 .. l-1-i;
	
	// find depth dl for node l;
	cn1 = get_item(l)->p->constr[Z];
	int dl = l-i;   // node l may have at most depth l-i (at this time);

	int m;
	for (m = i; m < l; m++) {
	  cn2 = get_item(m)->p->constr[Z];
	  if ((cn1) && (cn2) && (cn1->order(cn2,QUERY))) {
	    // node l must have at most depth of node m;
	    dl = min(dl,(*depth)[m]);
	  }
	}
	// node l gets depth dl;
	// shift the other nodes into deeper depths;
	for (m = i; m < l; m++) {
	  if ((*depth)[m] >= dl) (*depth)[m] += 1;
	}
	(*depth)[l] = dl;
	
      }
	   
    }
    i = k-1;
  }

  // make additional space for 2 1/2 D layout;
  y_height += maxdepth * backgraph->getzspace();
  // center nodes in Z direction;
  for (i = 0 ; i < c_items; i++) (*depth)[i] -= maxdepth / 2;

  if (next_level) next_level->constr_Z();
}
