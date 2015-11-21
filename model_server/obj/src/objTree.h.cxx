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
////////////////////  ObjTree.h.c
//
// Description : contains tree manipulation methods
//		 1-st part - internal structure related methods

#include <objTree.h>
#ifndef _objArr_h
#include <objArr.h>
#endif

// Constructor for objTree
objTree::objTree()
  {
  tree_parent = tree_first = tree_next = tree_prev = 0;
  return;
  }

// The copy constructor and the "=" operator must be defined to prevent
// copying tree link pointers.

// Copy constructor - initialize tree link pointers instead of copying
objTree::objTree(const objTree &)
  {
  tree_parent = tree_first = tree_next = tree_prev = 0;
  }			

// "=" operator - do nothing (do not copy tree link pointers)
objTree & objTree::operator = (const objTree &)
  {
  return *this;
  }
	

// Destructor for objTree
objTree::~objTree()
  {
  // Remove all tree - specific relations
  this->remove();
  }

// add this to be the parent of the list from t1a to t2a
// Step 1: The whole list is replaced by this.
// Step 2: The whole list is added to the tree as the child of this.
void objTree::add_parent_to_list(objTree *t1a, objTree *t2a)
{
    Initialize(objTree::add_parent_to_list);
    // The next statement is not needed now because "this" is just created
    // by db_new before coming here.
    /* this->remove_from_tree(); */

    /**********/
    /* STEP 1 */
    /**********/
    objTree *cr = t1a->get_parent();

    if (t1a->get_prev() == 0)
        cr->put_first_link(this);

    this->put_parent_link(cr);
    this->put_prev_link(t1a->get_prev());
    this->put_next_link(t2a->get_next());
    this->put_first_link(t1a);

    /**********/
    /* STEP 2 */
    /**********/
    t1a->put_prev_link(0);
    t2a->put_next_link(0);

    cr = t1a;
    while(1)
    {
        cr->put_parent_link(this);
        if (cr == t2a)
            break;
        cr = cr->get_next();
    }
}

// Put obj to tree of this before this
void objTree::put_before(objTree * t) 
  {
  objTree * parent = this->get_parent();
  t->remove_from_tree();                      	// remove obj from old tree
  t->put_prev_link(this->get_prev());	
  this->put_prev_link(t);     			// setup new prev/next
  t->put_parent_link(parent);     		// setup rel. to parent
  if (t->get_prev() == 0)            		// replace this as the first child
    parent->put_first_link(t);
  }

// Put obj to tree of this after this
void objTree::put_after(objTree * obj)
  {
  obj->remove_from_tree();                      // remove obj from old tree
  obj->put_next_link(this->get_next());
  this->put_next_link(obj);
  obj->put_parent_link(this->get_parent());
  }

// Put obj to tree of this as a first child
void objTree::put_first(objTree * obj)
  {
  obj->remove_from_tree();
  obj->put_parent_link(this);
  obj->put_next_link(this->get_first());
  this->put_first_link(obj);
  }

// Remove this (with its subtree) from tree
void objTree::remove_from_tree()
  {
  if(this->get_prev() == 0)
    {
    objTree *  w = this->get_parent();
    if(w)
      w->put_first_link(this->get_next());
    w = this->get_next();
    if(w)
      w->put_prev_link(0);
    }
  else
    this->get_prev()->put_next_link(this->get_next());
  this->put_next_link(0);
  this->put_prev_link(0);
  this->put_parent_link(0);
  }

void objTree::splice()
  {
  Initialize(objTree::splice);
  objTree * par = get_parent();		// p = parent
  objTree * prv = get_prev();
  objTree * nxt = get_next();
  objTree * fst = get_first();
  objTree * lst;
  Assert(par);
  Assert(fst);

  this->put_parent_link(0);

  this->put_first_link(0);

  for(objTree * t = fst; t; t = t->get_next())
    {
    t->put_parent_link(par);
    lst = t;
    } 

  if(prv == 0)
    par->put_first_link(fst ? fst : nxt);
  else
    {
    this->put_prev_link(0);
    prv->put_next_link(fst ? fst : nxt);
    }

  if(nxt)
    {
    this->put_next_link(0);
    nxt->put_prev_link(lst ? lst : prv);
    }

  Return;
  }

//		 2-nd part - internal structure unrelated methods
//
init_abstract_relational(objTree,Relational);

int objTree::depth()
  {
  int d = -1;
  for(objTree *  cur = this; cur; cur = cur->get_parent())
    ++d ;
  return d;
  }

// Find common root for two nodes
objTree *  obj_tree_common_root(objTree * node1, objTree * node2,
				objTree * *n1, objTree * *n2)
  {
  Initialize(obj_tree_common_root);
  int delta = node2->depth() - node1->depth();
  int i;
  if(delta > 0)
    for(i = 0; i < delta; i++)
      node2 = node2->get_parent();
  else 
    for(i = 0; i > delta; i--)
      node1 = node1->get_parent();
  objTree * x1 = 0, * x2 = 0;
  while(node1 != node2)
    {
    x1 = node1;
    x2 = node2;
    node1 = node1->get_parent();
    node2 = node2->get_parent();
    }
  if(n1) *n1 = x1;
  if(n2) *n2 = x2;
  ReturnValue(node1);
  }

boolean objTree::is_before(objTree *  node2)
  {
  objTree *  node1 = this;
  obj_tree_common_root(node1, node2, &node1, &node2);
  for(objTree *  cur = node1; cur; cur=cur->get_next())
    {
    if(cur==node2)
      return 1;
    }
  return 0;
  }

boolean objTree::is_between(objTree * n1, objTree * n2)
  {
  return n1->is_before(this) && this->is_before(n2);
  }


boolean objTree::subtree_of(objTree * root) 
  {
  for( objTree *  cur=this; cur; cur = cur -> get_parent())
    {
    if(cur==root)
      return 1;
    }
  return 0;
  }

// get last child
objTree * objTree::get_last() const
  {
  objTree * t, * p = 0;
  for(t = get_first(); t; t = t->get_next())
  {
    p = t; 
  }
  return p;
  }

void objTree::remove()    // remove this from tree completely
  {                               // cutting links in all directions
  objTree * next_child;
  objTree * curr = this->get_first();
  this->remove_from_tree();
  while (curr != NULL)
    {
    next_child = curr->get_next();
    curr->remove_from_tree();
    curr = next_child;
    }
  }

void objTree::split_family(objTree *  foster_parent)
  {
  objTree *  last_staying_child = this;
  objTree *  last_adopted_child = NULL;
  objTree *  adopted_child;
  objTree *  first_child;
  objTree *  old_parent = last_staying_child->get_parent();
            
  while((adopted_child = last_staying_child->get_next()) != NULL)
    {
    adopted_child->remove_from_tree();
    if (last_adopted_child != NULL)
      last_adopted_child->put_after(adopted_child);
    else
      {
      if ( !(first_child = foster_parent->get_first() ) )
        foster_parent->put_first(adopted_child);
      else
        first_child->put_after(adopted_child);
      }
    last_adopted_child = adopted_child;
    }
  if ( old_parent->get_parent() )
    old_parent->put_after(foster_parent);
  else
    this->put_after(foster_parent);
  }

#define MISSED_TOP 253

void split_tree_path(objTree *  bottom_node, objTree *  top_node)
  {
  Initialize(split_tree_path);
  objTree *bottom_sibling = checked_cast(objTree,bottom_node->get_next()),
  *bottom_parent =  checked_cast(objTree,bottom_node->get_parent()),
  *new_parent;
  if (bottom_parent == NULL)
    Error(MISSED_TOP);
  new_parent = checked_cast(objTree, bottom_parent->rel_copy());
  bottom_node->split_family(checked_cast(objTree,new_parent));
  if (bottom_parent != top_node)
    split_tree_path(bottom_parent, top_node);
  return;
  }

// Get first leaf of subtree
objTree * objTree::get_first_leaf()
  {
  objTree * t, * l;
  for(t = l = this; t; t = t->get_first())
    l = t;
  return l;
  }

// Get last leaf of subtree
objTree * objTree::get_last_leaf()
  {
  objTree * t, * l;
  for(t = l = this; t; t = t->get_last())
    l = t;
  return l;
  }

// Get next leaf of subtree
objTree * objTree::get_next_leaf(objTree * root)
  {
  objTree * t, * w;
  for(t = this; t; t = w)
    {
    if(w = t->get_first())
      continue;
    if(t != this)
      break;
    for(; t != 0 && t != root; t = t->get_parent())
      if(w = t->get_next())
        break;
    if(t == root)
      return 0;
    }
  return t;
  }

// Get previous leaf of subtree
objTree * objTree::get_prev_leaf(objTree * root)
  {
  objTree * t, *w;
  for(t = this; t; t = w)
    {
    if(w = t->get_last())
      continue;
    if(t != this)
      break;
    for(; t != 0 && t != root; t = t->get_parent())
      if(w = t->get_prev())
	break;
    if(t == root)
      return 0;
    }
  return t;
  }

//    get_next_last();
//

objTree * objTree::get_next_last()
  {
  objTree * next, * curr=this;

  while (next = curr->get_next())
                           curr = next;
  return (curr);
  }

// Propagate method for objTree class
void objTree::propagate(int flag, objArr* arr)
  {
  objTree *t;
  ((Relational *)this)->Relational::propagate(flag,arr);
  if(flag & relationMode::USDC)		// Unload Copy & Delete propagated to children
    for(t = this->get_first(); t != NULL; t = t->get_next())
      arr->insert_last(t);
  }

objTree *  objTree::get_root() const
  {
    Initialize(objTree::get_root);

  objTree * cur;
  objTree * par;
  for(par = checked_cast(objTree,this); par; par = par->get_parent())
    cur = par;
  return checked_cast(objTree, cur);
  }

objTree *  objTree::get_root(objTree * node)
  {
  return obj_tree_common_root(this, node);
  }
/*
   START-LOG-------------------------------------------

   $Log: objTree.h.cxx  $
   Revision 1.5 2002/03/04 17:24:32EST ktrans 
   Change M/C/D/W/S macros to enums
Revision 1.2.1.9  1993/07/20  15:13:29  aharlap
added  Assert(fst);
to splice

Revision 1.2.1.8  1993/03/29  19:28:31  mg
#define U     32

Revision 1.2.1.7  1992/12/21  21:37:58  so
revision 1.2.1.6
is bad

Revision 1.2.1.5  1992/12/01  15:16:08  so
Add Initialize.
Use more efficient routines.

Revision 1.2.1.4  1992/11/27  21:56:04  so
Add one public member function add_parent_to_list(objTree *t1a, objTree *t2a)
The object "this" will be the parent of the list from t1a to t2a.
Step 1: The whole list is replaced by the object "this".
tep 2: The whole list is added to the tree as the child of the object "this".

Revision 1.2.1.3  1992/11/23  18:49:54  wmm
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:55:33  boris
Fix comments

  
   History:   05/04/92     M.Furman	   Reimplemented

   END-LOG---------------------------------------------

*/
