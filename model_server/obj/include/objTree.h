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
//////////////////////   FILE objTree.h  //////////////////////////////////
//
// -- Contains Tree related stuff
//
#ifndef _objTree_h
#define _objTree_h

#ifndef _objRelation_h      
#include <objRelation.h>
#endif      

RelClass(objTree);

class objArr_Int;

class objTree : public Relational
  {
  objTree * tree_parent;
  objTree * tree_first;
  objTree * tree_next;
  objTree * tree_prev;

protected:
  // Attention: put_..._link methods do not remove some old links. For example,
  // put_next_link change "tree_prev" link of argument node but do not reset
  // "next" field of old previous node !!!!!!!!!! - It must be done manually !!
  void put_parent_link(objTree * t) { tree_parent = t; }
  void put_next_link(objTree * t) { tree_next = t; if(t) t->tree_prev = this; }
  void put_prev_link(objTree * t) { tree_prev = t; if(t) t->tree_next = this; }
  void put_first_link(objTree * t) { tree_first = t; }

public: 
  define_relational(objTree,Relational);
  objTree();
  // The copy constructor and the "=" operator must be defined to prevent
  // copying tree link pointers.
  objTree(const objTree &);			// Copy constructor
  objTree & operator = (const objTree &);	// "=" operator
  virtual ~objTree ();
  virtual void propagate(int,objArr*oa=0);     
  virtual  void remove();

  objTree * get_parent() const { return tree_parent; }
  objTree * get_next()   const { return tree_next;   }
  objTree * get_prev()   const { return tree_prev;   }
  objTree * get_first()  const { return tree_first;  }
  objTree * get_last()   const;

  void put_before(objTree *  obj);
  void put_after(objTree *  obj);
  void put_first(objTree *  obj);
  void add_parent_to_list(objTree *t1a, objTree *t2a);
  void remove_from_tree();
  void split_family(objTree *  foster_parent);
  int     depth();
  boolean subtree_of(objTree * node); 
  boolean is_before(objTree * node); 
  boolean is_between(objTree * node1, objTree * node2);

  objTree *  get_next_leaf(objTree * = 0);// Argument is root of subtree
  objTree *  get_prev_leaf(objTree * = 0);// which be used to search 
  objTree *  get_first_leaf();
  objTree *  get_last_leaf();
  objTree *  get_next_last();

  virtual objTree *  get_root() const;
  objTree *  get_root(objTree * node);
  void put_chain_after(objTree * obj);
  void splice();

  friend Obj * obj_copy_deep(Obj&, objCopier *, objArr_Int* = NULL);
  friend Obj * obj_copy_deep(Obj&, objCopier *, objArr_Int *, int do_regions);
  };

generate_descriptor(objTree,Relational);

extern void split_tree_path(objTree *  bottom, objTree *  top);
extern void obj_tree_get_leaves(objTree * root, objArr& coll);
//extern void obj_tree_merge(objArr& coll);
extern "C" void tree_prt(objTree * const);
extern objTree * obj_tree_copy(objTree * rt, objCopier *cp=0, RelType *rl=0);
extern objTree * obj_tree_common_root(objTree * node1,
                                       objTree * node2,
                                       objTree * *n1=0, objTree * *n2=0);

#define obj_tree(name)			       				\
    name* get_parent () const {return (name*)objTree::get_parent ();}		\
    name* get_first () const {return (name*)objTree::get_first ();}		\
    name* get_next () const {return (name*)objTree::get_next ();}		\
    name* get_prev () const {return (name*)objTree::get_prev ();}		\
    name* get_first_leaf () {return (name*)objTree::get_first_leaf ();}	\
    name* get_last_leaf () {return (name*)objTree::get_last_leaf ();}	\
    name* get_prev_leaf (objTree* r = NULL)    				\
                          {return (name*)objTree::get_prev_leaf (r);}	\
    name* get_next_leaf (objTree* r = NULL)	 			\
                          {return (name*)objTree::get_next_leaf (r);}

#endif // _objTree_h

/*
   START-LOG-------------------------------------------

   $Log: objTree.h  $
   Revision 1.3 1996/11/21 20:57:58EST mg 
   
 * Revision 1.2.1.9  1994/05/05  20:04:58  boris
 * Bug track: 6794
 * Fixed "slow merge" problem, and #6977 asyncronous save
 *
 * Revision 1.2.1.8  1993/05/26  12:10:02  aharlap
 * changed declaration of obj_copy_deep()
 *
 * Revision 1.2.1.7  1993/02/12  00:53:43  aharlap
 * added obj_tree macro
 *
 * Revision 1.2.1.6  1992/12/21  21:41:03  so
 * revision 1.2.1.5
 * is bad
 *
 * Revision 1.2.1.4  1992/12/01  19:38:02  boris
 * Changed first argument in obj_copy_deep() form Relational& to Obj&
 * to fix CUT functionality
 *
 * Revision 1.2.1.3  1992/11/27  21:51:53  so
 * Add one public member function add_parent_to_list(objTree *t1a, objTree *t2a)
 * The object "this" will be the parent of the list from t1a to t2a.
 * Step 1: The whole list is replaced by this.
 * Step 2: The whole list is added to the tree as the child of this.
 *
 * Revision 1.2.1.2  1992/10/09  18:58:20  boris
 * Fix comment
 *


   END-LOG---------------------------------------------

*/
