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
#include "globdef.h"
#ifndef ISO_CPP_HEADERS
#include <string.h>
#else /* ISO_CPP_HEADERS */
#include <cstring>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <defaultparser.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "constr.h"
#include "sugilevel.h"
#include "abstraction.h"

/* History :         ....                                                     */
/*              10/27/92      Sergey            Cleaned up ~graph             */

extern int debug;

/* declarations of external C functions */

#ifdef __cplusplus
extern "C" { // Allows linkage to non-C++ objects
#endif /* __cplusplus */

extern PFI find_function (char *, char *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

extern edge *new_edge (char *);

extern int IsBiDirEdge(graph*, node*, node*);
extern int IsBackEdge(graph*, node*, node*);
extern node* new_dummynode(graph*, node*, int, int);
extern int edge_is_visible(edge*);

extern void MakeImproper(graph*);
extern int node_x_pos (node *, graph *);
extern int node_y_pos (node *, graph *);
extern void takenodeoffnodelist (node *);

extern char* globargv[];
extern nlist *nodelist;
extern graph* globgraph; 

/*
 * graph constructor
 */
graph::graph(long )
{

    layoutalgorithm = barycenter;
    layoutfrequency = every;
    layoutparameter[0] = SUGI_BC_I;
    layoutparameter[1] = SUGI_BC_II;
    layoutparameter[2] = SUGI_PR;
    layoutparameter[3] = SUGI_SORTCC;
    layoutparameter[4] = SUGI_CC;
    layoutparameter[5] = INCR_LAYOUT;
    layoutparameter[6] = INSTAB_EXTENT;
    layoutparameter[7] = SUGI_LEVEL;
    xmax = X_MAX;
    ymax = Y_MAX;
    xbase = X_BASE;
    ybase = Y_BASE;
    xspace = X_SPACE;
    yspace = Y_SPACE;
    zspace = Z_SPACE;
    width = WIDTH_WINDOW;
    height = HEIGHT_WINDOW;
    scaling = 1;
    orientation = top_to_bottom;
    topsort = high;


  dirty = 1;
  proper = 1;
  headlevel = 0;
  headnode = 0;
  tailnode = 0;
  headedge = 0;
  tailedge = 0;
  rnode = 0;
  maxlevel = 0;
  lastnode = nexttolastnode = 0;
  selectednodes = new nlist;
  constraints = new constr_3D(this);
}

/*
 * graph constructor
 */
graph::graph(char *)
{
    layoutalgorithm = barycenter;
    layoutfrequency = every;
    layoutparameter[0] = SUGI_BC_I;
    layoutparameter[1] = SUGI_BC_II;
    layoutparameter[2] = SUGI_PR;
    layoutparameter[3] = SUGI_SORTCC;
    layoutparameter[4] = SUGI_CC;
    layoutparameter[5] = INCR_LAYOUT;
    layoutparameter[6] = INSTAB_EXTENT;
    layoutparameter[7] = SUGI_LEVEL;
    x = X_WINDOW;
    y = Y_WINDOW;
    xmax = X_MAX;
    ymax = Y_MAX;
    xbase = X_BASE;
    ybase = Y_BASE;
    xspace = X_SPACE;
    yspace = Y_SPACE;
    zspace = Z_SPACE;
    width = WIDTH_WINDOW;
    height = HEIGHT_WINDOW;
    scaling = 1;
    orientation = top_to_bottom;
    topsort = high;


  dirty = 1;
  proper = 1;
  headlevel = 0;
  headnode = 0;
  tailnode = 0;
  headedge = 0;
  tailedge = 0;
  rnode = 0;
  maxlevel = 0;
  lastnode = nexttolastnode = 0;
  selectednodes = new nlist ();
  constraints = new constr_3D(this);
}

/*
 * graph destructor
 */
graph::~graph()
{
    delete constraints;
    delete_nodes_all();
    delete selectednodes;
    delete_edges_all();
    if (headlevel)
        delete headlevel;
}

void graph::copy(graph* newg)
{
    newg->setlayouttype (layoutalgorithm);
    newg->setlayoutfreq (layoutfrequency);
    for (int i = 0; i <= 7; i++)
        newg->setlayoutpar (i, layoutparameter[i]);
    newg->setxspace (xspace);
    newg->setyspace (yspace);
    newg->setzspace (zspace);
    newg->setscaling (scaling);
    newg->setorientation (orientation);
    newg->settopsort (topsort);
    newg->constraints = constraints;
    newg->setxbase (xbase);
    newg->setybase (ybase);
    newg->setdirty (1);
}
/*
* add_node
*/
node*
graph::add_node(int d,int sgd,int type,char* type_name)
{
node *newnode = new node(type,type_name);
newnode->setdummy(d);
newnode->setsgdummy(sgd);

return (this->add_node(newnode));
}

/*
* add_node
*/
node*
graph::add_node(node* newnode)
{
    if(newnode->getwidth ()== 0)
                  newnode->setwidth(newnode->calcwidth());
    if(newnode->getheight() == 0)
                  newnode->setheight(newnode->calcheight());

newnode->setprevnode(tailnode);
newnode->setnextnode(0);
if(headnode ==0)
   headnode = newnode;
if (tailnode !=0)
 tailnode = newnode;
newnode->setbackgraph(this);
 return newnode;
}



/*
 * layout
 */
void 
graph::layout()
{
    long timestart;
    long timeend;
    node* n;
    timestart = time(0);

    for (n = this->getheadnode(); n != 0; n = n->getnextnode())
      if ((n->getrgraph() != 0) && n->getncon())
        n->disconnect();

    for (n = this->getheadnode(); n != 0; n = n->getnextnode())
      if ((n->getrgraph() != 0) && n->getncon())
        n->connect();

    if (this->getdirty()) {
     if (layoutfrequency == every) {
      if (layoutalgorithm == barycenter)
	  Sugilayout();
      else if (layoutalgorithm == isi)
	  Isilayout();
      else if (layoutalgorithm == tree) 
	  Treelayout();
      else if (layoutalgorithm == LAYOUT_SUGICONSTR)
	  Constraintlayout();
      else
	  Sugilayout();
    }
    else
 	 Nolayout ();
      this->setdirty(0);
    } 

    this->compute_size();

    timeend = time(0);
    if (debug) 
	printf("layout on graph took %d seconds\n", timeend-timestart);
}

/*
 * delete_nodes_all
 */
void 
graph::delete_nodes_all()
{
    node* n;
    if (this == NULL) return;
    n = this->getheadnode();
    while (n != 0) {
        node* nn = n->getnextnode();
	delete_node(n);
        n = nn;
    }
}


/*
 * delete_edges_all
 */
void 
graph::delete_edges_all()
{
    if (this == NULL) return;
    edge* e = this->headedge;
    while ( e != 0){
        edge *ee = e->getnextedge();
	delete e;
        e = ee;
    }
}

/*
 * layout_edges_all
 */
void 
graph::layout_edges_all()
{
    edge* e;
    if (this == NULL) return;
    for (e = this->headedge; e != 0; e=e->getnextedge()) 
	e->layout();
}


/*
 * delete_node
 */
void 
graph::delete_node(node* n)
{
    /* invoke application provided delete node procedure */

    // delete edges having n as source or target
    n->disconnect();

    // deleteing the node itself

    if (n == headnode) 
	headnode = n->getnextnode();
    if (n == tailnode) 
	tailnode = n->getprevnode();
    if (n->getprevnode() != 0)
	n->getprevnode()->setnextnode(n->getnextnode());
    if (n->getnextnode() != 0)
	n->getnextnode()->setprevnode(n->getprevnode());

    if (n->getrgraph() != 0)
	delete n->getrgraph();

    delete n;
    this->setdirty (1);
}

/*
*find_edge
*/
edge*
graph::find_edge(node* s,node* t)
{
    edge* e;
     if(this==NULL) return 0;
       for(e=this->headedge; e!=0; e=e->getnextedge())
         if ((e->getsource()== s) && (e->gettarget()== t))
             return e;
         return 0;
}



/* 
 * set_nodes_unreached
 *	used for determining where cycles occurr
 */
void 
graph::set_nodes_unreached()
{
    node* n;
    for (n = this->headnode; n != 0; n = n->getnextnode())
      if (n->getsgdummy() == 0) n->setreached(0);
                              else n->setreached(1);
}

/* 
 * compute_size
 *	Figure out how big the zoomed-in abstraction's box has to be
 */
void 
graph::compute_size()
{
    int nw;
    width = 0;
    height = 0;
    node *n;
    if ((this->orientation == left_to_right) || (this->orientation == right_to_left))
      for (n = this->getheadnode(); n != 0; n = n->getnextnode()) {
        if ((nw = n->gety() + n->getwidth() + 2*n->getborderwidth()) > width) width = nw;
        if ((nw = n->getx() + n->getheight() + 2*n->getborderwidth()) > height) height = nw;
      } else
      for (n = this->getheadnode(); n != 0; n = n->getnextnode()) {
        if ((nw = n->getx() + n->getwidth() + 2*n->getborderwidth()) > width) width = nw;
        if ((nw = n->gety() + n->getheight() + 2*n->getborderwidth()) > height) height = nw;
      }
    width = width + this->getxspace();
    height = height + this->getyspace();
}
 /*
 * add_edge
 */
edge* 
graph::add_edge(node* s, node* t, int d,int type,char *type_name,int check_back)
{

    if(s==t) 
            return 0;
    edge* newedge;
    if ((s == 0) || (t == 0)) {
        if (debug) printf("source and target nodes must exist before you can add an edge\n");
        return NULL;
    }

    newedge = new edge(s, t,type,type_name);
    newedge->setdummy(d);
    if (check_back && IsBackEdge(this, s,t)) {
        if (debug && (!s->isdummy()) && (!t->isdummy()))
        newedge->setreversed(1);
    }

    newedge->setprevedge(tailedge);
    newedge->setnextedge(0);
    if (tailedge != 0) tailedge->setnextedge(newedge);

    if (headedge == 0) headedge = newedge;
    tailedge = newedge;
    newedge->setbackgraph(this);                        // back pointer to this graph
    newedge->setdummy(d);                       // set dummy value
    newedge->getsource()->addsucc(newedge->gettarget());
    newedge->gettarget()->addpred(newedge->getsource());

    return newedge;
}


#if  0
     no refs, and it seems to be missing an else clause at end, fails
     buildxref because of  no final return stmt
/*
 * add_edge
*/
edge *graph::add_edge (edge *newedge)
{
  node *source = newedge->getsource ();
  node *target = newedge->gettarget ();
  graph* sg = source->getbackgraph();
  graph* tg = target->getbackgraph();
	
  sg->setdirty(1); tg->setdirty(1);

  if ((source == NULL) || (target == NULL)) {
    return NULL;
  }

  if (sg == tg) {

    if (((source->getbackgraph() != this) && (source->getsgdummy() != INDUMMY))
	|| ((target->getbackgraph() != this) && (target->getsgdummy() != OUTDUMMY))) {

    }

    if (IsBackEdge (this, source, target)) {
      newedge->setdummy (0); 
      newedge->setreversed (1);
    } 
    else {
      newedge->setreversed (0);
      newedge->setdummy (0);
    }

    newedge->setprevedge (tailedge);
    newedge->setnextedge (0);
    if (tailedge != 0) 
      tailedge->setnextedge (newedge);
    if (headedge == 0) 
      headedge = newedge;
    tailedge = newedge;
    newedge->setbackgraph (this);	                // back pointer to this graph
    newedge->setdummy (0);			// set dummy value
    newedge->getsource()->addsucc (newedge->gettarget());
    newedge->gettarget()->addpred (newedge->getsource());
      return newedge;
  }
}
#endif


void graph::Nolayout()
{
    for (node* n = getheadnode(); n != NULL; n = n->getnextnode())
	n->movenode(n->getx(), n->gety());
    this->layout_edges_all ();
}

void graph::setlayouttype(layout_algorithms i)
{ 
    if ((layoutalgorithm != LAYOUT_SUGICONSTR) && (i == LAYOUT_SUGICONSTR)) {
      MakeImproper(this);
      for (node* n=headnode; n; n=n->getnextnode()) {
	if (n->isabstraction())  {
	    n->getrgraph()->setlayouttype(i);
	    n->getrgraph()->setdirty(1);
	}
	n->stable=0;
      }
    }
    layoutalgorithm = i;
}

void graph::setlayoutfreq(layoutfrequencies i)
{
    layoutfrequency = i;
    for (node* n=headnode; n; n=n->getnextnode()) 
	if (n->isabstraction())
	    n->getrgraph()->setlayoutfreq(i);
}

void graph::setlayoutpar(int i, int val)
{
  if (i < 8) {
    layoutparameter [i]= val;
    for (node* n=headnode; n; n=n->getnextnode()) 
	if (n->isabstraction())
	    n->getrgraph()->setlayoutpar(i, val);
  }
}

void graph::setorientation(orientations i)
{
    orientation = i;
    for (node* n=headnode; n; n=n->getnextnode()) 
	if (n->isabstraction())  {
	    n->getrgraph()->setorientation(i);
	    n->getrgraph()->setdirty(1);
	}
}

void graph::settopsort(topsorts i)
{
    topsort = i;
    for (node* n=headnode; n; n=n->getnextnode()) 
	if (n->isabstraction())  {
	    n->getrgraph()->settopsort(i);
	    n->getrgraph()->setdirty(1);
	}
}

void graph::sety(int i)
{
  y  = i;
}

void graph::setx(int i)
{
  x  = i;
}

void graph::setxmax(int i)
{
  xmax  = i;
}

void graph::setymax(int i)
{
  ymax  = i;
}

void graph::setxbase(int i)
{
    xbase = i;
    for (node* n=headnode; n; n=n->getnextnode()) 
	if (n->isabstraction())  {
	    n->getrgraph()->setxbase(i);
	    n->getrgraph()->setdirty(1);
	}
}

void graph::setybase(int i)
{
    ybase = i;
    for (node* n=headnode; n; n=n->getnextnode()) 
	if (n->isabstraction())  {
	    n->getrgraph()->setybase(i);
	    n->getrgraph()->setdirty(1);
	}
}

void graph::setxspace(int i)
{
    xspace = i;
    for (node* n=headnode; n; n=n->getnextnode()) 
	if (n->isabstraction())  {
	    n->getrgraph()->setxspace(i);
	    n->getrgraph()->setdirty(1);
	}
}

void graph::setyspace(int i)
{
    yspace = i;
    for (node* n=headnode; n; n=n->getnextnode()) 
	if (n->isabstraction())  {
	    n->getrgraph()->setyspace(i);
	    n->getrgraph()->setdirty(1);
	}
}

void graph::setzspace(int i)
{
    zspace = i;
    for (node* n=headnode; n; n=n->getnextnode()) 
	if (n->isabstraction())  {
	    n->getrgraph()->setzspace(i);
	    n->getrgraph()->setdirty(1);
	}
}


/*
 * IsBiDirEdge
 *	returns 1 if there already exists edge from t to s
 */
int IsBiDirEdge(graph* g, node* s, node* t)
{
    edge* e;
    e = g->find_edge(t, s);
    if (e != NULL)
	return 1;
    else 
	return 0;
}
