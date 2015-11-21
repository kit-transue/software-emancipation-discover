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
/*	Implements the sugiyama layout dummy nodes
 */
#include "abstraction.h"
#include "all.h"
#include "globdef.h"
#include "sugilevel.h"
#include "constr.h"
extern int debug;
void 
MakeProper(graph* g)
{
    edge* e;
    node* n;
    char* dummykey = new char [20];
#if 0  // unused
    static int dummycount = 0;                   
#endif
    int longlength = 0;
    node* ldn;
    edge* tempedge;
    node* source;
    node* target;

    //int equal;
    nlist column;

    if (g->isproper ())
	return;
    // for all edges
    for (e = g->getheadedge(); e != 0; e = e->getnextedge()) {
      source = e->getsource();
      target = e->gettarget();

        int slev, tlev;
        switch (source->getsgdummy()) {
          case NODUMMY : slev = source->getlevel(); break;
          case INDUMMY : slev = -1; break;
        }

        switch (target->getsgdummy()) {
          case NODUMMY : tlev = target->getlevel(); break;
          case OUTDUMMY : if (g->getheadlevel() == NULL) 
			    tlev = 0;
			  else
			    tlev = g->getmaxlevel() + 1; 
			  break;

        }

	longlength = tlev - slev;
        if (longlength >= 0) 
              e->setreversed(0); 
	else 
	    e->setreversed(1);

        int straight = 0;
        if (source->getstraight() && target->getstraight())
          straight = source->getstraight();

        if (longlength > 1) {
	    n = g->add_node( 1);
	    //if (equal) column.append(n);
	    n->setlevel(++slev); 
	    n->setstraight(straight); 
	    n->setrealsource(e->getsource());
	    n->setrealtarget(e->gettarget());
	    tempedge = g->add_edge
               (e->getsource(), n,  1, 0, DEFAULTEDGENAME);
	    e->copy (tempedge);
	    tempedge->setreversed(0);
	    ldn = n;
            for (int i = 0; i < (longlength - 2); i++) {
	        n = g->add_node( 1);
		// if (equal) column.append(n);
	        n->setlevel(++slev); 
		n->setstraight(straight);
	        n->setrealsource(e->getsource());
	        n->setrealtarget(e->gettarget());
	        tempedge = g->add_edge(ldn, n, 1, 0, DEFAULTEDGENAME);
		e->copy (tempedge);
	        tempedge->setreversed(0);
		ldn = n;
	    }
	    ldn->setrealsource (e->getsource());
	    ldn->setrealtarget (e->gettarget());
	    tempedge = g->add_edge
                (ldn,e->gettarget(), 1, 0, DEFAULTEDGENAME);
	    e->copy (tempedge);
	    tempedge->setreversed(0);
	} else if (longlength < -1) { 			
	    n = g->add_node( 1);
	    // if (equal) column.append(n);
	    n->setlevel(--slev); 
	    n->setstraight(straight);
 	    n->setrealsource(e->getsource());
 	    n->setrealtarget(e->gettarget());
	    // first dummy edge takes over  of the "long" edge
	    tempedge = g->add_edge
                (e->getsource(), n, 1, 0, DEFAULTEDGENAME);
	    e->copy (tempedge);
	    tempedge->setreversed(1);	    ldn = n;
            for (int i = 0; i > (longlength + 2); i--) {
                n = g->add_node( 1);
		// if (equal) column.append(n);
	        n->setlevel(--slev); 
 	        n->setrealsource(e->getsource());
 	        n->setrealtarget(e->gettarget());
	        tempedge = g->add_edge(ldn, n, 1, 0, DEFAULTEDGENAME);
		e->copy (tempedge);
	        tempedge->setreversed(1);
		ldn = n;
	    }
	    ldn->setrealsource (e->getsource());
	    ldn->setrealtarget (e->gettarget());
	    tempedge = g->add_edge(ldn, e->gettarget(), 1, 0, DEFAULTEDGENAME);
	    e->copy (tempedge);
	    tempedge->setreversed(1);
       }
    }
    delete dummykey;
    g->setproper (1);
}


void
MakeAllImproper (graph *g)
{
    edge* e;
    node* n;

    if (! g->isproper ())
	return;
    for (n = g->getheadnode(); n != 0; n = n->getnextnode()) {
        if (g->getlayouttype() == LAYOUT_SUGICONSTR) n->stable = 0;
  	if (n->isdummy(1)) 
          g->delete_node(n);
        else if (n->getrgraph())
		MakeAllImproper (n->getrgraph());
    }

    for (e = g->getheadedge(); e != 0; e = e->getnextedge()) {
    }
    g->setproper (0);
    g->setdirty(1);	
}


/* 
 * MakeImproper
 *	Remove all the dummy nodes and restore the long edges
 */
void 
MakeImproper(graph* g)
{
    edge* e;
    node* n;

/*
    if (! g->isproper ())
	return;
*/
    for (n = g->getheadnode(); n != 0; n = n->getnextnode()) {
        if (g->getlayouttype() == LAYOUT_SUGICONSTR) n->stable = 0;
  	if (n->isdummy(1)) 
          g->delete_node(n);
    }
    for (e = g->getheadedge(); e != 0; e = e->getnextedge()) {
    }
    g->setproper (0);
    g->setdirty (1);
}




