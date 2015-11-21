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
// File smt_modify.C

#include "objRelation.h"   // for debugging sake.
#include "objOper.h"
#include "smt.h"
#include "SmtSte.h"
#include "astTree.h"

#ifndef ISO_CPP_HEADERS
#include <ctype.h>
#else /* ISO_CPP_HEADERS */
#include <cctype>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#include <OperPoint.h>
#include <ste_smod.h>

#include <objCollection.h>


void smt_real_split( smtTree *trg, int off, smtTree * );

boolean smt_is_single_statement(smtTree *);
static void remove_stmt(smtHeader *, smtTree *, int do_tidy);
static smtTree * find_ins_down(smtTree *);
static smtTree * find_ins_after(smtTree *);
static void make_not_single(smtTree *);
static void cor_tbeg(smtTree *, int);


static smtTree* f_ins_after (smtTree* t)
{
    smtTree* x = t;
    for (; x && !x->get_next (); x = x->get_parent ())
	; 

    if (!x) {
	x = (smtTree*)t->get_root ();
	x = x->get_first ();
	if (x) {
	    smtTree* y;
	    for (; y = x->get_next (); x = y)
		;
	}
    }
    
    return x;
}

static int char_is_separator(int ch)
{
  switch(ch){
    case ' ':
    case '\t':
    case '\n':
    case ';':
    case ',':
    case '?':
    case '~':
    case '}':
    case '{':
    case '(':
    case ')':
    case '[':
    case ']':
      return 1;

    default:
      ;
  }
  return 0;
}
void smtHeader::obj_remove_light(smtTree* node, int flags)
{
   objRemover ro(node, flags);
   objRemover nr(ro);
   remove_obj_real(&ro, &nr, 0);
}
void smtHeader::obj_insert_light(objOperType tp, smtTree*src, 
      smtTree*trg, void*data, void*d1)
{
  objInserter os(tp,src,trg,data, d1);
  objInserter ns(os);
  insert_obj_real(&os, &ns, 0);
}

objSplitter* smtHeader::split_obj(objSplitter *os, objSplitter * ns)
  {
  Initialize(smtHeader:split_obj);
  smtTree* targ_node = checked_cast(smtTree, os->bottom_obj);
  smtTree* src_node  = checked_cast(smtTree, os->top_obj);
  obj_insert(this, SPLIT, src_node, targ_node, NULL);
  ns->bottom_obj = ns->top_obj = 0;
  ReturnValue(0);
  }

objMerger* smtHeader::merge_obj(objMerger *om, objMerger *nm)
  {
  Initialize(smtHeader::merge_obj);
  smtTree* targ_node = checked_cast(smtTree, om->targ_obj);
  smtTree* src_node  = checked_cast(smtTree, om->src_obj);
  obj_insert(this, MERGE, src_node, targ_node, NULL);
  nm->src_obj = nm->targ_obj = 0;
  ReturnValue(0);
  }


void do_touch_ddElement(smtTree * trg);

void smtHeader::set_touched_ddElements(smtTree *trg)
{
    Initialize(smtHeader::set_touched_ddElements);
    smtTree *prev = 0;
    while(trg->get_parent() != 0)
    {
      if (prev && 
	  prev->get_node_type() != SMT_decl && prev->get_node_type() != SMT_fdecl &&
	  trg->get_node_type() == SMT_list_decl) {
	for (smtTree *ch = trg->get_first(); ch;
	     ch = ch->get_next())
	  do_touch_ddElement(ch);
      }
      do_touch_ddElement(trg);
      prev = trg;
      trg = checked_cast(smtTree,trg->get_parent());
    }
}

void smtHeader::insert_obj(objInserter * oi, objInserter * ni)
{
  Initialize(smtHeader::insert_obj);
  insert_obj_real(oi, ni, 1);
}

void smtHeader::insert_obj_real(objInserter * oi, objInserter * ni, int do_tidy)
{
  Initialize(smtHeader::insert_obj_real);
  smtTree * src = checked_cast(smtTree,oi->src_obj);
  smtTree * trg = checked_cast(smtTree,oi->targ_obj);
  objOperType oper = oi->type;
  
  smtTreePtr built_trg = NULL;   // keeps smod_rebuilt trg;

  if ( src == 0 && trg == 0 && oper == SPLIT ){
     smod_set_visibility( oi->data );
     return;
  }
  int format = (oi->pt) ? 1 : 0;
  
  if ( trg && trg->get_header() != this )
     return;

  // call special handler for replace text

  if(oper==REPLACE && src==NULL && oi->data )
  {
      if ( !format ){
	  ni->type = NULLOP;
          vrn = 1;     // view is going to be rebuilt at the very end
          trg->smod_vision = SmodChange;
      }else{
          vrn = 0;     // instant view propagation
          built_trg = trg;
          smod_rebuild_node (trg, SmodChange, do_tidy);
      }    
      ni->targ_obj = smt_replace_text(trg, (char *)oi->data, format, do_tidy);
      set_modified();
      set_src_modified ();
      return;
  }

  if (oper == REMOVE){
     if (trg == 0 || oi->data == 0) {
        built_trg = src;
        smod_rebuild_node (src, SmodChange, do_tidy);
        smt_cut_region(0, src, -1, src, do_tidy);
	set_modified();
	set_src_modified ();
    } else {
        objRegion *obj_reg = (objRegion *)oi->data;
        if ( obj_reg->start && obj_reg->end ){
	    built_trg = src;
	    smod_rebuild_node(src, SmodChange, do_tidy);
	    if ( trg != src )
		smod_rebuild_node(trg, SmodChange, do_tidy);
	    smt_cut_region(obj_reg->start->offset_of, src,
			   obj_reg->end->offset_of,   trg, do_tidy);
	    set_modified();
	    set_src_modified ();
	}
    }

     vrn = 0;
     return;
  }

  OperPoint *opnt = ( (int) oi->pt == 1 ) ? 0 : (OperPoint *)oi->pt;
  int off = (opnt) ? opnt->offset_of : -1;
  char * data = (char *)oi->data;

  // Replace REGION from src to trg (including trg) to new data;
  if (oper == REPLACE_REGION) {                
      smtTreePtr st_cut = src->get_first_leaf(),
	         en_cut = trg->get_last_leaf();
      if (src && trg && data && oi->pt && (ni->src_obj = st_cut) && 
	  (ni->targ_obj = en_cut)) {
	  OperPoint *opt = (OperPoint *)oi->pt;
	  // if REPLACE with string
	  if (data[0]) {
	      if(en_cut)
		  smod_rebuild_node(en_cut, SmodChange, do_tidy);
	      smt_replace_text (st_cut, data, 1, do_tidy);
	      if (st_cut == en_cut)
		  st_cut = en_cut = NULL;
	      else 
		  st_cut = st_cut->get_next_leaf();
	      
	      // if cut to the end of a REGION (move end of the REGION)
	      if (en_cut == checked_cast(smtTree,opt->subnode))
		  opt->subtype = (int) st_cut->get_prev_leaf();
		  
          } else {  // no text, just CUT it all
	      if (st_cut == checked_cast(smtTree,opt->node) && 
		  en_cut != checked_cast(smtTree,opt->subnode))
		  opt->subtype = (int) en_cut->get_next_leaf();
	      else if (st_cut != checked_cast(smtTree,opt->node) && 
		  en_cut == checked_cast(smtTree,opt->subnode))
		  opt->subtype = (int) st_cut->get_prev_leaf();
	  }

	  objTree *com_root = (ni->src_obj == ni->targ_obj) ? ni->src_obj
	      : obj_tree_common_root (ni->src_obj, ni->targ_obj);
	  
	  // Cludge (one of many): put common root in offset_of of OperPoint opt
	  // in order to preserve common root in the case of cutting region
	  opt->offset_of = (int) com_root;
	  
	  // save alive parent pointer (if any)
	  objTree *prnt = com_root->get_parent();
	  
	  if (st_cut && en_cut)
	      smt_cut_region (0, st_cut, -1, en_cut, do_tidy);
	  else
	      if(do_tidy) tidy_tree ();
	  
	  // make sure that com_root in opt->offset_of is alive after cut
	  if (prnt && !com_root->get_parent())
	    opt->offset_of = (int) prnt;
	  
	  vrn = 0;
	  return;
      }
      ni->type = NULLOP;
      return;
  }

  if ( trg && !(oper == REPLACE && src == trg) ){
      built_trg = trg;
      smod_rebuild_node( trg, SmodChange, do_tidy );
  }

  if ( (oper == AFTER || oper == FIRST) && !trg->get_first() && off >= 0 ) {
      if ( off == 0 ) {
	  oper = BEFORE;
	  ni->data = 0;
      } else {
	  smt_real_split(trg, off, src);
	  ni->data = (void *)trg->get_next();
	  oper = AFTER;
      }
      vrn = 0;
  }

  smtExtModifier * em = 0;
  int node_type = 0;
  smtTree * toknode = 0;
  int ref = 0;
  smtTree * partrg, * temp;
  int trg_was_statement = 0;

  // call special handler for smod insert text

  if( (oper==FIRST || oper== AFTER) && src==NULL && format && data)
  {
      src = db_new(smtTree,());
      if ( oper == FIRST )
         trg->put_first(src);
      else
         trg->put_after(src);

      ni->src_obj = smt_replace_text(src, data, format, do_tidy);
      set_modified();
      set_src_modified ();
      return;
  }

  // Extract arguments from smtExtModifier if type = EXT_MODIFY
  if(oper == EXT_MODIFY)
    {
    em = (smtExtModifier *)data;
    if(em->node_type != SMT_token && em->node_type != SMT_group)
      node_type = em->node_type;
    data = (char *)em->data;
    oper = em->type;
    ref = em->ref;
    if (ref) 
      {
      get_reference_status (&trg, &oper);
      if (!is_correct(checked_cast(appTree,data), checked_cast(appTree,trg)) )
        {
        ni->type = NULLOP;
        return;
        }
      }
    }


// Return if it is a call to regenerate view
  if(oper == REPLACE && src == trg) {
      // Smod_rebuild_node() sets fields oldStart and oldLength
      // for updating of emacs buffer.
      smod_rebuild_node (trg, SmodChange, do_tidy);

      return;
  }

  set_modified();
  set_src_modified ();
  drn = 1;
  set_arn(1);

  // Recognize some special cases
  switch(oper)
    {
    case MERGE:		// It is very special case
      smtTree * tleft, * tright, *tpar;
      Assert(trg && src && trg != src);
      tpar = checked_cast(smtTree,obj_tree_common_root(trg, src, 
                                  (objTree **)&tleft, (objTree **)&tright));
      Assert(tleft->tnmax < tright->tnfirst);
      obj_remove(this, tleft, 0x01);	// Remove LDR node for tleft
      tright->put_first(tleft);		// Put tleft under tright
      smt_splice(tleft);		// Kill redundant node
      smt_touch(tright, 1);

      // make new inserter to regenerate LDR/VIEW for new compound node
      ni->type = REPLACE;
      ni->src_obj = tright;   
      ni->targ_obj = tright;
      return;

    case SPLIT:
      trg = src;
      src = 0;
      data = "\r";
      oper = BEFORE;
      // fall down to "BEFORE" case

    case BEFORE:
      if(!em && trg->type != SMT_token && trg->type != SMT_group)
	{
	// structured before
	smtTree * w;
	while(!smt_is_statement(trg))
	  {
	  w = checked_cast(smtTree,trg->get_parent());
	  if(w == 0 || w->type == SMT_file)
	    break;
	  trg = w;
	  }
	}
      make_not_single(trg);
      while (!(temp = trg->get_prev())) {
	if (trg->get_parent())
	    trg = trg->get_parent();
	else {
	    temp = 0;
	    break;
	}
      }
      if(temp)	{
	trg = temp;
	oper = AFTER;
      }else{
	oper = FIRST;
      }
      break;
    }

  // Test target and find appropriate insertion point if it is a
  // structured insertion
  if(format == 0 && em == 0 && 
     (oper == FIRST || oper == AFTER) &&
     ((data == 0 && smt_is_statement(src)) ||
      (data && (data[1] != 0 || (*data != '\r' && *data != '\n'))))
    )
    switch(oper)
      {
      case FIRST:
        if((temp = find_ins_down(trg)) != 0)
          trg = temp;
        else
          {
          if(!smt_is_statement(trg))
 	    return;
          }
        make_not_single(trg);

//      boris 970625: I think that this is a mistake. There will be a deliberate
//                     check for the type "FIRST". Let us preserve it.
//	oper = AFTER;

        break;
      case AFTER:
        if(trg->type == SMT_token || trg->type == SMT_group)
	  break;				// Non structured insert
        if((trg = find_ins_after(trg)) == 0 || trg == src)
          return;
        make_not_single(trg);
      // Fall down

    default:
        break;
      }

  // not to touch group if it is empty line is inserting after its last token
  if(data && *data == '\n' && trg->type == SMT_token && trg->get_next() == 0)
    trg = checked_cast(smtTree,trg->get_parent());

  while(1)      // Calculate target parent and kill group if insertion inside
    {
    if( (partrg = checked_cast(smtTree,trg->get_parent())) == 0 && 
        oper != FIRST )
      return;
    if(partrg == 0 || partrg->type != SMT_group)
      break;
    partrg->report_remove(0);
    partrg->splice();
    obj_delete(partrg);
    }  

  if(src)               // Remove source node (if specified) from tree
    {
    smtHeader * sh = checked_cast(smtHeader,src->get_header());
    if(sh == this)	// Is source from our SMT ?
      {			// Yes
      if(src->get_parent() == 0)
        return;
      src->report_remove(2);
      }
    else		// Source is from different SMT
      {
      int newsize = src_size + src->length ();
      if(src_asize < newsize)
	src_exp_buffer(newsize - src_asize + 64);
      strncpy(&srcbuf[src_size], sh->srcbuf + src->start (), src->length ());
      cor_tbeg(src, src_size - src->start ());
      src_size = newsize;
      src->report_remove(1);
      }
    remove_stmt(sh, src, do_tidy);
    }
  
  if ( built_trg != trg ){
     built_trg = trg;
     smod_rebuild_node(trg, SmodChange, do_tidy);
  }
  switch(oper)      // Make insertion
    {
    case AFTER:
    case FIRST:
      if(data)
         src = db_new(smtTree,());

      if (src->type == SMT_file) {
	  if (oper == AFTER) {
	      smtTree* tmp = f_ins_after (trg);
	      if (tmp)
		  trg = tmp;
	      else {
		  trg = (smtTree*)get_root ();
		  oper = FIRST;
	      }
	  }
      }
	      
      if(oper == AFTER)
        trg->put_after(src);
      else
        trg->put_first(src);

      if(data)
        ni->src_obj = toknode = src->replace_text(data, node_type, ref, format);
      else
	smt_touch(src, 1);

      if ( src->type == SMT_file )
         src->type = SMT_temp;

      break;

    case CHANGE_NAME:
    case REPLACE:
      if(src)
	{
	trg->put_after(src);
	smt_remove(trg);
	}
      else
	{
        trg_was_statement = smt_is_statement(trg);
        toknode = trg->replace_text(data, node_type, ref, format);
//	toknode = trg;
	}
    }

  if ( src ){
     src->ndm = 1;
     src->smod_vision = SmodChange;
  }

  if(do_tidy)
      tidy_tree();
  else if (src)
      src->vrn = 1;
  
  if(((oper == REPLACE && trg_was_statement) ||
     oper == FIRST ||
     (
      (smt_is_statement(trg) ||
       (trg->type == SMT_token && trg->extype != SMTT_ref 
        && srcbuf[trg->tbeg] == '{')) && oper == AFTER)) &&
      src == 0 &&
      strcmp(data, ";") == 0)
    { 
    smt_def_stmt(this, 0, SMT_stmt, toknode->tnfirst, toknode->tnmax);
    smt_tidy_level(toknode);
    }

  else if(toknode)
    {
    if(node_type)
       toknode = checked_cast(smtTree,toknode->get_first_leaf());   
    smtTree * parent = checked_cast(smtTree,toknode->get_parent());

    while(parent && parent->type == SMT_group)
      parent = checked_cast(smtTree,parent->get_parent());
    smt_tidy_level(parent ? parent : toknode);
    }
  if(ni){
     if(0 && format && toknode){  // not implemented yet
        smtTree * parent = checked_cast(smtTree, toknode->get_parent());
        ni->type = REPLACE;
        ni->src_obj = ni->targ_obj = parent;
     } else {
        ni->type = oper;
        ni->targ_obj = trg;

        if (oi->type == EXT_MODIFY)  {
            ni->src_obj = src;
            ni->data = NULL;
        }

        try_modify_ast();  // This updates view text!  No need cont propagation.
     }
  }
  return;
}

void smtHeader::remove_obj(objRemover * ro, objRemover * nr)
{
    Initialize(smtHeader::remove_obj);
    remove_obj_real(ro, nr, 1);
}

void smtHeader::remove_obj_real(objRemover * ro, objRemover * nr, int do_tidy)
{
    Initialize(smtHeader::remove_obj_real);
    if(ro->flags & 0x01)	// Return if it is only LDR removing asked
	return;
    set_modified();
    set_src_modified ();
    smtTree * src = checked_cast(smtTree,ro->src_obj);
    Assert(src);
    if(smt_is_statement(src) && smt_is_single_statement(src)) {
	obj_insert(this, REPLACE, 0, src, (char *)";");
	nr->src_obj = 0;
	return;
    }	
    vrn = drn = 1;
    set_arn(1);
    smtTree* next = checked_cast(smtTree,src->get_last_leaf());
    next = checked_cast(smtTree, next->get_next_leaf());
    if (next && !next->spaces && !char_is_separator (srcbuf[next->tbeg])) {
	next->extype = SMTT_untok;
	untok_flag = 1;
    }
    smtTree *n, *r, *p, *f = 0, *l = 0;
    for(n = src, r = 0; (p = checked_cast(smtTree,n->get_parent())); n = p) {
	r = n;
	if ( n->get_prev() || n->get_next()) break;
    }

    smod_rebuild_node (src, SmodChange, do_tidy);

    nr->src_obj = src;
    smt_cut (src);
    nr->src_parent = p;
    if (p) {
        smt_touch (p);
        p->clear_vrn ();
    }
    if(do_tidy) tidy_tree();
}

static void remove_stmt(smtHeader * h, smtTree * t, int do_tidy)
  {
  if(t->get_parent() == 0)
    {
    rem_relation(header_of_tree, t, h);
    obj_delete(h);
    }
  else
    {
    if(smt_is_statement(t) && smt_is_single_statement(t))
      {	// Insert empty statement if single statement removed
      smtTree *w = db_new(smtTree, ());
      t->put_after(w);
      w->replace_text(";", SMT_stmt);
      if(do_tidy) h->tidy_tree();
      smt_tidy_level(w);
      }
    smt_touch_parent(t, 1);
    t->remove_from_tree();	// Only remove, do not destroy
    }
  }

static void make_block(smtTree * t)
  {  
  smtTree *ns;
  smtTree * nn = db_new(smtTree,(SMT_block));
  t->put_after(nn);
  t->remove_from_tree();
  ns = db_new(smtTree,());
  nn->put_first(ns);
  ns->replace_text("{", SMT_group);
  ns->put_after(t);
  ns = db_new(smtTree,());
  t->put_after(ns);
  ns->replace_text("}", SMT_group);
  }

// Find right point for insertion "FIRST"
// returns node for inserting AFTER 
static smtTree * find_ins_down(smtTree * t)
  {
   Initialize(find_ins_down);

  smtTree * w, * x;

  // If block - insert inside block
  if(t->type == SMT_block)
    {
    w = checked_cast(smtTree,(t->get_first()));
    if(w->get_next() == 0 && (x = checked_cast(smtTree,w->get_first())) != 0)
      return x;
    else
      return w;
    }
 
  // Otherwise - try to find something under
  for(w = checked_cast(smtTree,t->get_first()); w; w = checked_cast(smtTree,w->get_next()))
    {
    if(smt_is_statement(w)) 
      break;
    x = find_ins_down(w);
    if(x)
      return(x);
    }

  if(w == 0 || !smt_is_statement(w))
    return 0;

  if(w->type != SMT_block || !smt_is_single_statement(w))
    {
    make_not_single(w);
    return(checked_cast(smtTree,w->get_prev()));
    }
  return checked_cast(smtTree,(w->get_first()));
  }

// Find right point for insertion "AFTER"
static smtTree * find_ins_after(smtTree * t)
  {
   Initialize(find_ins_after);

  return t;  // mg*

  smtTree * w, * x, * y;
  for(w = t; w; w = checked_cast(smtTree,w->get_parent()))
    {
    // Test for "between statements" condition
    if(w->type == SMT_comment)
      {
      for(x = checked_cast(smtTree,w->get_prev());
          x;
          x = checked_cast(smtTree,x->get_prev()))
        {
        if(smt_is_statement(x))
          break;
        }
      if(x)
        break;
      for(x = checked_cast(smtTree,w->get_next());
          x;
          x = checked_cast(smtTree,x->get_next()))
        if(smt_is_statement(x))
          break;
      if(x && smt_is_single_statement(x))
        return 0;
      if(x)
        break;
      }
    else if(smt_is_statement(w)) break;
    }
  if(w == 0 || w->type != SMT_block || !smt_is_single_statement(w))
    return(w);
  // It is single block - insert before "}"
  for(x = checked_cast(smtTree,w->get_first()); x; x = checked_cast(smtTree,x->get_next()))
    y = x;
  return checked_cast(smtTree,y->get_prev());
  }
	
// Make shure that statement is not single
static void make_not_single(smtTree * t)
  {
  if(smt_is_statement(t) && smt_is_single_statement(t))
    make_block(t);
  }

static void cor_tbeg(smtTree * t, int d)
  {
  Initialize(cor_tbeg);

  t->tbeg += d;
  for(smtTree * w = checked_cast(smtTree,t->get_first());
      w;
      w = checked_cast(smtTree,w->get_next()))
    {
    cor_tbeg(w, d);
    }
  }
