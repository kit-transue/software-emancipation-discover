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
/*	Implements ISI layout
 */

#include "all.h"
 

int globallastx;		// records max x position of a leaf, used by ISI
				// layout to determine where to place next leaf


/* declarations of external C functions */

#ifdef __cplusplus
extern "C" { // Allows linkage to non-C++ objects
#endif /* __cplusplus */



#ifdef __cplusplus
}
#endif /* __cplusplus */

int IsiXAveKids(graph*, node*);
int IsiYMaxParents(graph*, node*);
void IsiLayoutX(graph*, node*);
void IsiLayoutY(graph*, node*);

extern nlist* FindTopNodes(graph*);
extern nlist* FindBottomNodes(graph*);
extern nlist* findkids(nlist*);
extern nlist* findparents(nlist*);
extern void MakeImproper(graph*);
extern int  APPL_Redraw();

/*
 * Isilayout
 *	does the ISI layout in the X and Y directions
 */
void 
graph::Isilayout()
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

    // undo possible sugilayout
    MakeImproper(this);
    APPL_Redraw();

    // layout X direction
    globallastx = xbase;
    node *p;
    for (p = this->headnode; p != 0; p=p->getnextnode()) 
	p->setdisplayed(0);
    nlist* predlist = FindTopNodes(this);
    slist_iterator_forw iter2 (*(slist*)predlist);
    while (p=(node*)iter2()) 
	IsiLayoutX(this, p);

    // layout Y direction
    for (p = this->headnode; p != 0; p=p->getnextnode())
	p->setdisplayed(0);
    nlist* succlist = FindBottomNodes(this);
    slist_iterator_forw iter4 (*(slist*)succlist);
    while (p=(node*)iter4()) 
	IsiLayoutY(this, p);

    // move the nodes to the calculated positions
    for (p = this->headnode; p != 0; p=p->getnextnode())  
        p->movenode(p->getx(), p->gety());

    layout_edges_all();

    this->compute_size();
}

/*
 * find0pred
 *	return nlist of the nodes in graph g which don't have any predecessors
 */
nlist* 
find0pred(graph* g)
{
    nlist* predlist = new nlist;
    node* p;
    for (p = g->getheadnode(); p != 0; p=p->getnextnode()) 
	if (p->countpred() == 0)
	    predlist->append(p);
    return(predlist);
}

/*
 * find0succ
 *	return nlist of the nodes in graph g which don't have any successors
 */
nlist* 
find0succ(graph* g)
{
    nlist* succlist = new nlist;
    node* p;
    for (p = g->getheadnode(); p != 0; p=p->getnextnode()) 
	if (p->countsucc() == 0)
	    succlist->append(p);
    return(succlist);
}

/*
 * IsiLayoutX
 *	do ISI layout in X direction
 */
void 
IsiLayoutX(graph* g, node* n)
{
    node* p;
    if (n->getdisplayed() == 0) { 
	int flag;
	flag = 0;
	nlist* nl = new nlist;
	nl->insert(n);
	g->set_nodes_unreached();
	nlist* kidlist = findkids(nl);
	slist_iterator_forw iter2(*(slist*)kidlist);
	while ( p=(node*)iter2() ) 
	    if (p->getdisplayed() == 0)
		flag = 1;
	if (flag == 1) {		// had unlayed out children 
	    // for each outcoming edge
	    slist_iterator_forw iter(*(slist*)kidlist);
	    while ( p=(node*)iter() ) 
	        if (p->getdisplayed() == 0)
		    IsiLayoutX(g, p);
	    n->setx(IsiXAveKids(g,n));
	} else {			// had no unlayed out children
	    n->setx(globallastx + g->getxspace() );
	    if (g->getorientation() == LEFT_TO_RIGHT)
	        globallastx = n->getx() + n->getheight();
	    else
	        globallastx = n->getx() + n->getwidth();
	}
	n->setdisplayed(1);
    }
}

/*
 * IsiLayoutY
 *	do ISI layout in Y direction
 */
void 
IsiLayoutY(graph* g, node* n)
{
    node* p;
    if (n->getdisplayed() == 0) { 
        nlist* nl = new nlist;
	nl->insert(n);
	g->set_nodes_unreached();
	nlist* parentlist = findparents(nl);
	if (parentlist->length() != 0) {  // this node has parents
	    // for each incoming edge
	    slist_iterator_forw iter(*(slist*)parentlist);
	    while ( p=(node*)iter() ) 
		    IsiLayoutY(g, p);
	    if (g->getorientation() == LEFT_TO_RIGHT)
	        n->sety(IsiYMaxParents(g,n) + g->getyspace());
	    else 
	        n->sety(IsiYMaxParents(g,n) + g->getyspace());
	} else
	    n->sety(g->getybase());
	n->setdisplayed(1);
    }
}

/*
 * IsiXAveKids
 *	return average X position of a node's children
 */
int 
IsiXAveKids(graph* g, node* n)
{
    int avg=0;
    int count=0;
    nlist* nl = new nlist;
    nl->insert(n);
    g->set_nodes_unreached();
    if (count != 0) 
	return avg/count;
    else 
	return 0;
}

/*
 * IsiYMaxParents
 *	return maximum Y position of a node's parents
 */
int 
IsiYMaxParents(graph* g, node* n)
{
    int max=0;
    nlist* nl = new nlist;
    nl->insert(n);
    g->set_nodes_unreached();
    slist_iterator_forw print (*(slist*)findparents(nl));
    return max;
}
