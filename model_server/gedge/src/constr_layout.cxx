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

extern int APPL_Constraints();
extern dimension H_direction;
extern dimension V_direction;
extern int find_stable_levels(level*);
extern void make_stability(level*);

int maxc_items = 0;  // maximum number of nodes per level;

void
graph::Constraintlayout(int BC_I, int BC_II, int PR, int SORTCC, int CC, int INCR, int EXTENT, int START_LEVEL)
{
    if (this->getheadnode() == NULL)
	return;
    for (node* n = this->getheadnode(); n != 0; n = n->getnextnode())
      if (n->getrgraph() != 0)
	{
        if (n->getncon())
	  {
          n->disconnect();
          n->connect();
        }
          n->getrgraph()->layout();
      }

    int t = (int) time(0);
    if (BC_I == 0) BC_I = layoutparameter[0];
    if (BC_II == 0) BC_II = layoutparameter[1];
    if (PR == 0) PR = layoutparameter[2];
    if (SORTCC == 0) SORTCC = layoutparameter[3];
    if (CC == 0) CC = layoutparameter[4];
    if (INCR == 0) INCR = layoutparameter[5];
    if (EXTENT == 0) EXTENT = layoutparameter[6];
    if (START_LEVEL == 0) START_LEVEL = layoutparameter[7];
    if (debug) {
        printf("layout parameters are:\n");
        printf("BC_I: %d\n", BC_I);
        printf("BC_II: %d\n", BC_II);
        printf("PR: %d\n", PR);
        printf("SORTCC: %d\n", SORTCC);
        printf("CC: %d\n", CC);
        printf("incremental layout: %d\n", INCR);
        printf("extent of instability: %d\n", EXTENT);
        printf("starting from level: %d\n", START_LEVEL);
      }
    if (INCR == 0) 
      MakeImproper(this);
    ConstrTopSort(this);
    MakeProper(this);
    headlevel = NewLevelStruct(this);

    // add constraints for flat edges;
    for (edge* e = getheadedge(); e; e = e->getnextedge()) {
      if (e->getsource()->getlevel() == e->gettarget()->getlevel()) {
	nlist nl;
	nl.append(e->getsource());
	if ((orientation == top_to_bottom) || (orientation == left_to_right))
	  nl.append(e->gettarget());
	else nl.insert(e->gettarget());
	constraints->left_neighbor(LAYOUT_PERM, &nl, DO, LAYOUT_PERM_PRI);
      }
    }

    Constraintbary(BC_I, BC_II, SORTCC, CC, INCR);
    Constraintfinetuning(PR, START_LEVEL);

    this->compute_size();

    // delete the layout constraints;
    constraints->delete_constraints(LAYOUT_TEMP);
    constraints->delete_constraints(LAYOUT_PERM);
    
    // delete old stability constraints and add new;
    if (INCR) make_stability(headlevel);
    
    constraints->consistency();
    t = (int) time(0) - t;
    if (debug) printf("Constraint layout took %d seconds\n", t);

    layout_edges_all();
}


/*
 * Constraintbary
 * the barycenter phase (phase 2 and 3) of the Sugiyama layout
 * modified for the Constraint layout
 */
void 
graph::Constraintbary(int BC_I, int BC_II, int, int CC, int INCR) 
{
  if ( headlevel ) {
    ///////////////////////////////////////////////// begin barycenter phase;

    if (debug) printf("Crossings before Constraint Layout: %d\n", headlevel->crossings(1));

    constraints->consistency();

    // find maximum number of nodes per level;
    level* maxl = headlevel;
    while (maxl) {
      maxc_items = max(maxc_items,maxl->getcitems());
      maxl = maxl->getnextlevel();
    }
    if (maxc_items == 0)
        return;

    // and constrain whole graph;
    interval interv(0,maxc_items);
    constraints->range(LAYOUT_TEMP,H_direction,0,interv,DO,LAYOUT_TEMP_PRI);

    int all_stable = 0;
    
    if (INCR) {
      // incremental layout;
      all_stable = find_stable_levels(headlevel);
    }

    if (! all_stable) {
      
      int count_BC_II = 0;
      for (;;) {
	////////////////////////////////////////////////// beginning phase I;
	int count_BC_I = 0;
	int changed = 1;
	
	level* l = headlevel;
	while ( changed && (count_BC_I++ < BC_I)) {
	  changed = 0;
	  
	  constraints->delete_constraints(H_direction,LAYOUT_TEMP);
	  constraints->consistency();
	  
	  if (l->getcitems() && ! l->isstable()) {
	    changed |= l->constr_order();
	    l->add_constraints();
	  }
	  while ( l->getnextlevel() ) {
	    if (! l->getnextlevel()->isstable()) {
	      int c = l->getnextlevel()->constr_order();
	      c |= l->constr_order_columns();
	      l->getnextlevel()->add_constraints();
	      if (c) c |= l->getnextlevel()->constr_order();
	      changed |= c;
	    }
	    l = l->getnextlevel();
	  };
	  
	  constraints->delete_constraints(H_direction,LAYOUT_TEMP);
	  constraints->consistency();
	  
	  if (l->getcitems() && ! l->isstable()) {
	    changed |= l->constr_order();
	    l->add_constraints();
	  }
	  while ( l->getprevlevel() ) {
	    l = l->getprevlevel();
	    if (! l->isstable()) {
	      int c = l->constr_order();
	      c |= l->constr_order_rows();
	      l->add_constraints();
	      if (c) c |= l->constr_order();
	      changed |= c;
	    }
	  };

	  if (debug) printf("BC I = %d  BC II = %d  crossings = %d\n",
			    count_BC_I, count_BC_II, headlevel->crossings());
	  
	};
	////////////////////////////////////////////////// end phase I;
	
	if ( !changed || (count_BC_II++ >= BC_II) ) break;
	
	////////////////////////////////////////////////// beginning phase II;
	changed = 0;
	
	constraints->delete_constraints(H_direction,LAYOUT_TEMP);
	constraints->consistency();

        if (l->getcitems() && ! l->isstable()) {
	  changed |= l->constr_order();
	  l->add_constraints();
	}
	while ( l->getnextlevel() ) {
	  if (! l->getnextlevel()->isstable()) {
	    int c = l->getnextlevel()->constr_order();
	    c |= l->constr_order_columns();
	    c |= l->constr_rev_columns(CC);
	    l->getnextlevel()->add_constraints();
	    if (c) c |= l->getnextlevel()->constr_order();
	    changed |= c;
	  }
	  l = l->getnextlevel();
	};
	
	constraints->delete_constraints(H_direction,LAYOUT_TEMP);
	constraints->consistency();
	
        if (l->getcitems() && ! l->isstable()) {
	  changed |= l->constr_order();
	  l->add_constraints();
	}
	while ( l->getprevlevel() ) {
	  l = l->getprevlevel();
	  if (! l->isstable()) {
	    int c = l->constr_order();
	    c |= l->constr_order_rows();
	    c |= l->constr_rev_rows(CC);
	    l->add_constraints();
	    if (c) c |= l->constr_order();
	    changed |= c;
	  }
	}; 
	////////////////////////////////////////////////// end phase II;
	
	if ( !changed ) break;
	
	if (debug) printf("BC I = %d  BC II = %d  crossings = %d\n",
			  count_BC_I, count_BC_II, headlevel->crossings());
	
      };
      
      //////////////////////////////////////////// end barycenter phase;
      
    }
    
    if (debug) printf("Crossings after Constraint Layout: %d\n", 
		      headlevel->crossings(1));
  }
}

/*
 * constr_order
 */
int level::constr_order() {
  permutation p(c_items);
  int ret = constr_correction(p);
  if (ret) permute_items(&p);
  return ret;
}

/*
 * constr_order_rows
 */
int level::constr_order_rows() {
  int r = 0;
  int k = interconnection->crossings();
  matrix* m1 = new matrix;
  m1->copy(interconnection);
  permutation* p = interconnection->sort_rows();
  r = constr_correction(*p);
  if (r) {
    m1->permute_rows(p);
    int k1 = m1->crossings();
    if ( k > k1 ) permute_items(p);
  }
  delete m1;
  delete p;
  return r;
}

/*
 * constr_order_columns
 */
int level::constr_order_columns() {
  int r = 0;
  int k = interconnection->crossings();
  matrix* m1 = new matrix;
  m1->copy(interconnection);
  permutation* p = interconnection->sort_columns();
  r = next_level->constr_correction(*p);
  if (r) {
    m1->permute_columns(p);
    int k1 = m1->crossings();
    if ( k > k1 ) next_level->permute_items(p);
  }
  delete m1;
  delete p;
  return r;
}

/*
 * constr_rev_rows
 */
int level::constr_rev_rows(int CC) {
  int changed = 0;
  register float curr_bc = interconnection->barycenter_row(0);
  register float next_bc;
  int delta_k;
  register int r;
  for (r=0; r < c_items - 1; r++) {
    next_bc = interconnection->barycenter_row(r+1);
    if (curr_bc >= next_bc) {
      
      // check if exchanging is allowed;
      constr_node* cn1 = get_item(r)->p->constr[H_direction];
      constr_node* cn2 = get_item(r+1)->p->constr[H_direction];
      if ((cn1) && (cn2) && (cn1->order(cn2,QUERY))) continue;
      
      delta_k = interconnection->crossings_rev_rows(r);
      if ((CC == 2) && (prev_level))
        delta_k += prev_level->interconnection->crossings_rev_columns(r);
      if ( delta_k >= 0) {
        exchange_items(r,r+1);
	changed = 1;
      }
    }
    curr_bc = next_bc;
  };
  if (changed) {
    permutation p(c_items);
    changed = constr_correction(p);
    if (changed) permute_items(&p);
  }
  return changed;
}

/*
 * constr_rev_columns
 */
int level::constr_rev_columns(int CC) {
  int changed = 0;
  register float curr_bc = interconnection->barycenter_column(0);
  register float next_bc;
  register int delta_k;
  register int c;
  for (c=0; c < next_level->c_items - 1; c++) {
    next_bc = interconnection->barycenter_column(c+1);
    if (curr_bc >= next_bc) {

      // check if exchanging is allowed;
      constr_node* cn1 = next_level->get_item(c)->p->constr[H_direction];
      constr_node* cn2 = next_level->get_item(c+1)->p->constr[H_direction];
      if ((cn1) && (cn2) && (cn1->order(cn2,QUERY))) continue;
      
      delta_k = interconnection->crossings_rev_columns(c);
      if ((CC == 2) && (next_level->next_level))
        delta_k = next_level->interconnection->crossings_rev_rows(c);
      if (delta_k >= 0) {
        next_level->exchange_items(c,c+1);
        changed = 1;
      }
    }
    curr_bc = next_bc;
  };
 if (changed) {
    permutation p(next_level->c_items);
    changed = next_level->constr_correction(p);
    if (changed) next_level->permute_items(&p);
  }
  return changed;
}

/*
 * add_constraints
 */
void level::add_constraints() {
  // remember constraints due to ordering of nodes;
  node* n;
  constr_node* cn;
  node* prev_n = 0;
  constr_node* prev_cn = 0;

  for (int i = 0; i < c_items; i++) {
    n = get_item(i)->p;
    cn = n->constr[H_direction];
    if (cn) {
      if (prev_cn) {
	if ((! prev_cn->order(cn,QUERY))  && (! cn->order(prev_cn,QUERY))) {
	  // introduce new order constraint;
	  backgraph->constraints->smaller(LAYOUT_TEMP,H_direction,prev_n,n,DO,
					  LAYOUT_TEMP_PRI);
	  prev_n = n;
	  prev_cn = cn;
	}
      }
      if (cn->get_cluster()) {
	// here range constraint is necessary to move nodes inside the
	// cluster together;
	interval interv;
	cn->range(interv,QUERY);
	if ((interv.includes(i)) && ((interv.low < i) || (interv.high > i))) {
	  // only add new range constraint if it has a chance to 
	  // change something;
	  interv.low = i;
	  interv.high = i;
	  backgraph->constraints->
	    range(LAYOUT_TEMP,H_direction,n,interv,DO,LAYOUT_TEMP_PRI);
	}
      }
    }
  }

  backgraph->constraints->consistency();
}
