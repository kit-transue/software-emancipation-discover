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
/*	Implements the Sugiyama layout
 */

#include "all.h"
#include "sugilevel.h"
#include "abstraction.h" 
#include "globdef.h"

extern int debug;
/* declarations of external C functions */

#ifdef __cplusplus
extern "C" { // Allows linkage to non-C++ objects
#endif /* __cplusplus */

extern void EdgeResizeWidget(long, int, int);

#ifdef __cplusplus
}
#endif /* __cplusplus */

extern level* NewLevelStruct(graph*);
extern void TopSort(graph*);
extern void MakeImproper(graph*);
extern void MakeProper(graph*);

/*
 * Sugilayout
 * the layout algorithm described by Sugiyama, Tagawana and Toda:
 * "Methodes for visual understanding of hierarchical system structures"
 * IEEE Transactions on Systems, Man, and Cybernetics,
 * SMC - 11(2): 109 - 125 February 1981
 * see edge/doc/layouts for more information
 * Parameters:
 * BC_I    max. number of passes in crossing reduction phase
 * BC_II   max. number of passes in the equal barycenter phase
 * PR      max. number of passes in the finetuning phase
 * SORTCC  sort nodes in crossing reduction and equal barycenter phase only if leads to
 *         less crossings? (param.=0 don't count crossings, param.=1 do)
 * CC      count crossings in 1 or both directions in equal barycenter
 *         phase
 */
void 
graph::Sugilayout(int BC_I, int BC_II, int PR, int SORTCC, int CC, int START_LEVEL) 
{
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

    if (BC_I == 0) BC_I = layoutparameter[0];
    if (BC_II == 0) BC_II = layoutparameter[1];
    if (PR == 0) PR = layoutparameter[2];
    if (SORTCC == 0) SORTCC = layoutparameter[3];
    if (CC == 0) CC = layoutparameter[4];
    if (START_LEVEL == 0) START_LEVEL = layoutparameter[7];
    if (debug) {
	printf("layout parameters are:\n");
	printf("BC_I: %d\n", BC_I);
	printf("BC_II: %d\n", BC_II);
	printf("PR: %d\n", PR);
	printf("SORTCC: %d\n", SORTCC);
	printf("CC: %d\n", CC);
	printf("starting from level: %d\n", START_LEVEL);
    }
    MakeImproper(this);
    TopSort(this);
    MakeProper(this);
    headlevel = NewLevelStruct(this);
    Sugibary(BC_I, BC_II, SORTCC, CC);

    Finetuning(PR, START_LEVEL);

    layout_edges_all();
    this->compute_size();
}

/*
 * Sugibary
 * the barycenter phase (phase 2 and 3) of the Sugiyama layout
 */
void 
graph::Sugibary(int BC_I, int BC_II, int SORTCC, int CC) 
{
  level* l = headlevel;
  if ( l ) {
    ///////////////////////////////////////////////// begin barycenter phase;

    if (debug) printf("Crossings before Sugiyama Layout: %d\n", headlevel->crossings(1));

    l = headlevel;
    int count_BC_II = 0;
    for (;;) {
      ////////////////////////////////////////////////// beginning phase I;
      int count_BC_I = 0;
      int changed = 1;
      while ( changed && (count_BC_I++ < BC_I)) {
	changed = 0;
	while ( l->getnextlevel() ) {
	  int c;
	  if (SORTCC == 0) 
	    c = l->baryorder_columns_quick();
	  else c = l->baryorder_columns();
	  changed = (changed || c);
	  l = l->getnextlevel();
	};
	while ( l->getprevlevel() ) {
	  l = l->getprevlevel();
	  int c;
	  if (SORTCC == 0) 
	    c = l->baryorder_rows_quick();
	  else c = l->baryorder_rows();
	  changed = (changed || c);
	};
      };
      ////////////////////////////////////////////////// end phase I;
      
      if ( !changed || (count_BC_II++ >= BC_II) ) break;
      
      ////////////////////////////////////////////////// beginning phase II;
      changed = 0;
      while ( l->getnextlevel() ) {
	int c;
	if (SORTCC == 0)
	  c = l->baryorder_columns_quick();
	else c = l->baryorder_columns();
	changed = (changed || c);
	c = l->reverse_columns(CC);
	changed = (changed || c);
	l = l->getnextlevel();
      };
      while ( l->getprevlevel() ) {
	l = l->getprevlevel();
	int c;
	if (SORTCC == 0)
	  c = l->baryorder_rows_quick();
	else c = l->baryorder_rows();
	changed = (changed || c);
	c = l->reverse_rows(CC);
	changed = (changed || c);
      }; 
      ////////////////////////////////////////////////// end phase II;
      
      if ( !changed ) break;
      
    };
    
    //////////////////////////////////////////// end barycenter phase;

    if (debug) printf("Crossings after Sugiyama Layout: %d\n", headlevel->crossings(1));
  }
}

/*
 * Finetuning
 * the finetuning phase (phase 4) of the Sugiyama algorithm
 */
void 
graph::Finetuning(int PR, int START_LEVEL) {
  if (headlevel) {
    //////////////////////////////////////////// begin priority layout phase;
    headlevel->init_finetuning();
    level* l = headlevel;
    for (int count_PR=0; count_PR < PR; count_PR++) {
      while (l->getnextlevel()) {
	l = l->getnextlevel();
	l->adjust_down();
      }
      while (l->getprevlevel()) {
	l = l->getprevlevel();
	l->adjust_up();
      }
    };
    // looking for widest level while going down;
    int wide = -1;
    level* widest_level = 0;
    while (l->getnextlevel()) {
      l = l->getnextlevel();
      l->adjust_down(); 
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
      while (l->getprevlevel()) {
	l=l->getprevlevel();
	l->adjust_careful_up();
	edge_vector* p = l->getposition();
	int b = (*p)[l->getcitems()-1] - (*p)[0];
	if (b >= wide) {
	  wide = b;
	  widest_level = l;
	}
      };
      if (START_LEVEL != -1) {
	if (debug) printf("starting from specified level %d\n", START_LEVEL);
        for (l = headlevel; l->getclevel() != START_LEVEL; l=l->getnextlevel());
      } else {
	if (debug) printf("starting from widest level %d\n", widest_level->getclevel());
        l = widest_level;
      }
      // going down from widest_level (or specified level) for the last time;
	while (l->getnextlevel()) {
	  l=l->getnextlevel();
	  l->adjust_careful_down();
	}
    }
    headlevel->last_adjust();
    //////////////////////////////////////////// end priority layout phase;
    
  };
  
  headlevel->set_display();

  for (node* p = headnode; p != 0; p = p->getnextnode())
    p->movenode(p->getx(), p->gety());
  dirty = 0;
}

/*
 * NewLevelStruct
 * creates a new level structure for graph g by inserting all nodes of g
 */
level* 
NewLevelStruct(graph* g) 
{
  level* headlevel;
  headlevel = new level(g);
  g->setheadlevel(headlevel);
  for (node* p = g->getheadnode(); p != 0; p = p->getnextnode())
    if ((p->getsgdummy() == NODUMMY))
      headlevel->add_node(p);
  headlevel->remove_empty_levels();
  g->getheadlevel()->create_ic_matrix();
  return (g->getheadlevel());
}


