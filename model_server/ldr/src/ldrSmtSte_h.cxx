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
////////////////////////  FILE ldrSmtSte_h.C  ////////////////////////
#include <objOperate.h>
#include "ldrSmtSte.h"
#include "lde_smt_text.h"
#include "steDisplayNode.h"
#include <objArr.h>
#include "reference.h"

init_relational(ldrSmtSte,steDisplayHeader);

steDisplayNodePtr ldrSmtSte::do_extract( smtTreePtr rt, steNodeType tp ){
  return lde_extract_smt_text( rt, tp);
}

ldrSmtSte::ldrSmtSte(smtHeaderPtr smt_app, smtTreePtr root) {
  Initialize(ldrSmtSte::ldrSmtSte);

  smtTreePtr text_root = root;
  if ( text_root == NULL )
     text_root = checked_cast(smtTree,smt_app->get_root());

  app_put_ldr(smt_app, this);
  set_type(Rep_SmtText);

  steDisplayNode * new_display = NULL;
  if ( text_root )
     new_display = do_extract( text_root, steSTR );

  if ( new_display )
     put_root(new_display);
}

ldrTree * ldrSmtSte::generate_tree(appTree * t, int m)
// This does not seem so be used anywhere. -jef
  {
  Initialize(ldrSmtSte::generate_tree);

  return do_extract(checked_cast(smtTree,t), (steNodeType)m);
  }

objTree * smt_find_ldr(ldr * lh, objTree * an);
void ste_get_subtrees (objTreePtr st, objTreePtr en, objArr&, int uppest = 1);

static void ldr_node_delete (objTreePtr nd)
{
    Initialize(ldr_node_delete);
    objTree *prnt, *cur = nd;
    for( ; prnt = cur->get_parent(); cur = prnt) {
	if(cur->get_prev() || cur->get_next() || !prnt->get_parent()) {
	    cur->remove_from_tree();
	    obj_delete(cur);
	    break;
	}
    }
}

static void ldr_region_delete(objTreePtr nd1, objTreePtr nd2)
{
    Initialize(ldr_region_delete);
    if (!nd1)
	return;
    
    if (nd2 == 0 || nd2 == nd1) { 
	ldr_node_delete (nd1);
	return;
    }

    objArr ar;
    ste_get_subtrees (nd1, nd2, ar, 1);

    Obj *cur;
    ForEach(cur,ar) {
	((objTree *)cur)->remove_from_tree();
	obj_delete (cur);
    }
}

void ldrSmtSte::insert_obj(objInserter *oi, objInserter *ni) 
{
    Initialize(ldrSmtSte::insert_obj);

// Skip "smt split operation" that has sence just for ldrSmodShort Header
    if ( (oi->src_obj == NULL && oi->targ_obj == NULL && oi->data != NULL &&
	  oi->type == SPLIT) ||
	 (oi->src_obj == oi->targ_obj && oi->data == NULL &&
	  oi->pt == (void *)1 && oi->type == REPLACE) )
    {
	ni->type = NULLOP;
	return;
    }

    smtTree * so = checked_cast(smtTree,oi->src_obj);  // the source SMT node
    smtTree * to = checked_cast(smtTree,oi->targ_obj); // the target SMT node
    objOperType type = oi->type;
    char *d = (char *)oi->data;
    
// Convert REPLACE_REGION into REGENERATE
    if (type == REPLACE_REGION) {
	type = ni->type = REPLACE;
	ni->data = 0;
	d = 0;
	ni->src_obj = ni->targ_obj = obj_tree_common_root (so, to);
	so = to = checked_cast(smtTree,ni->src_obj);
    }

    if (type == REMOVE) {
	steDisplayNodePtr st, en;
	st = checked_cast(steDisplayNode,smt_find_ldr(this, so));
	en = ( to ) ? checked_cast(steDisplayNode,smt_find_ldr(this, to)) : 0;
	if (st == 0) {
	    ni->type = NULLOP;
	    return;
	}
	ni->src_obj  = st;
	ni->targ_obj = en;
	if (en == 0 || d == 0)
	    ldr_node_delete (st);
	else {
	    objRegion * obj_reg = (objRegion *)oi->data;
	    int off1 = obj_reg->start->offset_of;
	    int off2 = obj_reg->end->offset_of;
	    if (st == en) {
		if (off1 == 0 && off2 == -1)
		    ldr_node_delete (st);
		return;
	    }

	    objTreePtr frst = st->get_first_leaf();
	    objTreePtr last = en->get_last_leaf();

	    if (frst == last) {
		if (off1 != 0 || off2 != -1)
		    frst = last = 0;
            } else if (frst->get_next_leaf() == last) {
		if (off1 > 0 && off2 == -1)
		    frst = last;
		else if (off1 == 0 && off2 != -1)
		    last = frst;
		else if (off1 != 0 && off2 != -1)
		    frst = last = 0;
	    } else {
		if (off1 != 0)
		    frst = frst->get_next_leaf();

		if ( off2 != -1 )
		    last = last->get_prev_leaf();
            }

	    ldr_region_delete (frst, last);

	    obj_reg->start->subnode = checked_cast(objTree,frst);
            obj_reg->end->subnode = checked_cast(objTree,last);
	}
	return;
    }

    steDisplayNode * tl = checked_cast(steDisplayNode,smt_find_ldr(this, to));
    int format = (oi->pt) ? 1 : 0;
    OperPoint *opnt = ( (int) oi->pt == 1 ) ? 0 : (OperPoint *)oi->pt;
    int off = (opnt) ? opnt->offset_of : -1;
    
//
// Insert inside of a token case
    if (type == AFTER && off > 0 && tl && !tl->get_first()) {
	steDisplayNodePtr dn = db_new(steDisplayNode,());
	dn->set_node_type(steREG);
	tl->put_after( dn );
	appTreePtr next_ap = checked_cast(appTree,oi->data);
	if ( next_ap ) {
	    apptree_put_ldrtree(next_ap, dn);
	    ni->data = (void *) dn;
	}else
	    ni->type = NULLOP;
    } else if ( (type == AFTER || type == FIRST) && so && // one token insert
		!so->get_first() && d && tl && format ) {
	steDisplayNodePtr dn = db_new(steDisplayNode,());
	dn->set_node_type(steREG);
	if ( type == FIRST )
	    tl->put_first(dn);
	else
	    tl->put_after(dn);

	apptree_put_ldrtree(so, dn);
	ni->targ_obj = tl;
	ni->src_obj = dn;
	return;
    }  

    if(tl == 0)
	tl = checked_cast(steDisplayNode,get_root());
    smtTree * nto = checked_cast(smtTree,ldrtree_get_apptree(tl));
    
// under certain conditions (such as splice operations and the like) it is
// possible that smt_find_ldr will return 0. get_root then will not but
// the smtTree associated with the root has lost its pointers due to a 
// previous obj_delete operation. we defend against that here.
    if (nto->get_id() < 0 || !nto->get_header()) nto = 0;
  
    if(nto == 0)
	obj_delete(this);
    if (nto != to) {
	type = REPLACE;
	so = to = nto;
	d = 0;
    }

    if ((type == AFTER || type == FIRST) && tl && so) {
	steDisplayNodePtr dn = do_extract(so, steREG);
	ni->targ_obj = tl;
	ni->src_obj = dn;
	if (dn) {
	    if ( type == AFTER )
		tl->put_after( dn );
	    else
		tl->put_first( dn );
	}else
	    ni->type = NULLOP;
	return;
    }
 
    if (tl == 0 || to == 0 || type != REPLACE || (so == 0 && d == 0)) {
	ni->type = NULLOP;
	return;
    }

    if(oi->data && !tl->get_first() && so == 0 && d ) {
	// One token changed: do not change LDR and propagate to VIEW
	ni->targ_obj = tl;
	return;
    }

    ni->data = 0;
    ni->targ_obj = ni->src_obj = tl;
    ni->type = REPLACE;
    
    // Create LDR for new subtree and replace
    ni->src_obj = do_extract(to, (steNodeType)tl->get_node_type());
    ni->targ_obj = tl;
    ni->type = REPLACE;
    if(tl->get_parent()) {
	tl->put_after(ni->src_obj);
	tl->remove_from_tree();
    } else
	put_root(ni->src_obj);

    obj_delete (tl);

#if 0
    // Let's try to regenerate view without text touching
    if (so == to) {
	ni->type = SPLIT;
	ni->data = (void *)1;    // non ZERO data is a SPLIT flag;
    }
#endif
}

objTree * smt_find_ldr (ldr * lh, objTree * an)
{
    Initialize(smt_find_ldr);

    if (an == NULL)
	return NULL;

    objTree *found = lh->find_ldr( an );
    if ( found )
	return found;

    ldrTree * lr = checked_cast(ldrTree,lh->get_root()); 
    appTree * alr = ldrtree_get_apptree(lr);

// If apptree of the root is a reference node continue with the
// referenced node (H)
    if (alr && is_ReferenceNode(alr)) {
	alr = get_ReferenceRoot(alr);
	if (!alr) return NULL;
	// check for root case because root does not have a direct
	// relation to the app tree (apptree_of_ldrtree points to
	// the reference node, not the reference root). -jef
	if (alr == an)
	    return lr;
    }

    if(alr == 0 ||alr->subtree_of(an))
	return lr;		// Whole LDR should be regenerated
    else if(an->subtree_of(alr)) {
	for(; an; an = an->get_parent()) {
	    if ( (found = lh->find_ldr( an )) ) 
		return found;
	}
	return 0;		// Boris, please check this one
    }else
	return 0;
}

void ldrSmtSte::find_selection(const OperPoint& pt, ldrSelection& ls){
 Initialize(steSmtSte::find_selection);
   ls.offset = pt.offset_of;
   appTreePtr apn = (symbolPtr) pt.node;
   if ( apn )
      ls.ldr_node = checked_cast(ldrTree,smt_find_ldr( this, apn ));
   else
      ls.ldr_node = NULL;
}

void ldrSmtSte::build_selection(const ldrSelection& ls, OperPoint& pt){
 Initialize(ldrSmtSte::build_selection);
   pt.offset_of = ls.offset;
   appTreePtr apn = NULL;
   for ( ldrTreePtr cur = ls.ldr_node ; cur ; 
             cur = checked_cast(ldrTree,cur->get_parent()) ){
       apn = ldrtree_get_apptree( cur );
       if ( apn )
          break;
   }
   pt.node = apn;
}

/*
   START-LOG-------------------------------------------

   $Log: ldrSmtSte_h.cxx  $
   Revision 1.5 1997/05/22 09:44:13EDT twight 
   Change '& const' into 'const &'.
Revision 1.2.1.21  1993/10/05  00:58:17  boris
Bug track: Testing
Fixed smoke/test_smod_cut/test.ccc to succeed

Revision 1.2.1.20  1993/10/01  17:38:27  kws
Port

Revision 1.2.1.19  1993/08/25  00:37:01  boris
Fixed hanging bug #4503, with buffer disappearing

Revision 1.2.1.18  1993/08/22  19:19:27  boris
fixed bug #4443 with wrong zone message

Revision 1.2.1.17  1993/08/03  23:31:15  boris
added REPLACE_REGION functionality

Revision 1.2.1.16  1993/06/23  18:01:25  boris
Fixed bug #3804 with outline view blinking

Revision 1.2.1.15  1993/06/23  16:14:15  boris
Added zero checking in smt_find_ldr()

Revision 1.2.1.14  1993/04/23  17:34:23  davea
bug 3482 - added return to smt_find_ldr()

Revision 1.2.1.13  1993/03/26  04:39:05  boris
Skip Smod Split call

Revision 1.2.1.12  1993/02/19  21:04:10  boris
Fixed bug #2500

Revision 1.2.1.11  1993/02/17  20:31:38  boris
Fixed crash in a second smodView of a file.

Revision 1.2.1.10  1993/02/16  23:20:54  boris
Added _paste on insert_obj() without ldr tree regeneration

Revision 1.2.1.9  1993/02/12  17:09:13  boris
Added smt cut without view regeneration

Revision 1.2.1.8  1992/12/17  14:43:34  jon
Updated smt_find_ldr to understand references.

Revision 1.2.1.7  1992/12/02  01:15:37  boris
*** empty log message ***

Revision 1.2.1.6  1992/11/21  22:54:33  builder
typesafe casts.

Revision 1.2.1.5  1992/11/11  00:06:55  boris
Added temp zone case.

Revision 1.2.1.4  1992/11/10  16:21:30  sharris
(BUG 1295) defend against use of previously deleted smt nodes

Revision 1.2.1.3  1992/11/05  20:48:02  boris
Fixed replace text on a token

Revision 1.2.1.2  1992/10/09  19:44:01  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
