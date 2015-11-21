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

/* History: 05/05/92   S.Spivakovksy  Rewrote TopSort to use
*                                     view (graphical) LEVELS.
*/

#include <msg.h>
#include "all.h"
#include "constr_all.h"
#include "sugilevel.h"
#include "defaultparser.h"
#include "globdef.h"
#include "constr.h"

extern nlist* FindTopNodes(graph* g);
extern nlist* FindBottomNodes(graph* g);

dimension H_direction;
dimension V_direction;

nlist* FindLowMarginNodes(graph* g) 
{
  nlist* l = new nlist;
  for (node* n = g->getheadnode(); n; n = n->getnextnode()) {
    if ((n->constr[V_direction] != 0)) {
      if (n->constr[V_direction]->left_margin(QUERY)) {
	l->append(n);
      }
    }
  }
  return l;
}

nlist* FindHighMarginNodes(graph* g)
{
  nlist* l = new nlist;
  for (node* n = g->getheadnode(); n; n = n->getnextnode()) {
    if ((n->constr[V_direction] != 0)) {
      if (n->constr[V_direction]->right_margin(QUERY)) {
        l->append(n);
      }
    }
  }
  return l;
}

nlist* FindPredNodes(graph* g, node* p)
{
  nlist* l = new nlist;
  for (node* n = g->getheadnode(); n; n = n->getnextnode()) {
    if ((n->constr[V_direction] != 0)) {
      if (g->constraints->smaller(LAYOUT_TEMP,V_direction,n,p,QUERY)) {
        l->append(n);
      }
    }
  }
  return l;
}

nlist* FindSuccNodes(graph* g, node* p)
{
  nlist* l = new nlist;
  for (node* n = g->getheadnode(); n; n = n->getnextnode()) {
    if ((n->constr[V_direction] != 0)) {
      if (g->constraints->smaller(LAYOUT_TEMP,V_direction,p,n,QUERY)) {
        l->append(n);
      }
    }
  }
  return l;
}

nlist* FindEqualNodes(graph* g, node* p)
{
  nlist* l = new nlist;
  for (node* n = g->getheadnode(); n; n = n->getnextnode()) {
    if ((n->constr[V_direction] != 0)) {
      if (g->constraints->equal(LAYOUT_TEMP,V_direction,p,n,QUERY)) {
        l->append(n);
      }
    }
  }
  return l;
}

/*
 * ConstrTopSort
 *      Does a topological sort of the graph
 *      regarding constraints in the corresponding layout V_direction.
 *      Some nodes are flexible as to what level they are assigned to.
 */
void ConstrTopSort(graph* g)
{
  if ((g->getorientation() == top_to_bottom) || 
      (g->getorientation() == bottom_to_top)) {
    V_direction = Y;
    H_direction = X;
  }
  else {
    V_direction = X;
    H_direction = Y;
  }

  // insert for each edge which is visible and not reversed 
  // an order constraint into the constraint network;
  for (edge* e = g->getheadedge(); e; e = e->getnextedge()) {
    if ((! e->isreversed())) {
      g->constraints->smaller(LAYOUT_TEMP, V_direction, e->getsource(),
			      e->gettarget(), DO, LAYOUT_TEMP_PRI);
    }
  }
  
  // let the constraint manager do the rest;
  int timestart = (int) time(0);
  g->constraints->consistency();
  int timeend = (int) time (0);
  if (debug) msg("topsort took $1 seconds") << timeend - timestart << eom;

  // assign nodes the level numbers;

//   Levels already set by constructor which uses VIEW levels;
//   mark nodes as being reached

   for (node* n = g->getheadnode(); n; n = n->getnextnode())
        n->setreached(1);

   g->setmaxlevel(0);
}



/*
   START-LOG-------------------------------------------

   $Log: constr_topsort.cxx  $
   Revision 1.3 2000/07/12 18:08:44EDT ktrans 
   merge from stream_message branch
Revision 1.2.1.3  1992/11/21  21:26:40  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:57:37  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
