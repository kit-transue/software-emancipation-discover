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
/*	Implements node class
 */
#include "all.h"
#include "constr_all.h"
#include "globdef.h"
#include "defaultparser.h"
//#include "defaults.h"
#include <sys/stat.h>
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */

extern int debug;
extern int editnodewidth, editnodeheight;
extern graph *globgraph;

/* declarations of external C functions */

#ifdef __cplusplus
extern "C" { // Allows linkage to non-C++ objects
#endif /* __cplusplus */

extern void error (char *, ...);
extern void warning (char *, ...);
#ifdef __cplusplus
}
#endif /* __cplusplus */
extern int APPL_ActionNode(node*);
extern int node_x_pos (node *, graph *);
extern int node_y_pos (node *, graph *);
extern void MakeImproper (graph *);
extern void MakeProper (graph *);


char *
Translate (char *name)
{
        char *newname;
        int i;
        if (name == NULL)
                return (NULL);
        newname = new char [MAXTEXTSIZE];
        i = 0;
        while (*name) {
                if (*name == '\\') {
                        name++;
                        switch (*name) {
                                case 'n': newname [i] = '\n';
                                          break;
                                default: newname [i] = *name;
                                         break;
                        }
                }
                else
                        newname [i] = *name;
                name++;
                i++;
        }
        newname [i] = '\0';
        return (newname);
}


/* 
 * node constructor, containing everything that can possibly be specified 
 *	in the interface description language
 */
node::node(int t,char* tn,int wd, int h, int bw)
{
  type_name = NULL;
  succlist = new nlist();
  ssucclist = new nlist ();
  predlist = new nlist();
  nextnode = NULL;
  prevnode = NULL;
  backgraph = NULL;
  reached = 0;
  straight = 0;

  level = 0;
  visible = 0;
  dfnum = 0;
  displayed = 0;
  dummy = 0;
  rgraph = NULL;
  sgdummy = 0;
  ncon = 0;
  realsource = NULL;
  realtarget = NULL;
  constr[X] = NULL;  constr[Y] = NULL;  constr[Z] = NULL;
  standardiconpixmap = NULL;
  standardiconheight = standardiconwidth = 0;
  stable = 0;

/*  --------- defaults are not used.
  node *def = NULL;

  if (def != NULL) {
    settype(t);
    if (tn != NULL)
      settypename (tn);
    else
      settypename (def->gettypename ());
    width = wd;
    height = h;
    if (bw != 0)
      borderwidth = bw;
    else
      borderwidth = def->getborderwidth ();
    loc.x = def->getx (); loc.y = def->gety ();
  }
*/

    settype(t);
    if (tn != NULL)
      settypename (tn);
//    else
//      settypename (DEFAULTNODENAME);
    width = wd;
    height = h;
    borderwidth = bw;
    loc.x = 0; loc.y = 0;

}

node::node(char *tname)
{

  type_name = NULL;
  succlist = new nlist();
  ssucclist = new nlist ();
  predlist = new nlist();
  nextnode = NULL;
  prevnode = NULL;
  backgraph = NULL;
  reached = 0;
  straight = 0;
  level = 0;
  visible = 0;
  dfnum = 0;
  displayed = 0;
  dummy = 0;
  rgraph = NULL;
  sgdummy = 0;
  ncon = 0;
  realsource = NULL;
  realtarget = NULL;
  constr[X] = NULL;  constr[Y] = NULL;  constr[Z] = NULL;
  standardiconpixmap = NULL;
  standardiconheight = standardiconwidth = 0;
  stable = 0;

    settype(DEFAULTNODE);
    if (tname != NULL)
      settypename (tname);
//    else
//      settypename (DEFAULTNODENAME);
    width = 0;
    height = 0;
    borderwidth = BORDERWIDTH;
    loc.x = 0; loc.y = 0;

}


/* 
 * node destructor
 */
node::~node()
{
    if (type_name)
        delete type_name;
    if ( succlist )
        delete succlist; 
    if ( ssucclist )
        delete ssucclist; 
    type_name = 0;

    succlist = 0;
    if (predlist)
        delete predlist;
    // node *n;
    //delete constr[X];
    //delete constr[Y];
    //delete constr[Z];
}


/*
 * copy
 */
void
node::copy (node *newn)
{
    newn->settype (type);
//    newn->settypename (type_name);
    type_name = newn->type_name;
    newn->setwidth (width);
    newn->setheight (height);
    newn->setborderwidth (borderwidth);
    newn->setstandardiconwidth (standardiconwidth);
    newn->setdfnum (dfnum);
    newn->setstraight (straight);
}
/* 
 * action
 *      invoke application-provided action routine (if any)
 */
void 
node::action()
{
    APPL_ActionNode(this);
}
/*
 * settypename
 *	sets the typename field of the node 
 */
void 
node::settypename(char* s)
{
  if (s != NULL) {
    if (type_name != NULL)
      delete type_name;
    type_name = new char [MAXTEXTSIZE];
    strcpy (type_name, s);
  }
}
/*
 * setdummy
 */
void 
node::setdummy(int d)
{
    dummy = d;
    width = 0;
    height = 0;
}

/* 
 * addsucc
 *      add node n to the successor list of this node
 */
void 
node::addsucc(node* n)
{
    succlist->append(n);
}

/* 
 * deletesucc
 *      delete from the succecessor list of this node, nodes which have node
 *      n as the target
 */
void 
node::deletesucc(node* n)
{
    succlist->del(n);
}

/* 
 * addpred
 *      add node n to the predecessor list of this node
 */
void 
node::addpred(node* n)
{
    predlist->append(n);
}

/* 
 * deletepred
 *      delete from the predecessor list of this node, nodes which have node
 *      n as the source
 */
void 
node::deletepred(node* n)
{
    predlist->del(n);
}

/* 
 * countsucc
 *      count of successors (this will include count of dummy nodes if
 *      they are present.
 */
int 
node::countsucc()
{
    int n = 0;
    if (succlist != NULL) {
      slist_iterator_forw count(*(slist*)succlist);
      while ( count() ) n++;
    }
    return n;
}

/* 
 * countpred
 *      count of predecessors (this will include count of dummy nodes if
 *      they are present.
 */
int 
node::countpred()
{
    int n = 0;
    if (predlist != NULL) {
      slist_iterator_forw count(*(slist*)predlist);
      while ( count() ) n++;
    }
    return n;
}

/* 
 * movenode
 *      move the node's window to the specified position
 */
void 
node::movenode(int x, int y)
{
    if (backgraph->getorientation() == bottom_to_top)
	y = backgraph->getheight() - backgraph->getybase() - y;
    if (backgraph->getorientation() == right_to_left)
	y = backgraph->getwidth() - backgraph->getxbase() - y;
		
    setx(x);
    sety(y);
}


/*
 * find_edges_to_succs
 *      return an elist of this node's sucessors
 *      for successors in other subgraphs the main edge is returned,
 *      i.e. OUTDUMMY -> targetnode or OUTDUMMY -> INDUMMY
 *
 *      however, if *this* is a sugiyama layout dummy, only the edges
 *      to the *direct* successors are returned (not necessarily the main edges).
 */
elist* 
node::find_edges_to_succs()
{
    elist* result = new elist;
    node* c;
    result->clean();
    slist_iterator_forw nextsucc(*(slist*) this->succlist);
    while (c = (node*) nextsucc()) {
        if ( ! this->isdummy() )
             result->append(this->getbackgraph()->find_edge(this, c));
        else {
             elist* ee = c->find_edges_to_succs();
             result->appendlist(ee);
             ee->clean();
        }
    }
    return result;
}

/*
 * find_edges_from_preds
 *	return an elist of this node's predecessors
 *      for predecesors in other subgraphs, the main edge is returned,
 *
 *      if *this* is a sugiyama dummy node, however, the edges from the
 *      *direct* predecessors are returned (not necessarily the main edges).
 */
elist* 
node::find_edges_from_preds()
{
    elist* result = new elist;
    node* c;
    result->clean();
    slist_iterator_forw nextpred(*(slist*) this->predlist);
    while (c = (node*) nextpred()) {
      if ( !this->isdummy()) 
        result->append(this->getbackgraph()->find_edge(c, this));
      else {
        elist* ee = c->find_edges_from_preds();
        result->appendlist(ee);
        ee->clean();
      }
    }

    return result;
}

/*
 *      for IN-/OUTDUMMY nodes only: return the *sugiyama dummy node*
 *      inside an abstraction (if any) that "propers" the connection between this
 *      and this->connected_node(). If no such dummy node exists,
 *      connected_node() is returned
 */

/* --- NOT USED 
node* 
node::connected_dummy()
{
    nlist* cc;
    slist_iterator_forw cncn(*(slist*) cc);
    node* con;
    node* result = (node*) cncn();
    while ((result != 0) && (result->isdummy() == 0)) {
      con = result;
      result = (node*) cncn();
    }
    if (result != 0) return result; else return con;
}

*/

/*
 * connect
 *     for abstraction nodes only: build connections to the rest of the graph
 *     according to the edges crossing the subgraph boundaries.
 */
void 
node::connect()
{
    if (this->getrgraph() != 0) {
      for (node *c = this->getrgraph()->getheadnode(); c != 0; c = c->getnextnode()) {
        if (c->getrgraph())
                c->connect();
        slist_iterator_forw nextpred (*(slist *) c->getpredlist());
        slist_iterator_forw nextsucc(*(slist *) c->getsucclist());
      }
      this->ncon = 0;
    }
}


/*
 * disconnect
 *      delete all edges having this as source or target
 */
void 
node::disconnect()
{
    node* c;

    if (this->getrgraph() != 0)
      for (c = this->getrgraph()->getheadnode(); c != 0; c = c->getnextnode())
        if (c->getrgraph() != 0) c->disconnect();

    elist* edges = this->find_edges_from_preds();
    elist* ee = this->find_edges_to_succs();

    edges->appendlist(ee);

//    ee->clean();
    edges->clean();
    delete edges;
    delete ee;
}

/* 
 * getwidth 
 *	return the width of the node (even if it is a zoomed abstraction)
 */
int 
node::getwidth()
{
    if ((this->getrgraph() == 0) || (this->getstatus() == 0))
      return this->width;
    else
      return this->getrgraph()->getwidth();
}

/*
 * setwidth
 *	sets the width of the node 
 */
void 
node::setwidth(int _width)
{
    this->width = _width;
}

/*
 * calcwidth
 *	calculates the width of the node 
 */
int
node::calcwidth()
{
    int _width;
    if (dummy == 1)
	_width = 0;
        else 
	_width = this->width;
    return _width;
}

/*
 * getheight
 *	return the height of the node (even if it is a zoomed abstraction)
 */
int 
node::getheight()
{
    if ((this->getrgraph() == 0) || (this->getstatus() == 0))
      return this->height;
    else
      return this->getrgraph()->getheight();
}

/*
 * setheight
 *	sets the height of the node 
 */
void 
node::setheight(int _height)
{
	this->height = _height;
}

/*
 * calcheight
 *	calculates the height of the node 
 */
int
node::calcheight()
{
    int _height;
    if (dummy == 1)
	_height = 0;
        else
	_height = this->height;
    return _height;
}
/* 
 * FinishConstructor
*/
int node::finishconstructor ()
{
  return 1;
}

/*
   START-LOG-------------------------------------------

   $Log: node.cxx  $
   Revision 1.6 2000/07/10 23:02:53EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.11  1993/04/29  17:56:12  sergey
More clean up in ~node(). Part of bug #3463.

Revision 1.2.1.10  1993/04/27  17:40:46  sergey
Cleaned up node::disconnect ( memory leaks ). Part of bug #3463.

Revision 1.2.1.9  1993/04/26  22:03:37  sergey
Added deletion of tmp allocated memory in disconnect(). Part of bug #3463.

Revision 1.2.1.8  1993/04/26  15:53:30  sergey
More constructor clean up. Part of bug #3463.

Revision 1.2.1.7  1993/04/25  22:49:59  sergey
Left tname NULL in node constructor. Part of bug #3463.

Revision 1.2.1.6  1993/04/22  21:06:21  sergey
Removed default node code to simplify and to prevent memory leak. Part of bug #3463.

Revision 1.2.1.5  1993/04/19  18:05:37  sergey
>
>
Minor corrections tp clean the warnings. Part of bug #3420.

Revision 1.2.1.4  1992/11/21  21:26:40  builder
typesafe casts.

Revision 1.2.1.3  1992/10/22  15:04:37  sergey
Fixed EDGE bugs in find_edges_to_succs() and find_edges_to_preds().

Revision 1.2.1.2  92/10/09  18:57:47  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
