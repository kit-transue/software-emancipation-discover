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
#include "defaultparser.h"
//#include "defaults.h"

extern int debug;


/* declarations of external C functions */

#ifdef __cplusplus
extern "C" { // Allows linkage to non-C++ objects
#endif /* __cplusplus */
extern void error (char *, ...);
extern void warning (char *, ...);
#ifdef __cplusplus
}
#endif /* __cplusplus */

extern int APPL_DrawEdge(edge*);
extern int APPL_EditEdge(edge*, char*);
extern int APPL_ShowEdge(edge*);
extern int APPL_ActionEdge(edge*);
extern graph *loadgraph;
extern node* new_node(char*);

/*
 * edge constructor, one that does everything that can be spec. in IDL
 */
edge::edge(node* s, node* t , int type, char* tname)
{
    settype(type);
    type_name = NULL;
    if (tname != NULL)
      settypename (tname);

//    else
//      settypename(DEFAULTEDGENAME);
    nextedge = 0;
    prevedge = 0;
    source = s;
    sourcename = NULL;
    target = t;
    targetname = NULL;
    sourceloc.x = s->getx()+(s->getwidth()/2);
    sourceloc.y = s->gety()+(s->getheight());
    targetloc.x = t->getx()+(t->getwidth()/2);
    targetloc.y = t->gety();
    reversed = 0;
    dummy = 0;
}

/*
 * edge constructor
 */
edge::edge (char *tname)
{

    type_name = NULL;
    if (tname != NULL)
      settypename (tname);
//    else
//      settypename ("");
    sourcename = NULL;
    targetname = NULL;

  settype (DEFAULTEDGE);

  source = NULL;
  target = NULL;
  nextedge = NULL;
  prevedge = NULL;
  backgraph = NULL;
  sourceloc.x = 0;
  sourceloc.y = 0;
  targetloc.x = 0;
  targetloc.y = 0;
  reversed = 0;
  direction = 0;
  dummy = 0;
}

/*
 * edge destructor
 */
edge::~edge()
{
    if (type_name != NULL)
      delete type_name;
    if (sourcename != NULL)
      delete sourcename;
    if (targetname != NULL)
      delete targetname;
}

/*
 * edge copy
 */
void edge::copy(edge* newe)
{
    newe->settype (type);
    newe->settypename (type_name);
// ??    type_name = newe->type_name;
    newe->setdirection (direction);
    newe->setreversed(reversed);
}
/* 
 * action
 */
void 
edge::action()
{
    APPL_ActionEdge(this);
}
/*
 * settypename
 *	sets the typename field of the edge 
 */
void 
edge::settypename(char* l)
{
  if (l != NULL) {
    if (type_name != NULL)
      delete type_name;
    type_name = new char [MAXTEXTSIZE];
    strcpy (type_name, l);
  }
}

/*
 * setsourcename
 *	sets the sourcename field of the edge 
 */
void 
edge::setsourcename(char* l)
{
  if (l != NULL) {
    if (sourcename != NULL)
      delete sourcename;
    sourcename = new char [MAXTEXTSIZE];
    strcpy (sourcename, l);
  }
}

/*
 * settargetname
 *	sets the targetname field of the edge 
 */
void 
edge::settargetname(char* l)
{
  if (l != NULL) {
    if (targetname != NULL)
      delete targetname;
    targetname = new char [MAXTEXTSIZE];
    strcpy (targetname, l);
  }
}

/*
 * node_x_pos
 */
int 
node_x_pos(node* n, graph* g)
{
  if ((n->getbackgraph()->getorientation() == top_to_bottom) || (n->getbackgraph()->getorientation() == bottom_to_top)){
    if (n->getbackgraph() == g)
      return n->getx();
    else
      return n->getx()
           + node_x_pos(n->getbackgraph()->getrnode(), g)
           + n->getbackgraph()->getrnode()->getborderwidth();
  } else {
    if (n->getbackgraph() == g)
      return n->gety();
    else
      return n->gety()
           + node_x_pos(n->getbackgraph()->getrnode(), g)
           + n->getbackgraph()->getrnode()->getborderwidth();
  }
}

/*
 * node_y_pos
 */
int 
node_y_pos(node* n, graph* g)
{
  if ((n->getbackgraph()->getorientation() == top_to_bottom) || (n->getbackgraph()->getorientation() == bottom_to_top)) {
    if (n->getbackgraph() == g)
      return n->gety();
    else
      return n->gety()
           + node_y_pos(n->getbackgraph()->getrnode(), g)
           + n->getbackgraph()->getrnode()->getborderwidth();
  } else {
    if (n->getbackgraph() == g)
      return n->getx();
    else
      return n->getx()
           + node_y_pos(n->getbackgraph()->getrnode(), g)
           + n->getbackgraph()->getrnode()->getborderwidth();
  }
}

/*
 * layout
 */
void
edge::layout()
{
    node* s, * t;

    s = source;
    t = target;

    if ((backgraph->getorientation() == top_to_bottom) || (backgraph->getorientation() == bottom_to_top)) {
	if (node_y_pos(s, this->backgraph) == node_y_pos(t, this->backgraph)) {
	    if (node_x_pos(s, this->backgraph) < node_x_pos(t, this->backgraph)) {
	      /* edge points from left to right */
    	      sourceloc.x = node_x_pos(s, this->backgraph) + s->getwidth() + 2*s->getborderwidth();
    	      sourceloc.y = node_y_pos(s, this->backgraph) + s->getheight()/2;
    	      targetloc.x = node_x_pos(t, this->backgraph);
    	      targetloc.y = node_y_pos(t, this->backgraph) + t->getheight()/2;
	    } else {
	      /* edge points from right to left */
    	      sourceloc.x = node_x_pos(s, this->backgraph);
    	      sourceloc.y = node_y_pos(s, this->backgraph) + s->getheight()/2;
    	      targetloc.x = node_x_pos(t, this->backgraph) + t->getwidth() + 2*t->getborderwidth();
    	      targetloc.y = node_y_pos(t, this->backgraph) + t->getheight()/2;
	    }
	  return;
	} 
        if (!s->isdummy()) {
	  if ((reversed == 0) && (backgraph->getorientation() == top_to_bottom) || 
	      (reversed == 1) && (backgraph->getorientation() == bottom_to_top)) {
    	    sourceloc.x = node_x_pos(s, this->backgraph)
                        + (s->getwidth()/2)+(s->getborderwidth());
    	    sourceloc.y = node_y_pos(s, this->backgraph)
                        + (s->getheight())+2*(s->getborderwidth());
	  } else {	// reversed edge
    	    sourceloc.x = node_x_pos(s, this->backgraph)
                        + (s->getwidth()/2)+(s->getborderwidth());
    	    sourceloc.y = node_y_pos(s, this->backgraph)
                        + (s->getborderwidth());
	  }
	} else {	// dummy node
    	    sourceloc.x = node_x_pos(s, this->backgraph);
                        //+ (s->getwidth()/2)+(s->getborderwidth());
    	    sourceloc.y = node_y_pos(s, this->backgraph);
                        //+ (s->getheight()/2)+(s->getborderwidth());
	}
	if (!t->isdummy()) {
	  if ((reversed == 0) && (backgraph->getorientation() == top_to_bottom) ||
	      (reversed == 1) && (backgraph->getorientation() == bottom_to_top)) {
    	    targetloc.x = node_x_pos(t, this->backgraph)
                        + (t->getwidth()/2)+(t->getborderwidth());
    	    targetloc.y = node_y_pos(t, this->backgraph);
	  } else {
    	    targetloc.x = node_x_pos(t, this->backgraph)
                        + (t->getwidth()/2)+(t->getborderwidth());
    	    targetloc.y = node_y_pos(t, this->backgraph)
			+ t->getheight() + 2*(t->getborderwidth());
	  }
	} else {	// dummy node
    	    targetloc.x = node_x_pos(t, this->backgraph);
                        //+ (t->getwidth()/2)+(t->getborderwidth());
    	    targetloc.y = node_y_pos(t, this->backgraph);
                        //+ (t->getheight()/2)+(t->getborderwidth());
	}
    } else {		// orientation is LEFT-TO-RIGHT
	if (node_x_pos(s, this->backgraph) == node_x_pos(t, this->backgraph)) {
	    if (node_y_pos(s, this->backgraph) < node_y_pos(t, this->backgraph)) {
	      /* edge points from top to bottom */
    	      sourceloc.x = node_x_pos(s, this->backgraph) + s->getwidth()/2;
    	      sourceloc.y = node_y_pos(s, this->backgraph) + s->getheight();
    	      targetloc.x = node_x_pos(t, this->backgraph) + t->getwidth()/2;
    	      targetloc.y = node_y_pos(t, this->backgraph);
	    } else {
	      /* edge points from bottom to top */
    	      sourceloc.x = node_x_pos(s, this->backgraph) + s->getwidth()/2;
    	      sourceloc.y = node_y_pos(s, this->backgraph);
    	      targetloc.x = node_x_pos(t, this->backgraph) + t->getwidth()/2;
    	      targetloc.y = node_y_pos(t, this->backgraph) + t->getheight();
	    }
	  return;
	} 
        if (!s->isdummy()) {
	  if ((reversed == 0) && (backgraph->getorientation() == left_to_right) || 
	      (reversed == 1) && (backgraph->getorientation() == right_to_left)) {
    	    sourceloc.x = node_x_pos(s, this->backgraph)
                        + (s->getwidth())+2*(s->getborderwidth());
    	    sourceloc.y = node_y_pos(s, this->backgraph)
                        + (s->getheight()/2)+(s->getborderwidth());
	  } else {
    	    sourceloc.x = node_x_pos(s, this->backgraph);
    	    sourceloc.y = node_y_pos(s, this->backgraph)
                        + (s->getheight()/2)+(s->getborderwidth());
	  }
	} else {	// dummy node
    	    sourceloc.x = node_x_pos(s, this->backgraph);
                        //+ (s->getwidth()/2)+(s->getborderwidth());
    	    sourceloc.y = node_y_pos(s, this->backgraph);
                        //+ (s->getheight()/2)+(s->getborderwidth());
	}
	if (!t->isdummy()) {
	  if ((reversed == 0) && (backgraph->getorientation() == left_to_right) || 
	      (reversed == 1) && (backgraph->getorientation() == right_to_left)) {
    	    targetloc.x = node_x_pos(t, this->backgraph);
    	    targetloc.y = node_y_pos(t, this->backgraph)
                        + (t->getheight()/2)+(t->getborderwidth());
	  } else {
    	    targetloc.x = node_x_pos(t, this->backgraph) 
			+ t->getwidth() + 2*(t->getborderwidth());
    	    targetloc.y = node_y_pos(t, this->backgraph)
                        + (t->getheight()/2)+(t->getborderwidth());
	  }
	} else {	// dummy node
    	    targetloc.x = node_x_pos(t, this->backgraph);
                        //+ (t->getwidth()/2)+(t->getborderwidth());
    	    targetloc.y = node_y_pos(t, this->backgraph);
                        //+ (t->getheight()/2)+(t->getborderwidth());
	}
    }
}
/*
   START-LOG-------------------------------------------

   $Log: edge.cxx  $
   Revision 1.2 1998/08/10 18:20:28EDT pero 
   port to VC 5.0: removal of typename, or, etc.
Revision 1.2.1.7  1993/08/25  21:38:16  sergey
Correction in copy().

Revision 1.2.1.6  1993/04/26  15:52:55  sergey
More constructor clean up. Part of bug #3463.

Revision 1.2.1.5  1993/04/25  22:38:13  sergey
Set targetname to NULL in edge constructor. Part of bug #3463.

Revision 1.2.1.4  1993/04/22  21:05:22  sergey
Removed default edge code to simplify things. Part of bug #3463.

Revision 1.2.1.3  1992/11/21  21:26:40  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:57:39  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
