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
// File smt_replace_text.C

#include <msg.h>
#include "smt.h"
#include "SmtSte.h"

void smt_real_split( smtTree *trg, int off, smtTree *src);  

smtTree * smt_find_insertion(objOperType& oper, smtTree*trg, int src_type)
{
   Initialize(smt_find_insertion);

    if(src_type==SMT_token || src_type==SMT_group)
       return trg; 
 
    for(smtTree*parent;;trg=parent){

      if(smt_is_statement(trg)) break;
    
      if(oper==BEFORE){    
        if(trg->get_prev()) break;
      } else if(oper==AFTER) {
        if(trg->get_next()) break;
      }

      parent = checked_cast(smtTree,trg->get_parent());
      if(!parent) break;
      if(parent->type==SMT_file) break;

    }
    return trg;

}

smtTree * smt_replace_text(smtTree *toknode, char const *data, int format, int do_tidy)
{
  Initialize(smt_replace_text);

  smtHeader * smt_header = checked_cast(smtHeader, toknode->get_header());
  smt_header->set_modified();
  smt_header->set_src_modified();

  smtTree * partrg;

  int node_type = 0;
  int ref = 0;

  smt_header->drn = 1;
  smt_header->set_arn(1);
  if ( !format )
      smt_header->vrn = 1;

  if (!format){
      while(1) {  // Calculate target parent and kill group if insertion inside
	  if((partrg = checked_cast(smtTree,toknode->get_parent())) == 0)
	      return(toknode);
	  if(partrg == 0 || partrg->type != SMT_group)
	      break;
	  partrg->report_remove(0);
	  partrg->splice();
	  obj_delete(partrg);
      }  
  }
  toknode = toknode->replace_text(data, node_type, ref, format);

  if (do_tidy) smt_header->tidy_tree();
  if (!format) {
      smt_tokenizer(smt_header, toknode);

      if(node_type)
	  toknode = checked_cast(smtTree,toknode->get_first_leaf());   
      
      smtTree * parent = checked_cast(smtTree,toknode->get_parent());

      while(parent && parent->type == SMT_group)
	  parent = checked_cast(smtTree,parent->get_parent());
      smt_tidy_level(parent ? parent : toknode);
      try_modify_ast();
  }
  return toknode;
}

void smt_set_header_modified( smtHeaderPtr h );
void smt_replace_middle (smtTree *st, int off1, int off2, const char *txt, int do_tidy)
{
    Initialize(smt_replace_middle);

    // delete
    if (!txt || !txt[0]) {
	smt_cut_region (off1, st, off2, st, do_tidy);
	return;
    }

    int tok_len      = st->length();

    // insert
    if (off1 == off2 && off1 >= 0 && off1 <= tok_len) {
	smtTree *nxt =  db_new(smtTree,());

	if (off1 == tok_len) {
	    st->put_after(nxt);
	} else if (off1 == 0) {
	    smtTree *p = st->get_prev_leaf ();
	    if (p)
		p->put_after(nxt);
	    else {
		p = checked_cast(smtTree,st->get_root());
		Assert(p);
		p->put_first(nxt);
	    }
	} else {
	    smt_real_split (st, off1, st);
	    st->put_after(nxt);
	}
	smt_replace_text(nxt, (char *)txt, 1, do_tidy);
	return;
    }

    // replace
    smtTree *rep = NULL; smtTree *old_nxt = st->get_next();

    if (off2 > 0 && off2 < tok_len) {
	smt_real_split (st, off2, st);
	old_nxt = st->get_next();
	off2 = -1;
    }

    if (off1 == 0 && (off2 == -1 || off2 >= tok_len))
	rep = st;                        // replace whole token
    else if (off1 > 0 && off1 < tok_len) {
	smt_real_split (st, off1, st);
	smtTree *nxt = st->get_next();
	if (nxt != old_nxt)
	    rep = nxt;
	else {
	    rep = db_new(smtTree,());
	    st->put_after(rep);
	}
    }

    if (rep) smt_replace_text(rep, (char *)txt, 1, do_tidy);
    return;
}


void smt_replace_region (int off1, smtTree *st, int off2, smtTree *en, const char *txt, int do_tidy)
{
    Initialize(smt_replace_region);
    if (!st || !en) return;

    if (txt == 0 || txt[0] == 0) { 
	smt_cut_region (off1, st, off2, en, do_tidy);
	return;
    }
    
    smtHeader *h = checked_cast(smtHeader,st->get_header());
    if ( h == 0 ) return;

    smt_set_header_modified( h );
    st = st->get_first_leaf();
    en = en->get_last_leaf();

    DBG {
	msg("$1: $2 $3; $4 $5\n") << _my_name << eoarg << off1 << eoarg << st << eoarg << off2 << eoarg << en << eom;
	st->print(cout, 0);
	en->print(cout, 0);
	msg("") << eom;
    }

    if (st != en ) {
	smtTree *nxt = st->get_next_leaf();
	IF (!nxt) return;
	
	smt_cut_region (0, nxt, off2, en, do_tidy);
	en   = st;
	off2 = -1;
    }

    smt_replace_middle (st, off1, off2, txt, do_tidy);
    return;
}


/*
   START-LOG-------------------------------------------

   $Log: smt_replace_text.cxx  $
   Revision 1.9 2000/07/12 18:13:49EDT ktrans 
   merge from stream_message branch
// Revision 1.11  1993/08/30  18:51:57  trung
// revert to prev version
//
// Revision 1.9  1993/08/05  20:00:31  boris
// Fixed saving in SMOD
//
// Revision 1.8  1993/02/21  20:06:06  boris
// Remove view regenerate from replace_text
//
// Revision 1.7  1992/11/22  01:14:28  builder
// typesafe casts.
//
// Revision 1.6  1992/11/01  22:25:18  aharlap
// changed smt_replace_text ()
//
// Revision 1.5  1992/10/30  23:08:23  boris
// Added smt_real_split()
//
// Revision 1.4  92/10/29  20:35:42  boris
// Introduced smt region handler
// 
// Revision 1.3  92/10/22  10:44:44  mg
// paste
// 
// Revision 1.2  92/10/15  14:31:10  mg
// Fixed history
// 


   END-LOG---------------------------------------------

*/
