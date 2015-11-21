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
///////////////////////  FILE ldrSmodShort_h.C  ////////////////////////
//
#include <SmtSte.h>
#include <genError.h>
#include <objRelation.h>
#include <ldrSmodShort.h>
#include <ldrSmodNode.h>
#include <reference.h>
#include <ste_view.h>
#include <lde_smod_short.h>
#include <smt.h>
#include <ste_smod.h>
#include <steHeader.h>

#ifndef ISO_CPP_HEADERS
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <strstream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

init_relational(ldrSmodShort,ldrSmtSte);

ldrSmodShort::ldrSmodShort(smtHeaderPtr smt_app, smtTreePtr root)     
{
    Initialize(ldrSmodShort::ldrSmodShort);

    smtTreePtr text_root;
    text_root = (root != NULL) ? root : checked_cast(smtTree,smt_app->get_root());

    if ( text_root->smod_vision == SmodZero )
	text_root->smod_vision = SmodLocal;

    app_put_ldr(smt_app, this);
    set_type(Rep_SmtText);

    steDisplayNode * new_display = do_extract(text_root, steSTR);

    put_root(new_display);

    if (text_root->get_parent())      // set read only if sub root
	set_read_only(1);
}

ldrTree *ldrSmodShort::generate_tree(appTree * t, int tp)
{
  Initialize(ldrSmodShort::generate_tree);
  steDisplayNodePtr dnd = NULL;
  if ( t ){
     smtTreePtr smt_t = checked_cast(smtTree,t);
     dnd = do_extract( smt_t, (steNodeType)tp);
  }
  return dnd;
}

steDisplayNodePtr ldrSmodShort::do_extract( smtTreePtr root, steNodeType tp)
{
  Initialize(ldrSmodShort::do_extract);
  
  IF(root->get_parent());

  steDisplayNodePtr ans = NULL;
  ans = db_new(steDisplayNode,(steSTR)); 
  ans->set_appTree( root );
  root->smod_vision = SmodLocal;
  ans->put_first( db_new(ldrSmodNode,(root,root)) );
  return ans;

#ifdef PMG
  steDisplayNodePtr dnd = NULL;
  if ( smod_vision == SmodFull )
     dnd = ldrSmtSte::do_extract( root, tp );
  else if ( root ){
     smtHeaderPtr h = checked_cast(smtHeader,(ldr_get_app(this)));
     if ( h ){
        int unparse_flag = h->test_status() & 0x01; // 1 - parse is needed
        (void) set_visible( root, unparse_flag );
        dnd = lde_smod_short_extract(root, tp);
#if 0
        if ( h->arn && root->arn == 0 ){   // set raw_file if parent's arn ==1
           for ( smtTreePtr cur = root->get_parent() ;
                 cur ; 
                 cur = cur->get_parent() ){
              if ( cur->arn == 1 ){
                 dnd->raw_flag = 1;
                 break;
              }
           }
        }
#endif
     }
  }
  return dnd;
#endif
}

// Sets visible field value to flag
//
//
void ldrSmodShort::put_visible( smtTreePtr root, int flag ){
   underInitialize(ldrSmodShort::put_visible);
     root->temp_vis = flag;
     objTreePtr cur = root->get_first();
     for( ; cur ; cur = cur->get_next() )
        put_visible( checked_cast(smtTree,cur), flag);
}

// Sets temp_vis flag to 1 for nodes that will get extracted and
// 0 for others. Uses temp_vis field on smtTree
//
int ldrSmodShort::set_visible( smtTreePtr root, int unparse_flag )
{
  underInitialize(ldrSmodShort::set_visible);

  if ( root == NULL )
     return 0;

  root->temp_vis = 0;    // invisible by default;

  if ( root->smod_vision == SmodFull ){
     put_visible( root, 1 );
     return 1;
  }

  if ( check_visible_relations(root) && root->smod_vision == SmodZero )
      root->smod_vision = SmodLocal;

  if ( check_visibility(root, unparse_flag) ) //There is some local attention
      root->temp_vis = 1;

  objTreePtr cur = root->get_first();
  int child_flag = 0;
  for ( ; cur ; cur = cur->get_next() ){
     if ( set_visible(checked_cast(smtTree,cur), unparse_flag) )
        child_flag = 1;
  }

  if ( child_flag )
     root->temp_vis = 1;

  return root->temp_vis;
}

// Seeks for region under dnd, that includes st point.
// en is not used so far; returns ldrSmodNodePtr casted to
// steDisplayNodePtr or NULL if not found.
//
steDisplayNodePtr
ldrSmodShort::tree_search( steDisplayNodePtr dnd, int st, int en)
{
    Initialize(ldrSmodShort::tree_search);
 
    if (!dnd || dnd->get_id() < 0 || !dnd->get_header())
	return 0;

    smtTreePtr t_st = checked_cast(smtTree,ldrtree_get_apptree( dnd ));
    if ( t_st ){
	int cur_st = t_st->start();
	int cur_en = cur_st + t_st->length();
	smtTreePtr end_nd;
	if ( is_ldrSmodNode( dnd ) && (end_nd = ((ldrSmodNode *)dnd)->end_smt())){  
	    cur_en = end_nd->start() + end_nd->length();
	    if ( st >= cur_st && st < cur_en )
		return dnd;
	}else{
	    if ( st < cur_st || st >= cur_en )
		return 0;
	}
    }

    steDisplayNodePtr ans = NULL;
    for ( objTreePtr cur = dnd->get_first() ; cur ; cur = cur->get_next() ){
	ans = tree_search( checked_cast(steDisplayNode,cur), st, en );
	if ( ans )
	    break;
    }
    return ans;
}

// Seeks for SmodRegion that includes smtTreePtr nd start.
//
//
steDisplayNodePtr ldrSmodShort::region_search( smtTreePtr nd ){
 Initialize(ldrSmodShort::region_search);
   if (nd == NULL || nd->get_id() < 0 || !nd->get_header()) 
      return NULL;

   int nd_start = nd->start();
   int nd_end   = nd_start + nd->length();

   steDisplayNodePtr root = checked_cast(steDisplayNode,get_root());
   steDisplayNodePtr dnd = tree_search( root, nd_start, nd_end );

   return dnd;
}

int ldrSmodShort::visible_split( void *reg, steDisplayNodePtr& src, steDisplayNodePtr& trg )
{
    Initialize(ldrSmodShort::visible_split);
    smodRegion *s_reg = (smodRegion *)reg;
    smtTreePtr st = checked_cast(smtTree,s_reg->start->node);
 
    ldrTree *l_root = checked_cast(ldrTree,get_root());
    smtTree * rt = (l_root) ? checked_cast(smtTree,ldrtree_get_apptree(l_root)) : NULL;

    if (! st || !rt )
	return -1;

    if (rt != st && rt->subtree_of(st))
	st = rt;

    objTreePtr ww = find_ldr( st );
    trg = checked_cast(steDisplayNode,ww);

    if (trg && trg->smod_vision != SmodRegion) {
	if ( !st->get_first() ){ // no need: leaf is alredy built
	    if ( !trg->smod_vision )
		trg->smod_vision = s_reg->visibility;

	    return 0;
	}

    SmodVisibility ldr_v, new_v, smt_v;
    ldr_v = smod_vision_convert( trg->smod_vision );
    new_v = smod_vision_convert( s_reg->visibility );
    smt_v = smod_vision_convert( st->smod_vision );
    
    if (ldr_v == new_v && ldr_v == smt_v) {
	trg->smod_vision = s_reg->visibility;
	return 0;		// no need: tree is alredy built;
    }
    } 

    if (!trg || trg->smod_vision == SmodRegion) {
	trg = checked_cast(steDisplayNode,find_ldr_to_rebuild( st ));
	st = (trg) ? checked_cast(smtTree,ldrtree_get_apptree(trg)) : NULL;
    }

    if (!st || !trg)
	return -1;

    // Create LDR for new subtree and replace
    src = do_extract(st, (steNodeType)trg->get_node_type());

    if(trg->get_parent()) {
	trg->put_after(src);
	trg->remove_from_tree();
    } else
	put_root(src);

    obj_delete(trg);
    return 1;
}

// Is Used for REGENERATE (REPLACE with src and trg )
// Seeks for "visible" ldr tree node with apptree_of_ldrtree relation.
//
objTree * ldrSmodShort::find_ldr_to_rebuild( objTree *tn ){
 Initialize(ldrSmodShort::find_ldr_to_rebuild);
   steDisplayNodePtr ans = NULL;
   steDisplayNodePtr ldr_root = checked_cast(steDisplayNode,get_root());
   objTree *root = ( ldr_root ) ? ldrtree_get_apptree(ldr_root) : NULL;

   if ( root && tn ){
      if ( tn->subtree_of(root) ){
         for ( ; tn ; tn = tn->get_parent() ) {
             ans = checked_cast(steDisplayNode,find_ldr( tn )) ;
             if ( tn == root )
                break;

             if ( ans == NULL || ans->smod_vision == SmodRegion )
                continue;

             if ( ans->get_first() && ans->smod_vision == SmodZero )
                continue;

             break;
         }
      }else if ( root->subtree_of( tn ) )
         ans = ldr_root;
   }
   return ans;
}

void ldrSmodShort::insert_obj(objInserter *oi, objInserter *ni)
{
    Initialize(ldrSmodShort::insert_obj);
    int ok_flag = 1;
    if (oi->src_obj)
	if (!is_smtTree(oi->src_obj))
	    ok_flag = 0;
  
    if (oi->targ_obj)
	if (!is_smtTree(oi->targ_obj))
	    ok_flag = 0;
	 
    if (ok_flag == 0) {
	steHeader(Message)
	    << "ldrSmodShort::insert_obj - not smtTree node on input."
	    << steEnd;
	ni->type = NULLOP;
	return;
    }

    smtTree * so = checked_cast(smtTree,oi->src_obj); // the source SMT node
    smtTree * to = checked_cast(smtTree,oi->targ_obj); // the target SMT node

    ldrTree *ldr_root = checked_cast(ldrTree,get_root ());

    ldrSmodNode *ldr_frst = checked_cast(ldrSmodNode,ldr_root->get_first());
    
    app *h = get_appHeader();
    appTree *rt   = (h)  ? checked_cast(appTree,h->get_root()) : NULL;
    smtTreePtr frst = (rt) ? checked_cast(smtTree,rt->get_first_leaf()) : NULL;
    smtTreePtr lst  = (rt) ? checked_cast(smtTree,rt->get_last_leaf()) : NULL;
    
    // Reset all basic ldr relations.
    if (rt && frst && lst && ldr_frst) {
	put_relation(ldrtree_of_apptree, rt, ldr_root);
	put_relation(ldrtree_of_apptree, frst, ldr_frst );
	ldr_frst->end_smt(lst);
    }

    // No spliting any more.
    if(oi->type == SPLIT )
    {
	ni->type = NULLOP;
	return;
    }

    // If this change originated from editor STOP.  Editor already 
    // knows about change!
    // The difference between this and the scripting mechanism is that 
    // opt->view is  null when running a script.
    if (oi->type == REPLACE_REGION && oi->pt && oi->data )
    {
	if( ((OperPoint*)oi->pt)->view_node )
	{
	    ni->type = NULLOP;
	    return;
	}
	// SCRIPTING MECHANISM.
	ni->targ_obj = ni->src_obj = ldr_root->get_first();
	OperPoint *opt = (OperPoint *)ni->pt;
	opt->node = to;
	opt->x =  to->oldStart;                // from
	opt->y =  opt->x + to->oldLength;      // to
	return;
    }

    // "rebuild ldr with out text".  This was only done when the text was 
    // correct but the zones for ldr and view node needed to be generated.
    if (oi->type == REPLACE && ni->pt == (void*)1)
    {
	ni->type = NULLOP;
	return;
    }


    // From this point forward all operations are via change propigation
    //   forward engineering, or merging.
    ni->src_obj = 0;
    ni->targ_obj = ldr_root->get_first();

    ni->type = REPLACE_REGION;
    ni->pt = new OperPoint;    //delete it after all, please.
    
    OperPoint *opt = (OperPoint *)ni->pt;
    opt->type = NULLOP;  // This flag indicates that it should be deleted in steView::insert_obj().
    if(oi->type == FIRST &&                   //first smod_replace_remove from emacs
       oi->pt && oi->pt != (void*)1)          //And a real oper pt is provided.
	opt->view_node = ((OperPoint*)oi->pt)->view_node; //Allow the various nodes to be created, but
                                              //Send change to emacs.

    opt->node = to;
    opt->subnode = so;

    if (oi->type == REMOVE) { 
	opt->x =  to->oldStart;    // from
	if( to && !to->get_parent() )
	  opt->y =  -1;
	else
	  opt->y =  opt->x + to->oldLength;      // to
	ni->data = 0;
    } else if (oi->type == REPLACE) {
	// REPLACE_REGION can only be true here if being reported from test
	// scripts.
	opt->x =  to->oldStart;    // from
	if( to && !to->get_parent() )
	  opt->y =  -1;	
	else
	  opt->y =  opt->x + to->oldLength;      // to
	if (oi->data)
	    ni->data = oi->data;
	else {
	    if (so == 0) {
		ni->type = NULLOP;
		return;
	    }

	}
    } else if (oi->type == BEFORE || oi->type == FIRST || oi->type == AFTER ) {
	opt->x = opt->y = to->oldStart;
	if(oi->type == AFTER)
	{
	    int len = to->oldLength;
	    opt->x += len;
	    opt->y += len;
	}

	if (oi->data)
	    ni->data = oi->data;
	else {
	    if (so == 0) {
		ni->type = NULLOP;
		return;
	    }
	}
    }
    else
    {
	ni->type = NULLOP;
	return;
    }
}

void ldrSmodShort::find_selection(const OperPoint& pt, ldrSelection& ls)
{
    Initialize(steSmodShort::find_selection);
    int cur_off = pt.offset_of;
    smtTreePtr apn = checked_cast(smtTree, (symbolPtr) pt.node);

// Let's convert to leaf  
    apn = smod_leaf_offset(apn, pt.offset_of, cur_off);
    steDisplayNodePtr dnd = (apn) ? checked_cast(steDisplayNode,find_ldr(apn))
	: NULL;

    if (dnd && !is_ldrSmodNode(dnd)) {
	ls.ldr_node = dnd;
	ls.offset = cur_off;
	return;
    }

    appPtr appl1 = ldr_get_app(this);
    appPtr appl2 = checked_cast(app,apn->get_header());

    if (appl1 && appl1 == appl2)
    {
	steDisplayNodePtr rt = checked_cast(steDisplayNode,get_root());
	int st = apn->start();
	dnd = tree_search(rt, st, 1);
	if ( dnd && is_ldrSmodNode(dnd) ){
	    smtTreePtr reg_smt = checked_cast(smtTree,ldrtree_get_apptree( dnd ));
	    if ( reg_smt == NULL )
		dnd = NULL;
	    else if ( cur_off != -1 )
		cur_off = st - reg_smt->start() + cur_off;
	    else if ( apn == (checked_cast(ldrSmodNode,dnd))->end_smt() )
		cur_off = -1;
	    else   
		cur_off = st - reg_smt->start() + apn->length();;
	} 
	ls.offset = cur_off;
    }
    ls.ldr_node = dnd;
}

void ldrSmodShort::build_selection(const ldrSelection& ls, OperPoint& pt){
 Initialize(ldrSmodShort::build_selection);
   int cur_off = ls.offset;
   smtTreePtr apn = NULL;

   if ( ls.ldr_node == NULL ){
      pt.node = (Obj *)apn;
      pt.offset_of = cur_off;
      return;
   }

   if ( ls.ldr_node->smod_vision == SmodRegion ){
      if ( ls.offset < 0 ){
         apn = (checked_cast(ldrSmodNode,ls.ldr_node))->end_smt();
      }else{
         apn =  checked_cast(smtTree,ldrtree_get_apptree( ls.ldr_node ));
         apn = smod_leaf_offset( apn, ls.offset, cur_off );
      }
      pt.node = (Obj *)apn;
      pt.offset_of = cur_off;
   }else
      ldrSmtSte::build_selection( ls, pt);
}

ldrSmodNodePtr ldrSmodShort::find_region (smtTreePtr st, steDisplayNodePtr rt)
{
    Initialize(ldrSmodShort::find_region);
    ldrSmodNodePtr ans = NULL;
    rt = (rt) ? rt : checked_cast(steDisplayNode,get_root());
    if (st && rt) {
	if (!rt->get_first()) {
	    if (is_ldrSmodNode(rt)) {
		smtTreePtr t = checked_cast(smtTree,ldrtree_get_apptree(rt));
		if (t == st)
		    ans =  checked_cast(ldrSmodNode,rt);
	    }
	} else {
	    steDisplayNodePtr cur;
	    for (cur = checked_cast(steDisplayNode,rt->get_first()) ;
		 cur ;
		 cur = checked_cast(steDisplayNode,cur->get_next()))
		if (ans = find_region (st, cur))
		    break;
	}
    }
    return ans;
}

/*
   START-LOG-------------------------------------------

   $Log: ldrSmodShort.cxx  $
   Revision 1.10 2000/07/10 23:06:04EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
// Revision 1.20  1994/04/06  20:33:31  boris
// Fixed bug6723 with C-w inside of a function
//
// Revision 1.19  1994/02/16  15:23:45  boris
// Bug track: #6248
// Move ste_finalize() before real_stmt execution
//
// Revision 1.18  1994/01/26  16:23:51  boris
// Bug track: #4782
// Fixed new Document creation
//
// Revision 1.17  1994/01/18  21:23:49  boris
// Bug track: Test
// Fixed SMOD region creation
//
// Revision 1.16  1994/01/15  23:49:18  boris
// Bug track: Test
// Fixed ParaTutor
//
// Revision 1.15  1993/12/20  23:32:59  mg
// Bug track: 1
// underInitialize - mini version of Initialize
//
// Revision 1.14  1993/11/06  22:05:09  so
// Bug track: n/a
// driver_instance->operation without viewshell
// This is to support -batch to merge two files.
// Done by boris
//
// Revision 1.13  1993/08/22  19:19:27  boris
// fixed bug #4443 with wrong zone message
//
// Revision 1.12  1993/08/03  23:30:08  boris
// added REPLACE_REGION functionality
//
// Revision 1.11  1993/07/16  21:41:32  so
// fix bug 4018 by boris
//
// Revision 1.10  1993/07/09  00:46:26  boris
// Fixed bug #3942 with wrong input node type in ldrSmodShort::insert_obj()
//
// Revision 1.9  1993/06/25  20:41:49  bakshi
// typecasts to resolve c++3.0.1 ambiguity
//
// Revision 1.8  1993/05/08  01:49:13  boris
// Fixed cut/paste problem
//
// Revision 1.7  1993/04/04  23:26:15  boris
// Used ndm flag for "Reparse" category
//
// Revision 1.6  1993/03/31  01:49:44  boris
// Fixed crash in region creating
//
// Revision 1.5  1993/03/30  21:50:00  boris
// fixed grey area is SMOD
//
// Revision 1.4  1993/03/30  18:51:25  jon
// Fixed category node assigning -boris
//
// Revision 1.3  1993/03/29  00:25:54  boris
// Added SmodFull handling on ldrSmodShort header
//
// Revision 1.2  1993/03/26  21:17:05  boris
// fixed ::visibility_split() to use find_ldr_to_rebuild()
//
// Revision 1.1  1993/03/26  04:39:05  boris
// Initial revision
//

   END-LOG---------------------------------------------

*/
