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
/* Does a topological sort of the graph, assigning each node to a level
 */


/* History: 03/31/21   S.Spivakovksy  Rewrote TopSortTop to use
*                                     view (graphical) LEVELS.
*/

#include "all.h"
#include "globdef.h"
#include "defaultparser.h"

extern void TopSort(graph*);
extern void TopSortBottom(graph*);
extern void TopSortTop(graph*);
extern nlist* findkids(nlist*);
extern nlist* findparents(nlist*);
extern nlist* find0pred(graph*);
extern nlist* find0succ(graph*);

/*
 *FindTopNodes
 *	Retuns a list of the nodes which should be on level 0
 *	This includes all nodes with 0 predecessors and those for whom the
 *	number of pred - # incoming reversed edges + # outgoing reversed edges
 *	is 0 (because these have 0 predecessors from later in the graph).
 */
nlist* FindTopNodes(graph* g)
{
  nlist* l = new nlist;
  for (node* n = g->getheadnode(); n; n = n->getnextnode()) {
      int pred = n->countpred();
      slist_iterator_forw iterp(*(slist*) n->getpredlist());
      node* p;
      while(p = (node*)iterp())
        if (((g->find_edge(p,n) != NULL) && (g->find_edge(p,n)->isreversed()))) 
	    pred -= 1;
      slist_iterator_forw iters(*(slist*) n->getsucclist());
      node* s;
      while(s = (node*)iters())
       if ((g->find_edge(n,s) != NULL) && (g->find_edge(n,s)->isreversed())) 		    pred += 1;
      if (pred == 0)
        l->append(n);
    }
  return l;
}

/*
 *FindBottomNodes
 *	Retuns a list of the nodes which should be on max. level.
 *	This includes all nodes with 0 succcessors and those for whom the
 *	number of succ + # incoming reversed edges - # outgoing reversed edges
 *	is 0 (because these have 0 successors from earlier in the graph).
 */
nlist* FindBottomNodes(graph* g)
{
  nlist* l = new nlist;
  for (node* n = g->getheadnode(); n; n = n->getnextnode())
      {
      int succ = n->countsucc();
      slist_iterator_forw iterp(*(slist*) n->getpredlist());
      node* p;
      while(p = (node*)iterp())
        if ((g->find_edge(p,n) != NULL) && (g->find_edge(p,n)->isreversed())) 
	    succ += 1;
      slist_iterator_forw iters(*(slist*) n->getsucclist());
      node* s;
       while(s = (node*)iters())
        if ((g->find_edge(n,s) != NULL) && (g->find_edge(n,s)->isreversed())) 
	    succ -= 1;
      if (succ == 0) 
	l->append(n);
    } 
  return l;
}

/*
 * TopSort
 *	Does a topological sort of the graph.  
 *	Some nodes are flexible as to what level they are assigned to.
 *	TopSortTop tends to put nodes near level 0
 *	TopSortBottom tends to put nodes near max level
 */
void TopSort(graph* g)
{
    if (g->gettopsort() == high)
	TopSortTop(g);
    else if (g->gettopsort() == low)
	TopSortBottom(g);
    else
	TopSortTop(g);
}

/*
 *DepthSearchBottom
 *	Does a depth first search starting from the max level
 */
void DepthSearchBottom(nlist* l, int level) {
  slist_iterator_forw iter2(*(slist*) l);
  node* p;
  while (p = (node*)iter2()) {
    if (!p->isreached()) {
      // mark this node as reached
      p->setreached(1);
      if (p->getlevel() < level) {
	p->setlevel(level);
        if (level > p->getbackgraph()->getmaxlevel())
                         p->getbackgraph()->setmaxlevel(level);
        DepthSearchBottom(p->getpredlist(), level + 1);
      }
      p->setreached(0);
    }
    // else cycle detected;
  }
}

/*
 * TopSortBottom
 *	Topological sort starting from max level
 */
void TopSortBottom(graph* g)
{
  node *n;
  for (n = g->getheadnode(); n; n = n->getnextnode())
    n->setlevel(-1);
  // initially mark nodes as being unreached
  g->set_nodes_unreached();
  g->setmaxlevel(0);
  DepthSearchBottom(FindBottomNodes(g), 0);

  // invert positions;
  for (n = g->getheadnode(); n; n = n->getnextnode())
    n->setlevel(g->getmaxlevel() - n->getlevel());
}

/*
 *DepthSearchTop
 *	Does a depth first search starting from level 0
 */
void DepthSearchTop(nlist* l, int level)
{
  slist_iterator_forw iter2(*(slist*) l);
  node* p;
  while (p = (node*)iter2()) {
    if (!p->isreached()) {
      // mark this node as reached
      p->setreached(1);
      if (p->getlevel() < level) {
	p->setlevel(level);
        if (level > p->getbackgraph()->getmaxlevel())
                         p->getbackgraph()->setmaxlevel(level);
	DepthSearchTop(p->getsucclist(), level+1);
      }
      p->setreached(0);
    }
    // else cycle detected;
  }
}

/*
 * TopSortTop
 *	Topological sort starting from level 0
 *      - changed to use node level set in node constructor as
 *        view level (sergey).
 */
void TopSortTop(graph* g)
{
//   Levels already set by constructor which uses VIEW levels;
//   mark nodes as being reached

 for (node* n = g->getheadnode(); n; n = n->getnextnode())
     n->setreached(1);

 g->setmaxlevel(0);

/*   DepthSearchTop(FindTopNodes(g),0);   */
}

/*
 * findkids
 *	returns a list of the kids of nodes in nodelist ll
 */
nlist* findkids(nlist* ll)
{
  nlist* kidlist = new nlist;
  node* p;
  int flag = 0;
  slist_iterator_forw iter (*(slist*)ll);
  while (p=(node*)iter()) {
    if (!p->isreached()) {
      // mark this node as reached
      p->setreached(1);
      edge* e;
      if (p->countsucc() != 0) {
	slist_iterator_forw s_iter (*(slist*)p->find_edges_to_succs());
	while (e=(edge*)s_iter()) {
	  if (!e->isreversed()) {
	    flag = 1;
	    kidlist->append(e->gettarget());
	  }
	}
      }
      if (p->countpred() != 0) {
	slist_iterator_forw p_iter (*(slist*)p->find_edges_from_preds());
	while (e=(edge*)p_iter()) {
	  if (e->isreversed()) {
	    flag = 1;
	    kidlist->append(e->getsource());
	  }
	}
      }
    }
  }
  return kidlist;
}

/*
 * findparents
 *	returns a list of the parents of nodes in nodelist ll
 */
nlist* findparents(nlist* ll)
{
  nlist* parentlist = new nlist;
  node* p;
  int flag = 0;
  slist_iterator_forw iter (*(slist*)ll);
  while (p=(node*)iter()) {
    if (!p->isreached()) {
      // mark this node as reached
      p->setreached(1);
      edge* e;
      if (p->countsucc() != 0) {
	slist_iterator_forw s_iter (*(slist*)p->find_edges_to_succs());
	while (e=(edge*)s_iter()) {
	  if (e->isreversed()) {
	    flag = 1;
	    parentlist->append(e->gettarget());
	  }
	}
      }
      if (p->countpred() != 0) {
	slist_iterator_forw p_iter (*(slist*)p->find_edges_from_preds());
	while (e=(edge*)p_iter()) {
	  if (!e->isreversed()) {
	    flag = 1;
	    parentlist->append(e->getsource());
	  }
	}
      }
    }
  }
  return parentlist;
}

/*
 * IsBackEdge
 *	returns 1 if node s is descendant of node t
 */
int IsBackEdge(graph* g, node* s, node* t)
{
    nlist* RecentSet = new nlist;
    nlist* NewRecentSet;
    node* p;
    RecentSet->append(t);
    g->set_nodes_unreached();

    while (RecentSet->length() != 0) {
        slist_iterator_forw iter2(*(slist*) RecentSet);
        while (p=(node*)iter2()) 
	    if (p == s) 
		return 1;
	NewRecentSet = findkids(RecentSet);
	RecentSet = NewRecentSet;
    }
    return 0;
}
/*
   START-LOG-------------------------------------------

   $Log: sugitopsort.cxx  $
   Revision 1.2 2000/07/07 08:08:52EDT sschmidt 
   Port to SUNpro 5 compiler
Revision 1.2.1.3  1992/11/21  21:26:40  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:57:56  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
