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
// objCopy.C
//
#include "objTree.h"
#include "objRelation.h"

#include <objArr.h>
#include <transaction.h>

class Relational;
class smtTree;

int is_ddElement (Relational*);
smtTree * smt_unite(Obj *ob, objArr_Int &region, int do_regions);

class copy_node : public Obj, public transactional
{
 public:
    Relational * old_ptr;
    Relational *new_ptr;
    unsigned hash() const {
	return (unsigned) old_ptr;
    }
          copy_node(Relational * n) {
	old_ptr = n;
	new_ptr = 0;
    }
          copy_node(Relational * n, Relational * o) {
	old_ptr = n;
	new_ptr = o;
    }
    bool  isEqual(const Object & x) const
    {
	return old_ptr == ((copy_node *) & x)->old_ptr;
    }
};

static inline Relational * get_new(objSet & copied, Relational * obj)
{
    copy_node key(obj);
    copy_node *c = (copy_node *) copied.os_list.findObjectWithKey(key);
    if (c)
      return c->new_ptr; 
    else
      return 0;
}



Obj  *
obj_copy_deep(Obj & root, objCopier * copier, objArr_Int* reg_int, int do_regions)
{
    Initialize(obj_copy_deep);
    
    Obj* answer;
    Obj  *w1;
    int   i;
    objSet copied;

    start_transaction() {
	/* 1-st stage: collect all object to be copied */
	objSet copy_set; 
	objArr copy_pend, copy_arr;
	Obj* el;
	ForEach (el, root)
	    copy_pend.insert_last(el);

	objArr prop;

	while (copy_pend.size()) {
	    Obj  *w = copy_pend[0];    	/* Get first pending object */
/* This code was replaced with the above copy pend because it produced a
   compilation warning.
            {
	      ForEach(w, copy_pend) 
	      {
		  if (w)
		      break;
	      }
	    }		
*/	    
	    copy_pend.remove(w);/* Remove it from pending set */
	    copy_set.insert(w);	/* and add to copy set */
	    copy_arr.insert_last(w);	/* and to copy_arr (to preserve
					 * order) */
	    ((Relational *) w)->propagate(relationMode::C, &prop);	/* Get set of nodes for
							 * copy */
	    {
		ForEach(w, prop)
		    if (!copy_set.includes(w))	/* &&
						 * copier->accept((Relational
						 * *)w, 0)) */
		    copy_pend.insert(w);
	    }
	    prop.removeAll();
	}

	/* 2-nd stage: copy all collected nodes */

	{
	    Obj  *w;
	    ForEach(w, copy_arr) {
		copied.insert((Relational *) new copy_node((Relational *) w,
					  copier->apply((Relational *) w)));
	    }
	}

	/* 3-rd stage: copy relations and tree links */
	{
	    Obj  *w;
	    ForEach(w, copied) {
		Relational *p_old = ((copy_node *) w)->old_ptr;
		Relational *p_new = ((copy_node *) w)->new_ptr;
		/* Copy tree */
		if (is_objTree(p_old) && ! is_ddElement (p_old))
		    for (i = 0; i < 4; i++) {
			objTree *to;
			switch (i) {
			case 0:
			    to = (checked_cast(objTree, p_old))->get_parent();
			    break;
			case 1:
			    to = (checked_cast(objTree, p_old))->get_first();
			    break;
			case 2:
			    to = (checked_cast(objTree, p_old))->get_next();
			    break;
			case 3:
			    to = (checked_cast(objTree, p_old))->get_prev();
			    break;
			}
			copy_node key(to);
			if (to)
			    to = checked_cast(objTree, get_new(copied, to));
			if (to)
			    switch (i) {
			    case 0:
				(checked_cast(objTree, p_new))->put_parent_link(to);
				break;
			    case 1:
				(checked_cast(objTree, p_new))->put_first_link(to);
				break;
			    case 2:
				(checked_cast(objTree, p_new))->put_next_link(to);
				break;
			    case 3:
				(checked_cast(objTree, p_new))->put_prev_link(to);
				break;
			    }
		    }
	    }
	}

	if (root.collectionp() == 1) {	/* objArr */
	    Assert(reg_int);
	    objArr & oa = *((objArr *) & root);
	    int   sz = root.size();
	    objArr new_root;
	    Relational *start_node = get_new(copied, (Relational *)copier->get_starter_obj());
	    
	    for (int ii = 0; ii < sz; ++ii)
		if(oa[ii]!=copier->get_starter_obj())
		    new_root.insert_last (get_new(copied, (Relational *) oa[ii]));
	    new_root.insert_first(start_node);
	    
	    //  The first node of new root _must_ be the one with the
	    //  header, because smt_unite will return this header.
	    answer = (Obj*)smt_unite(&new_root, *reg_int, do_regions);
	} else
	    answer = get_new(copied, (Relational *) & root);
	{
	    Obj  *w;
	    ForEach(w, copied) {
		Relational *p_old = ((copy_node *) w)->old_ptr;
		Relational *p_new = ((copy_node *) w)->new_ptr;
		relArr & rarr = p_old->get_arr();
		int   rsize = rarr.size();
		for (i = 0; i < rsize; i++) {
		    Relation *rel = rarr[i];	/* there was test for 0 here
						 * - is it needed? */
		    RelType *rt = rel->get_rel_type();
		    int   flags = rt->get_flags();
		    if (flags & (relationMode::C | relationMode::S)) {
			Obj  *mem = rel->get_members();
			ForEach(w1, *mem) {
			    Relational *to = (Relational *) w1;
			    if (flags & relationMode::C) {
				copy_node key(to);
				to = get_new(copied, to);
			    }
			    if (to)
				put_relation(rt, p_new, to);
			}
		    }
		}
	    }
	}
    } end_transaction();
    return answer;
}

Obj  *
obj_copy_deep(Obj & root, objCopier * copier, objArr_Int* reg_int)
{
    Initialize(obj_copy_deep);
    Obj *ob = obj_copy_deep (root, copier, reg_int, 0); // do not separate regions by default
    return ob;
}

objTree *
obj_tree_copy(objTree * root, objCopier * cp, RelType * rel)
{
    Initialize(obj_tree_copy);

#if 0
    // I beleave that this function is not called. If I'm wrong please
    // delete next line (call "Error") - 5/6/92 == MF
    Error(ERR_INPUT);
#endif


    objTree *nr = NULL;
    if (cp)
	nr = checked_cast(objTree, cp->apply(root));
    else
	nr = checked_cast(objTree,root->rel_copy());

    if (!nr)
	ReturnValue(NULL);
    if (rel)
	put_relation(rel, nr, root);

    objTree *prv = 0;
    for (objTree * cur = root->get_first(); cur; cur = cur->get_next()) {
	objTree *new_cur;
	if (new_cur = obj_tree_copy(cur, cp, rel)) {
	    if (prv)
		prv->put_after(new_cur);
	    else
		nr->put_first(new_cur);
	    prv = new_cur;
	}
    }
    ReturnValue(nr);
}

/*
 * START-LOG-------------------------------------------
 * 
 * $Log: objCopy.cxx  $
 * Revision 1.11 2002/03/04 17:24:18EST ktrans 
 * Change M/C/D/W/S macros to enums
Revision 1.2.1.11  1994/05/05  20:05:16  boris
Bug track: 6794
Fixed "slow merge" problem, and #6977 asyncronous save

Revision 1.2.1.10  1993/05/26  12:11:15  aharlap
fixed some copy/paste problems

Revision 1.2.1.9  1993/04/09  23:48:05  aharlap
bug #3209

Revision 1.2.1.8  1993/04/05  21:50:50  aharlap
part of bug #2749
 Revision 1.2.1.7  1993/01/05  20:54:10  jon Checked for
 * endless loop in obj_copy_deep
 * 
 * Revision 1.2.1.6  1992/12/01  19:36:19  boris Changed first argument in
 * obj_copy_deep() form Relational& to Obj& to fix CUT functionality
 * 
 * Revision 1.2.1.5  1992/11/23  18:49:54  wmm typesafe casts.
 * 
 * Revision 1.2.1.4  1992/10/19  17:23:55  wmm Write around bug in parser that
 * prevented importation.
 * 
 * Revision 1.2.1.3  92/10/15  11:29:09  mg if(root.collectionp() == 1)
 * 
 * Revision 1.2.1.2  92/10/09  18:55:16  boris Fix comments
 * 
 * 
 * END-LOG---------------------------------------------
 * 
 */
