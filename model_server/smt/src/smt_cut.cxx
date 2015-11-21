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
// smt_cut.C - contains offset cut region ( non merge ) implementation
//
//

#include <msg.h>
#include "smt.h"
#include "appMarker.h"
#include "psetmem.h"
#ifndef _objArr_h
#include <objArr.h>
#endif

enum {
 CUT_HEAD,
 CUT_TAIL,
 CUT_MIDDLE,
 CUT_SPLIT
};

int smt_tidy_markers( smtTree *node, int off1, int off2, int op ){
  Obj *ob = get_relation(marker_of_node,node);
  if ( ob == 0 )
     return 0;

  int answer = 1;
  Obj *cur;
  appMarker *m;
  ForEach(cur, *ob){
     m = (appMarker *)cur;
     switch ( op ){
        case CUT_MIDDLE:
           if (m->offset > off1 && (off2 == -1 || m->offset <= off2))
                m->offset = off1;
           else if (m->offset > off2  && off2 != -1)
                m->offset = m->offset - off2 + off1;
           break;
        case CUT_HEAD:
           if ( m->offset < off1 )
              m->offset = 0;
           else
              m->offset = m->offset - off1;    
           break;
        case CUT_TAIL:
           if ( m->offset >= off1 )
              m->offset = -1;
           break;
        case CUT_SPLIT:
           if ( m->offset != 0 && off1 > 0 && off1 < node->tlth ){
              appTreePtr next_nd = node->get_next();
              if ( next_nd ){
                 int new_off = m->offset - off1;
                 if ( m->offset < 0 || new_off >= 0 ){
                    rem_relation(marker_of_node,node,m);
                    put_relation(marker_of_node,next_nd,m);
                    if ( m->offset > 0 )
                       m->offset = new_off;
                 }
              }
           }
           break;
        default:
           answer = 0;
           break;
     }
  }
  return answer;
}

void smt_to_untok( smtTree *node )
{
  Initialize(smt_to_untok);

  smtTreePtr frst = node->get_first();
  if ( frst )
     for ( smtTreePtr cur = frst ;
           cur ;
           cur = cur->get_next() )
        smt_to_untok( cur );
  else{   
    node->extype = SMTT_untok; 
    checked_cast (smtHeader,node->get_header ())->untok_flag = 1;
    if ( node->spaces == 0 && node->newlines == 0 )
       return;

    node->tbeg -= node->spaces;
    node->tlth = node->length ();

    node->spaces   = 0;
    node->newlines = 0;     
  }
}

int smt_cut_tail(smtTree *node, int off)
{
  int len = node->length();
  off = ( off >= len ) ? -1 : off;
  if ( off == -1 )
       return len;

  smt_to_untok( node );
  smt_tidy_markers(node, off, off, CUT_TAIL);
  node->tlth = off;
  return off;
}

int smt_cut_head(smtTree *node, int off)
{
  int len = node->length ();
 
  if ( off > len ) 
     return 0;

  if( off == -1 || off == len ){
    node->tlth = 0;
    node->extype = 0;
    node->spaces = 0;
    node->newlines = 0;
    return node->length ();
  }
  
  smt_tidy_markers(node, off, off, CUT_HEAD);
  smt_to_untok( node );

  if( off ){
     node->tbeg += off;
     node->tlth -= off;
     node->extype = node->tlth ? SMTT_untok : SMTT_el;
     if (node->tlth)
	 ((smtHeader*)node->get_header ())->untok_flag = 1;
	 
  }
  return node->length();
}

void smt_cut_middle(smtTree *node, int off1, int off2)
{
   Initialize(smt_cut_middle);

  int len = node->length();
  if ( off1 >= len || off2 > len || ( (off2 != -1) && (off1 >= off2) ) )
     return;

  if ( off1 == 0 && (off2 == -1 || off2 >= len ) ){
     smt_cut(node);
     return;
  }

   smt_tidy_markers(node, off1, off2, CUT_MIDDLE);
   smt_to_untok( node );

   if ( off1 == 0 ){
      node->tbeg += off2;
      node->tlth = len - off2;
   }else if ( off2 == -1 ){
      node->tlth = off1;
   }else{
     int to   = node->tbeg + off1;
     int from = node->tbeg + off2;
     smtHeaderPtr hd = checked_cast(smtHeader,node->get_header());
     char *buf = hd->srcbuf; 
     strncpy(&buf[to], &buf[from], (len - off2));
     node->tlth = len - (off2 - off1);
   }
   smt_touch(node,1);
   node->ndm = 1;
}

void smt_set_header_modified( smtHeaderPtr h ){
   h->set_modified();
   h->set_src_modified ();
//   h->vrn = 1;
   h->drn = 1;
   h->set_arn( 1 );
}

void smt_cut_region(int off1, smtTree * node1, int off2, smtTree * node2, int do_tidy)
{
  Initialize(smt_cut_region);

  smtHeader *h = checked_cast(smtHeader,node1->get_header());
  if ( h == 0 )
       return;

  smt_set_header_modified( h );

  node1 = node1->get_first_leaf();
  node2 = node2->get_last_leaf();

  DBG{
    msg(" $1 $2") << off1 << eoarg << node1 << eom;
    msg(" $1 $2") << off2 << eoarg << node2 << eom;
    msg("") << eom;
    node1->print(cout, 0);
    msg("") << eom;
    node2->print(cout, 0);
    msg("") << eom;
  }

  if ( off1 == 0 && off2 == -1 ){   // let us switch to untok next/prev leaf
     smtTreePtr wrk = NULL;
     if ( !(wrk = node2->get_next_leaf()) )
        wrk = node1->get_prev_leaf();

     if ( wrk ){
        smt_to_untok( wrk );
//        smt_touch( wrk, 1 );     do not touch it if it is not broken
    }
  }

  if(node1==node2){
    smt_cut_middle(node1, off1, off2);
    if (do_tidy)
	h->tidy_tree();

    return;
  }

  int len1 = smt_cut_tail(node1, off1);
  int len2 = smt_cut_head(node2, off2);

  smtTreePtr first, last;

  if(len1==0){
    first = node1;
  } else {
    smt_touch(node1,1);
    node1->ndm = 1;
    first = node1->get_next_leaf();
  }

  if(len2==0){
     last = node2;
  }else {
     smt_touch(node2,1);
     node2->ndm = 1;
     last = node2->get_prev_leaf();
  }

  if ( first == node2 && last == node1 )
     first = 0;

  smtTree *cur, *next;
  for( cur = first ; cur && last ; cur = next ){
     next = cur->get_next_leaf();
     smt_cut(cur);
     if(cur == last)
       break;
  }

  if (do_tidy) {
      h->tidy_tree();
  }
}


// Unites all objects under parent of the first one
// If do_regions is 1, creates SMT_temp subtree for each region that
// contains more than one node
smtTree * smt_unite(Obj *ob, objArr_Int &region, int do_regions)
{
    Initialize(smt_unite);
    smtTree* st=0, *en=0, *root = NULL, *el = NULL, *tmp = NULL;
    Obj *obj_el;
    int len = 0, node_ind = 0;
    int cur_len;

    int no_regs = (&region) ? (region.size() / 4) : 0;
    int reg_st = -1, reg_en = -1, reg_ind = -1;
    if (&region && do_regions && no_regs) {
	reg_ind++;
	reg_st = region[reg_ind];
	reg_en = region[reg_ind + 2];
    }

    ForEach(obj_el, *ob){
	el = checked_cast(smtTree,obj_el);
	if (st) 
	{
	    // Only unite upper most nodes of subtrees cut from other
	    // smt`s.
	    if(  el->get_parent() )
		continue;
	    en->put_after(el);
	}
	else
	    st = el;
	
	en =  el;

	cur_len = en->start () + en->length ();
	if ( cur_len > len )
	    len = cur_len;

	if (do_regions && reg_st > -1 && reg_en > -1 && reg_ind > -1) {
	    if (reg_en > reg_st && reg_st == node_ind) {
		tmp = new smtTree;
		tmp->type = tmp->extype = SMT_temp;
		en->put_after (tmp);
		tmp->put_first (en);
	    }

	    if ( reg_en == node_ind ) {
		if ( reg_en > reg_st && tmp ) {
		    en = tmp;
		    tmp = NULL;
		}

		reg_ind ++;
		if (reg_ind < no_regs) {
		    reg_st = region[reg_ind*4];
		    reg_en = region[reg_ind*4 + 2];
		} else
		    reg_ind = reg_st = reg_en = -1;
	    }
	}

	node_ind++;
    }

    smtHeader *h = (st) ? checked_cast(smtHeader,st->get_header()) : NULL;

    if ( !h )
	return NULL;

    // copy srcbuf
    char * nb = (char *)psetmalloc(len);
    h->src_asize = h->src_size = len;
    strncpy(nb, h->srcbuf, len);
    h->srcbuf = nb;
    h->tidy_tree();

    if ( st != en ) {
	h->parsed = 1;
	root = st->get_parent();
    } else {
	root = st;
    }

    objArr& nodes = *(objArr*)ob;
    int off1, off2;
    smtTreePtr node1, node2;

    if (&region) {
	int no_reg = region.size();
	for (reg_ind = 0 ; reg_ind < no_reg ; reg_ind += 4) {
	    node1 = checked_cast(smtTree,nodes[region[reg_ind]]);
	    off1 = region[reg_ind+1];

	    node2 = checked_cast(smtTree,nodes[region[reg_ind+2]]);
	    off2 = region[reg_ind+3];
	    
	    node1 = node1->get_first_leaf();
	    node2 = node2->get_last_leaf();

	    smt_cut_tail(node2, off2);
	    smt_cut_head(node1, off1);
	}
    }
    return root;
}


void smt_real_split( smtTree *trg, int off, smtTree * ){
 Initialize(smt_split);
   if ( off == -1 || off == 0 || trg == 0 || trg->get_first() )
      return;

   smt_to_untok( trg );
   if ( off > trg->tlth )
      return;

   smtHeaderPtr h = checked_cast(smtHeader,trg->get_header());
   Assert(h);
   h->last_token = trg;
   smt_def_token(h, trg->tbeg, off, 0, SMTT_untok, 0, 0);
   smtTree *nxt = trg->get_next();
   Assert(nxt);
   nxt->newlines = 0;
   nxt->extype = SMTT_untok;
   h->untok_flag = 1;
   smt_touch(trg,1);
   trg->ndm = 1;
//   smt_tidy_markers( trg, off, 0, CUT_SPLIT);
}

/*
   START-LOG-------------------------------------------

   $Log:


   END-LOG---------------------------------------------

*/
