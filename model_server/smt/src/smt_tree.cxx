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
// File smt_tree.C - SMODE 
//
// History:   11/25/91   M.Furman       Start of initial coding

#include "smt.h"
#include "ddict.h"
#include "cLibraryFunctions.h"
#include <msg.h>
#include <messages.h>
#include <objArr.h>
//#include <ldr.h>
#include <driver_mode.h>

extern int smt_debug_flag;

ostream & smt_get_map_stream();
static smtTree* smt_set_bound(smtTree *, int, int, int, smtHeader *hd);
static smtTree* smt_set_bound_down(int,smtTree, int, int, int, smtHeader *hd);
static void deep_destroy_stmt(smtTree *);
static void tncorr(smtTree *);
static void setpar(smtTree *);
extern int smt_test_consistency(int, int, int);
extern int smt_stmt_priority(int, int);
void smt_unset (smtTree*);
boolean smt_type_stmt(int);
#ifdef XXX_has_ldr_root_relation
static bool has_ldr_root_relation (appTree* t)
{
    Initialize (has_ldr_root_relation);
 
    int retval = 0;
    Obj* ma = t->get_raw_ptr(ldrtree_of_apptree);
    Obj* curr_memb;
    ForEach (curr_memb, *ma) {
	if (checked_cast (objTree, curr_memb)->get_parent () == 0) {
	    retval = 1;
	    break;
	}
    }
    return retval;
}
#endif
void smt_report_minor_problem ()
{
  Initialize(smt_report_minor_problem);
  // mkamin 1998-11-06 bug #14702 Suppress this alarming message:
  // Warning: *** Minor Model problems have occurred.
  //          Please contact SET Customer Support.
}

// define token on SMT tree
int smt_def_token(smtHeader * h, int begin, int length, int skip, int ttype,
		  int newlines, int spaces)
  {
  Initialize(smt_def_token);
  smtTree * b, * t;
  int total_length = length + newlines + spaces;
 
  b = h->cn_na(begin);			// Find begin node

  if(b == 0 && skip)
    return(0);

  Assert(b != 0);
  Assert(b->tbeg == begin);
  
  if(skip)
    {
    if(b)
      {
      if(b->tlth <= length)
        {
        smt_cut(b);		// Remove white space token
        h->last_token = 0;
        }
      else
        {
        b->tbeg += length;	// Cut spaces at the beginning
        b->tlth -= length;
////	b->report_split(total_length, 0, b);
	smt_touch(b, 1);
	h->vrn = 1;
        }
      }
    return(0);
    }

  Assert(length <= b->tlth);
  b->tnmax = b->tnfirst;

  if(length < b->tlth)
    {
    // end cut needed
    if (!((t = b->get_next_leaf ()) && (t->tbeg + t->tlth + t->newlines <= b->tbeg + b->tlth)))
	t = db_new(smtTree,(SMT_token));	// Create new node
    t->tbeg = begin + length;		// Begin = number of next byte
    t->tlth = b->tlth - length;	// Length = residual length
    b->tlth = length;			// Set length of defined token
    t->tnfirst = t->tnmax = b->tnfirst + 1;
    if (!t->get_parent ())
	b->put_after(t);			// put into tree
    h->last_token = t;
    // update marker
    b->report_split(total_length, b, t);
    smt_touch(t, 1);
  }
  tncorr(b);

  for (t = b->get_next_leaf ();
       t && t->type != SMT_token;
       t = t->get_next_leaf ());

  if(t) 
    {
    t->tnfirst = t->tnmax = b->tnmax + 1;
    tncorr(t);
    }

  b->extype = ttype;
  smt_touch(b, 1);
  h->vrn = 1;

  h->set_ttable(b);
  b->spaces = spaces;
  b->newlines = newlines;
  if (ttype == SMTT_el)
      b->tlth = 0;
  return(b->tnfirst);
  }

// Define statement on SMT tree
static smtTree * smt_def_stmt_internal (smtHeader * h, int mode, int type, int s1, int s2)
  {
  Initialize(smt_def_stmt_internal);
  smtTree *t, *t1, *t2, *t1a, *t2a, *w, *x, *tn;

  if(!h->parsed)	// Ignore mapping for files which are not parsed
    return (0);

  if(s1 < 1 || s2 < 1 || s1 > s2)
    {
    // dis_message (NULL, MSG_WARN, "M_SMT_WRONGTOKENS", s1, s2);
    // SMT tree: Trying to define statement with wrong token numbers\nFrom %d to %d - IGNORED

        smt_report_minor_problem ();

	if (smt_debug_flag) {
	    ostream & os = smt_get_map_stream();
	    os << "SMT tree: Trying to define statement with wrong token numbers\nFrom " << s1 << " to " << s2 << " - IGNORED" << endl;
	}
    return(0);
    }

  t1 = h->tn_na(s1);			// Find first token 
  t2 = h->tn_na(s2);			// Find last token
  if(t1 == 0 || t2 == 0)
    {
    // dis_message (NULL, MSG_WARN, "M_SMT_NONEXISTENTTOKENS", s1, s2);
    // SMT tree: Trying to define statement with non existent tokens\nFrom %d to %d\n - IGNORED
        smt_report_minor_problem ();
	if (smt_debug_flag) {
	    ostream & os = smt_get_map_stream();
	    os << "SMT tree: Trying to define statement with non existing tokens\nFrom " << s1 << " to " << s2 << " - IGNORED" << endl;
	}
	return(0);
    }
  tn = 0;
  smtTree* p1 = t1->get_parent ();
  smtTree* p2 = t2->get_parent ();
  
  // smtTree* cr = (smtTree *)obj_tree_common_root(t1, t2,
  // (objTreePtr *)&t1a, (objTreePtr *)&t2a);
  // if (((cr->get_first_leaf() != t1) || (cr->get_last_leaf() != t2)))
  if ( t1 != t2 && (p1->get_parent () || p2->get_parent ()))
  {
    smtTree* cr = (smtTree *)obj_tree_common_root(t1, t2,
                  (objTreePtr *)&t1a, (objTreePtr *)&t2a); 
    if ((tn = smt_set_bound(cr, s1, s2, type, h)) && (tn->get_node_type() == type) )
	return tn;
  }
  
  // Get common root of first and last tokens
  tn = (smtTree*)
       obj_tree_common_root(t1, t2,(objTreePtr *)&t1a, (objTreePtr *)&t2a);

  // If it is a token go up to group
//  tn = cr;
  if(tn->type == SMT_token)
    {
    t = tn->get_parent();
    t1a = t2a = tn;
    tn = t;
    }

  if(tn->type == SMT_group)		// Go up to old statement
    {					// if it is group
    t = tn->get_parent();
    if(tn->tnfirst != s1 || tn->tnmax != s2)
      smt_splice(tn);
    else
      t1a = t2a = tn;
    tn = t;
    }

  // t1a and t2a are children of tn (might be the same child) which
  // contain tokens s1 and s2

  while(tn->tnfirst == s1 && tn->tnmax == s2  && tn->type != type &&
       tn->type != SMT_temp && smt_stmt_priority(tn->type, type))
    {	// If new statement has exectly same begin the new one
    t = tn->get_parent();	// has higher priority
    if(t)
      {
      t1a = t2a = tn;
      tn = t;
      }
    else
      break;
    }	

  if(tn->type == SMT_file ||
    (mode >= 2 && tn->type != type) ||
    tn->tnfirst != s1 || tn->tnmax != s2)
    {	// If something needed to change in structure

    // Kill statement if newone begins at the same point
    if(tn->tnfirst == s1 && 
       tn->get_parent() != 0 && 
       mode < 2 && 
       tn->type != SMT_file)
      {
      smt_splice(tn);
      tn = 0;
      }

    t = db_new(smtTree,(type));		// Create node for statement

    t->add_parent_to_list(t1a,t2a);

    setpar(t);
    smt_touch(t);
    h->vrn = 1;	
    }
  else
    {
    if(type != tn->type)
      {
      tn->type = tn->extype = tn->was_type = type;	// Change type of statement
      smt_touch(tn);
      tn->ndm = 1;
      h->vrn = 1;
      }
    t = tn;
    }

  // Destroy inner statements if mode= 0;
  if(mode == 0)
    {
    for(w = t->get_first(); w; w = x)
      {
      x = w->get_next();
      deep_destroy_stmt(w);
      }
    }

  return(t);
  }

static void clear_rel_ref_dd_of_smt (smtTree* t)
{
    smtTree* child = t->get_first ();

    if (!child) {
	if (get_relation (ref_dd_of_smt, t))
	    rem_relation (ref_dd_of_smt, t);
    } else {
	for (; child; child = child->get_next ())
	    clear_rel_ref_dd_of_smt (child);
    }
}

inline int is_stmt_or_clause(int type)
{
  return smt_type_stmt(type) 
         || (type==SMT_then_clause) 
         || (type==SMT_else_clause) 
  ;
}

static int is_macro(smtTreePtr t)
{
 return
   t->get_first_leaf()->is_macro ||
   t->get_last_leaf()->is_macro ;
}

static int smt_splice_is_dangerous(smtTree*t)
{

  if(!  is_stmt_or_clause(t->type))
       return 1;

  return ! is_macro(t);

}

static int smt_splice_count = 0;

smtTree * smt_def_stmt (smtHeader * h, int mode, int type, int s1, int s2)
{
   Initialize(smt_def_stmt);

    smt_splice_count = 0;

    smtTree *t, *par, *old_par, *t1;

    t = smt_def_stmt_internal (h, mode, type, s1, s2);
    if (! t ) return 0;

    if (type == SMT_comment) {
        clear_rel_ref_dd_of_smt (t);
        smtTree *root = (smtTree*)h->get_root ();

	// move comment from first or last position
	if ((old_par = par = t->get_parent ()) != root ) {
	    if (!t->get_prev ()) {
		for (; par != root && !(t1 = par->get_prev ());
		     par = par->get_parent ());
		if (t1)
		    t1->put_after (t);
		else
		    par->put_first (t);
		// make parent temp
	        if (!old_par->get_first () && old_par->type == SMT_group)
		    smt_cut (old_par);	
		if (t1 = t->get_next ()) t1->type = SMT_temp;
	    }
	}
	if ((old_par = par) != root ) {
	    if (!t->get_next ()) {
		for (; par != root && !(t1 = par->get_next ());
		     par = par->get_parent ());
		if (t1)
		    t1->put_before (t);
		else
		    par->get_last ()->put_after (t);
		// make parent temp
	        if (!old_par->get_first () && old_par->type == SMT_group)
		    smt_cut (old_par);
		else old_par->type = SMT_temp;
	    }
	}
    }

    if (smt_splice_count && smt_splice_is_dangerous(t)) {
      smt_report_minor_problem ();
      if(smt_debug_flag) {
	  ostream & os = smt_get_map_stream();
	  os << "smt_def_stmt: splicing while making:" << endl;
	  smt_prt_mark(h, s1, s2, type);
	  smt_splice_count = 0;
      }
    }

    return t;
}

void smt_tidy_level(smtTree * /*r*/)
  {
  return;
  }

static void setpar(smtTree *t)
  {
  Initialize(setpar);
  smtTree *a, *b, *c;
  t->tbeg = -1;
  t->tlth = t->tnfirst = t->tnmax = 0;
  a = t->get_first();
  if(a == 0)
    return;
  for(b = a; (c = b->get_next()) != 0; b = c);
  t->tbeg = a->tbeg;
  t->tlth = b->tbeg - a->tbeg + b->tlth;
  while(a && a->tnfirst == 0) a = a->get_next();
  while(b && b->tnfirst == 0) b = b->get_prev();
  if(a == 0)
    return;
  t->tnfirst = a->tnfirst;
  t->tnmax = b->tnmax;
  return;
  }

// destroy statemens wich non consistent with bounds of new statement
// boris: "added smtHeader *hd" parameter to set header vrn flag 
// without get_header() call if a node is of type SMT_temp
static smtTree* smt_set_bound_up(int level, smtTree *root, int b1, int b2, 
				 int ntype, smtHeader *hd )
  {
  Initialize(smt_set_bound_up);

  smtTree *t, *n;
  int x;

  n = root->get_parent();
  if (n == 0)
    return NULL;

  t = root;
  // Refer to the original definition of x in smt_set_bound_down
  if(t->tnfirst == b1 && t->tnmax == b2)
    x = 0;
  else
    x = 3;
  
  if (x == 0) {
    if (t->type == SMT_temp) {
      t->type = t->was_type = t->extype= ntype;
      t->vrn = hd->vrn = 1;
    }
    return t;
  } else if (level == 0 && t->type == SMT_fdef && ntype == SMT_fdef) {
    smt_splice(t);
  } else if (t->type == SMT_temp) {
    smt_splice(t);
  } else if(smt_test_consistency(t->type, ntype, x) 
            /*XXX: && !has_ldr_root_relation (t)*/ ) {
    if(! is_macro(t) /* || is_stmt_or_clause(t->type) */ )
      smt_splice(t);
  }
  else if ((t->tnfirst <= b1) && (t->tnmax >= b2)) return 0;
  return smt_set_bound_up(level+1, n, b1, b2, ntype, hd);
}

// destroy statemens wich non consistent with bounds of new statement
// boris: "added smtHeader *hd" parameter to set header vrn flag 
// without get_header() call if a node is of type SMT_temp
static smtTree* smt_set_bound_down(int level, smtTree *root, int b1, int b2, int ntype, smtHeader *hd)
{
  Initialize(smt_set_bound_down);

  smtTree *t, *n;
  int x;
  for(t = (smtTree *)root->get_first(); t && t->tnfirst <= b2 + 1; t = n) {
    n = (smtTree *)t->get_next();
    if(t->type == SMT_token) {
      /* delete line break at the begin / end of statement */
      if(t->extype == SMTT_lb && 
	 (t->tnfirst == b1 - 1 || t->tnfirst == b2 + 1) &&
	 smt_type_stmt(ntype)) {
	t->report_remove(0);
	smt_touch_parent(t, 0);
	hd->vrn = 1;
	smt_unset(t);
	t->remove_from_tree();
	obj_delete(t);
      }
      continue;
    }
    
    if(t->tnfirst > b2 || t->tnmax < b1)
      continue;
    
    if(t->tnmax >= b1) {
      smtTree* ot = smt_set_bound_down(level+1, t, b1, b2, ntype, hd);
      if (ot)
	return ot;
    }

    // Calculate case number:
    //  1, 2 , 3 - old one wider then new
    //  0 - equal exactly
    //  1, -1 - the same left tokens
    //  2, -2 - the same right tokens
    
    if(t->tnfirst < b1) {
      if(t->tnmax < b2)
	x = 4;
      else if(t->tnmax == b2)
	x = 2;
      else
	x = 3;
    } else if(t->tnfirst == b1) {
       //      if(b1 == b2)		// Special code for macros: 
       //        continue;		// allow b1 = b2 = tnfirst
       if(t->tnmax < b2) 
	 x = -1;
       else if(t->tnmax == b2)
	 x = 0;       /* It will be this case only when b1 == b2.*/
       else
	 x = 1;
     } else {
       if(t->tnmax < b2)
	 x = -3;
       else if(t->tnmax == b2)
	 x = -2;
       else
	 x = 4;
     }
    if (x == 4)
      smt_splice(t);
    else if (x == 0) { 
      if (t->type == SMT_temp) {
	t->type = t->was_type = ntype;
	t->vrn = hd->vrn = 1;
      }
      return t; 
    } else if (level == 0 && t->type == SMT_fdef && ntype == SMT_fdef) {
      smt_splice(t);
    } else if (smt_test_consistency(t->type, ntype, x)){
      smt_splice(t);
    }
  }
  return 0;
}

// destroy statemens wich non consistent with bounds of new statement
// boris: "added smtHeader *hd" parameter to set header vrn flag 
// without get_header() call if a node is of type SMT_temp
static smtTree* smt_set_bound(smtTree *root, int b1, int b2, int ntype, smtHeader *hd)
{
  Initialize(smt_set_bound);
    smtTree *tn = smt_set_bound_down(0, root, b1, b2, ntype, hd);
    if (tn)
        return(tn);
    tn = smt_set_bound_up(0, root, b1, b2, ntype, hd);
    return(tn);
}

// deep destroy statement
static void deep_destroy_stmt(smtTree *t)
  {
  Initialize(deep_destroy_stmt);
  smtTree *w, *n;
  if(t->type == SMT_token)
    return;
  for(w = t->get_first();
      w;
      w = n)
    {
    n = w->get_next();
    deep_destroy_stmt(w);
    }
  smt_splice(t);
  return;
  }

// Correct t->tnfirst and t->tnmax fields of upper nodes
static void tncorr(smtTree *t)
  {
  Initialize(tncorr);

  smtTree * w;
  t->tnmax = t->tnfirst;
  if(t->tnmax == 0)
    return;
  for(w = t->get_parent(); w; w = w->get_parent())
    {
    if(t->get_prev() == 0) w->tnfirst = t->tnfirst;
    if(t->tnmax > w->tnmax) w->tnmax = t->tnmax;
    t = w;
    }
  }

smtTree * smt_cut(smtTree *t)   // returns actually cut parent
  {
  Initialize(smt_cut);
  smtTree * n, * p, *r;
  int first_child;

  for(n = t, r = 0; (p = n->get_parent()); n = p)
    {
    r = n;
    if(n->get_prev() || n->get_next()) break;
    }
  if(r)
    {
    first_child = ! r->get_prev();
    smtHeader::set_touched_ddElements(r);    
    smt_remove(r);
    }
    
  if(first_child)  
    {
    if(p && p->type != SMT_file)
         p->type = p-> extype = SMT_temp;
    }
  return r;
  }


// Going to be moved to separate file : AST related staff

static char stmt_table_stat[] =  { SMT_STAT , 0 };
static char stmt_table_decl[] =  { SMT_DECL , 0 };
static char stmt_table_defi[] =  { SMT_DEFI , 0 };

boolean smt_type_cstmt(int x)
  {
  int i;
  for(i = 0; stmt_table_stat[i] != 0 && x != stmt_table_stat[i]; i++);
  return stmt_table_stat[i] != 0;
  }

boolean smt_type_cdecl(int x)
  {
  int i;
  for(i = 0; stmt_table_decl[i] != 0 && x != stmt_table_decl[i]; i++);
  return stmt_table_decl[i] != 0;
  }

boolean smt_type_cdefi(int x)
  {
  int i;
  for(i = 0; stmt_table_defi[i] != 0 && x != stmt_table_defi[i]; i++);
  return stmt_table_defi[i] != 0;
  }

boolean smt_type_stmt(int x)
  {
  return smt_type_cstmt(x) || smt_type_cdecl(x) || smt_type_cdefi(x);
  }


boolean smt_is_statement(smtTree *t)
  {
  return smt_type_stmt(t->type);
  }


static int single_tab[] =
  {SMT_if, SMT_ifelse, SMT_else, SMT_for, SMT_while, SMT_do, SMT_clause,
   SMT_nstdloop, SMT_dowhile, SMT_then_clause, SMT_else_clause,
   SMT_else_if_clause, 0};

boolean smt_is_single_statement(smtTree *t)
  {
  Initialize(smt_is_single_statement);

  smtTree * w = t->get_parent();
  int tp = w->type;
  for(int i = 0; single_tab[i] != 0; i++)
    if(w->type == single_tab[i])
      return 1;
  return 0;  
  }

extern "C" void smt_prt_tree(smtTree*);

void smt_kill_comment(smtTree * t)
  {
  Initialize(smt_kill_comment);

  if(t == 0 || t->type != SMT_token)
    return;
  smtTree * w = t->get_parent();
  if(w == 0)
    return;
  if(w->type == SMT_group)
    w = w->get_parent();
  if(w && w->type == SMT_comment)
    {
    w->type = w->extype = SMT_temp; 	// Switch to temporary structure which
    w->arn = 1;				//  will gone on next reparse
    }
  }


void smt_splice(smtTree *t)
{
  Initialize(smt_splice);

  rem_relation(def_dd_of_smt, t, 0);
  t->report_remove(0);
  if(t->type != SMT_temp) {

    if (smt_splice_is_dangerous(t)) {
      smt_report_minor_problem ();
      if(smt_debug_flag) {
	ostream & os = smt_get_map_stream();
	os << "smt_splice: splicing smtTree:" << endl;
	smt_prt_tree(t);
	smt_splice_count ++;
      }
    }

    smt_touch_parent(t);
    }
  smt_unset(t);
  if (t->get_first()){
      t->splice();
      obj_delete(t);
  }
  return;
}

void smt_remove(smtTree *t)
  {
  Initialize(smt_remove);

  smtTree * nxt;
  for(smtTree * w = t->get_first();
      w;
      w = nxt)
    {
    nxt = w->get_next();
    smt_remove(w);
    }
  t->report_remove(1);
  smt_touch_parent(t, 1);
  smt_unset (t);
  t->remove_from_tree();
  obj_delete(t);
  }

// boris: converted from inline. Removed up-tree vrn setting
//
void smt_touch(smtTree *t, int ast)
{
  t->vrn = 1;

  if(ast)
    t->arn = 1;
}

//
// returns: -1: error (invalid data), >=0: the number of nodes that got cleared
//
int
smtTree_clear_is_macro(smtTree * tnode)
{
    int ret_val = -1;

    if (tnode) {
	for (ret_val=0; tnode!=0; tnode=tnode->get_next()) {
	    // -- clear the flag on the node's children
	    smtTree * tsubr = tnode->get_first();
	    if (tsubr) {
		int ret_rcr = smtTree_clear_is_macro(tsubr);
		if (ret_rcr > 0)
		    ret_val += ret_rcr;
	    }
	    // -- clear the flag on the node itself
	    if (tnode->is_macro) {
		tnode->is_macro = 0;
		ret_val++;
	    }
	}
    }

    return ret_val;
}

/*
   START-LOG-------------------------------------------

   $Log: smt_tree.cxx  $
   Revision 1.23 2001/07/25 20:43:08EDT Sudha Kallem (sudha) 
   Changes to support model_server.
 * Revision 1.1  1996/12/05  15:41:03  joshj
 * Initial revision
 *
Revision 1.2.1.30  1994/07/26  22:36:56  boris
Bug track: 7617
Fixed crash in uotline view while editting SMODE file

Revision 1.2.1.29  1994/03/21  20:00:16  trung
Bug track: 6625
submitting for cpp_open_file, etc.

Revision 1.2.1.28  1994/01/19  20:04:27  aharlap
undo prev change

Revision 1.2.1.26  1993/07/27  22:40:44  mg
static int is_macro(smtTreePtr t)

Revision 1.2.1.25  1993/07/20  15:14:19  aharlap
don't splice leaf

Revision 1.2.1.24  1993/06/24  14:55:44  mg
smt_splice_is_dangerous.

Revision 1.2.1.23  1993/05/09  20:50:57  aharlap
don't move existent token around the tree

Revision 1.2.1.22  1993/04/22  13:34:43  aharlap
added field was_type to smtTree

Revision 1.2.1.21  1993/04/04  23:27:49  boris
used ndm smtTree node flag for "Repasrse" category

Revision 1.2.1.20  1993/03/31  20:20:29  aharlap
fixed bug # 2792

Revision 1.2.1.19  1993/03/03  03:16:40  trung
check when mapping

Revision 1.2.1.18  1993/03/01  00:39:03  aharlap
fixed bugs

Revision 1.2.1.17  1993/02/27  23:03:01  boris
Partial fixes for bug #2716;



   END-LOG---------------------------------------------

*/
