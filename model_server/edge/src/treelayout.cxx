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

// History:   Sergey      06/11/92  Alligned nodes of one level top-y.

//            Sergey      06/19/92  Node overlaping bug fixed; removed
//                                  duplication of children shift; 
//                                  renamed local variables to avoid
//                                  conflict with same class data names.
//            Sergey      07/27/92  Fixed another overlaping in set_subtree.

/*	Implements the tree layout
 */

#include <msg.h>
#include "all.h"
#include "sugilevel.h"
#include "defaultparser.h"

int notree = 0;
extern int debug;


/* declarations of external C functions */

#ifdef __cplusplus
extern "C" 
{ // Allows linkage to non-C++ objects
#endif /* __cplusplus */

extern void error (char *, ...);
extern void warning (char *, ...); 

#ifdef __cplusplus
}

#endif /* __cplusplus */


extern nlist* FindTopNodes(graph*);
extern void MakeImproper(graph*);


/*
 * tree_insert
 * insert a node and all his successors into the tree level structure
 * returns the pointer to the inserted levelitem
 */

levelitem* 
tree_insert(node* pp, int levelnr, level* headlevel) 
{
  levelitem* li = 0;
  if (!pp->isreached()) {
    pp->setreached(1);
    slist_iterator_forw sons(*(slist*) pp->getsucclist());
    node* son;
    levelitem* first_son = 0;
    int soncount = 0;
    if (son = (node*)sons()) {
      if (!son->isreached()){
	first_son = tree_insert(son,levelnr+1,headlevel);
	soncount+=1; 
      }
      else notree = 1;
      while (son = (node*)sons()) {
	if (!son->isreached()) {
	  first_son = tree_insert(son,levelnr+1,headlevel);
	  soncount+=1; 
	}
	else
           notree = 1;
      }
    }
    
    pp->setlevel(levelnr);
    int x,y;
    if ((pp->getbackgraph()->getorientation() == left_to_right) || (pp->getbackgraph()->getorientation() == right_to_left)) {
      x = pp->getheight()/2 + pp->getborderwidth();
      y = pp->getwidth() + 2 * pp->getborderwidth();
    } else {
      x = pp->getwidth()/2 + pp->getborderwidth();
      y = pp->getheight() + 2 * pp->getborderwidth();
    }
    treeinfo* ti = new treeinfo(soncount,x,y,first_son);
    li = headlevel->add_node(pp,ti);
  }
  else 
    msg("warning: Graph is not tree.") << eom;

  return li;
}


/*
 * NewTreeLevelStruct
 * inserts nodes into the level structure according to a tree traversal
 *
 * after this procedure for each node exists a levelitem on the level
 * with levelnumber == depth of the node from the root.
 * On each level the items are in reverse order: the leftmost son of 
 * the leftmost parent appears on the rightmost position
 */
level* 
NewTreeLevelStruct(graph* g) 
{
  // create new level structure;
  level* headlevel;
  headlevel = new level(g);
  
  // find all top nodes (the roots of trees) and insert their nodes;
  // into the level structure;
  g->set_nodes_unreached();
  nlist* tmp = FindTopNodes(g);
  slist_iterator_forw topnodes(*(slist*) tmp);
  node* ppp;
  while(ppp = (node*)topnodes()) {
        tree_insert(ppp,0,headlevel);
  }

  if (tmp)
        delete tmp;

  return (headlevel);
}


/* 
 *void set_subtree
 * computes x and y coordinates of a subtree rooted in li
 */
void 
set_subtree(levelitem* li_it) 
{

  int tmp_abs, tmp_rel, tmp_h, tmp_h_node, tmp_y;

  tmp_abs = li_it->getinfo()->abs_pos;

//   tmp_h = li_it->getinfo()->half_x_width;  ? old EDGE code
//   tmp_x = tmp_abs - tmp_h;                 ? why they do it ???
//   li_it->getnode()->setx(tmp_x);
   li_it->getnode()->setx(tmp_abs);           // sergey.

// make nodes top-y alligned - sergey;
//--old code  li_it->getnode()->sety(li->getlevel()->getheight());

  tmp_h = li_it->getlevel()->getheight();
  tmp_h_node = li_it->getnode()->getheight();
  tmp_y =  tmp_h + tmp_h_node /2;      // - default "bottom to top" layout
  li_it->getnode()->sety(tmp_y );
                     
  levelitem* son = li_it->getinfo()->first_son;
  for (int i=0; i<li_it->getinfo()->nr_kids; i++) {
    tmp_abs = li_it->getinfo()->abs_pos;
    tmp_rel = son->getinfo()->rel_pos;
    int pos = tmp_abs + tmp_rel;

    son->getinfo()->abs_pos = pos;
    set_subtree(son);
    son = son->getprevitem();
  }
}


/*
 * set_tree_display
 * computes the x and y coordinates from the tree level structure
 * by replacing the relative coordinates by absolute coordinates 
 */
void 
level::set_tree_display() 
{
  int y = backgraph->getybase();
  int yy;
  for (level* l = this; l ; l = l->getnextlevel()) {
    for (levelitem* l_it=l->first_item; l_it; l_it=l_it->getnextitem())
      if (l->y_height < l_it->getinfo()->y_height) 
	l->y_height = l_it->getinfo()->y_height;
    yy = l->y_height;

    l->y_height = y;
    y += yy + backgraph->getyspace();
  }
  for (levelitem* l_it = first_item; l_it; l_it=l_it->getnextitem()) {
    // for all roots;
    set_subtree(l_it);
  }
}


/*
 * shift_right
 * shifts a levelitem to the right as much as possible
 */
void 
shift_right(levelitem* l_it) 
{
  int* s = &l_it->getinfo()->max_r_shift;
  l_it->getinfo()->abs_pos += *s;
  if (l_it->getprevitem()) 
    l_it->getprevitem()->getinfo()->max_r_shift += *s;
  *s = 0;
}


/*
 * Treelayout
 */
void 
graph::Treelayout()
{
  // undo poss. sugi layout
  MakeImproper(this);

  for (node* n = this->getheadnode(); n != 0; n = n->getnextnode())
    if (n->getrgraph() != 0) {
	if (n->getncon()) {
	    n->disconnect();
	    n->connect();
        }
        n->getrgraph()->layout();
    }
  
  headlevel = NewTreeLevelStruct(this);

  levelitem* lli;
  levelitem* first;
  level* l = headlevel;
  while (l->getnextlevel()) 
         l = l->getnextlevel();
  // l points to the bottom level;
  
  if (l->getfirstitem()) {  // if there are nodes on this level ...;
                            // initialisation: put nodes on bottom level 
			    // as close together as possible;
    first=l->getfirstitem();
    lli = first;
    int node_pos = xbase + lli->getinfo()->half_x_width;
    while (lli) {
      lli->getinfo()->abs_pos = node_pos;
      if (lli->getnextitem())
	node_pos += lli->getinfo()->half_x_width + xspace + 
	            lli->getnextitem()->getinfo()->half_x_width;
      lli = lli->getnextitem();
    }
    
    // start of the layout algorithm;
    while (l = l->getprevlevel()) {
      first = l->getfirstitem();
      
      // first step: compute positions of those nodes which have sons;
      lli = first;
      int kids;
      while (lli) {
	kids = lli->getinfo()->nr_kids;
	if (kids > 0) {
          int i;	  
	  levelitem* son = lli->getinfo()->first_son;

// - removed unneeded buggy code - sergey
	  // compute absolute position and maximum right shift for father;
	  int p = son->getinfo()->abs_pos;
	  for (i=1; i<kids; i++)
               son = son->getprevitem();
	  p = (p + son->getinfo()->abs_pos) / 2;
	  lli->getinfo()->abs_pos = p;
	  lli->getinfo()->max_r_shift = 
               lli->getinfo()->first_son->getinfo()->max_r_shift;
	  // compute relative positions of sons;
	  son = lli->getinfo()->first_son;
	  for (i=0; i<kids; i++) {
              son->getinfo()->rel_pos = son->getinfo()->abs_pos - p;
	      son = son->getprevitem();
	  }
	}
	lli=lli->getnextitem();
      }
      
      // second step: put in those nodes which have no sons;
      // the other nodes which have already their absolute positions may be
      // shifted to the right if there is not enough space;

      lli = first;
      int shift = 0;
      int pos;
      kids = lli->getinfo()->nr_kids;
      if (kids > 0) {
	pos = lli->getinfo()->abs_pos;
	int d = pos - lli->getinfo()->half_x_width - xbase;
	if (d < 0) {
	  shift = -d;
	  pos += shift;
	  lli->getinfo()->abs_pos = pos;
	}
      }	
      else 
          lli->getinfo()->abs_pos = pos = xbase + lli->getinfo()->half_x_width;

      while (lli = lli->getnextitem()) {
 	kids = lli->getinfo()->nr_kids;
	treeinfo* previnfo = lli->getprevitem()->getinfo();
	int nextpos = previnfo->abs_pos + previnfo->half_x_width +
	              previnfo->max_r_shift + xspace + lli->getinfo()->half_x_width;
	if (kids == 0) {
	  lli->getinfo()->abs_pos = nextpos;
	  previnfo->max_r_shift = 0;
	} 
        else {
	    if (shift > 0) 
	        lli->getinfo()->abs_pos += shift;
            int space = lli->getinfo()->abs_pos - nextpos;
	    if (space < 0) {
	        shift -= space;
                lli->getinfo()->abs_pos -= space;
	    }
            else {
	        if (previnfo->nr_kids == 0) 
                previnfo->max_r_shift = space;
            }
	}
      }
    }
    
    headlevel->set_tree_display();
    for (node* ppp = headnode; ppp; ppp = ppp->getnextnode())
        ppp->movenode(ppp->getx(), ppp->gety());

    layout_edges_all();
    dirty = 0;
    
    delete headlevel;
    headlevel = 0;
    
    this->compute_size();
  }
}
/*
   START-LOG-------------------------------------------

   $Log: treelayout.cxx  $
   Revision 1.2 2000/07/12 18:08:50EDT ktrans 
   merge from stream_message branch
Revision 1.2.1.5  1993/04/29  17:56:57  sergey
Added clean up after slist iterator. Part of bug #3463.

Revision 1.2.1.4  1993/04/19  17:44:29  sergey
Minor correction to clean the warning. Part of bug #3420.

Revision 1.2.1.3  1992/11/21  21:26:40  builder
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:57:58  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
