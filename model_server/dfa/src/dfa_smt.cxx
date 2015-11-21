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
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#include <cstdio>
#endif /* ISO_CPP_HEADERS */
#include <general.h>
#include <msg.h>
#include <genError.h>
#include <astTree.h>
#include <ast_cplus_tree.h>
#include <dfa.h>
#include <smt.h>
#include <ddict.h>
#include <objArr.h>
#include <genArr.h>
#include <machdep.h>

#define ST(node) node->get_start()
#define EN(node) (node->get_start() + node->get_length())

static int dfa_smt_debug = (int)OSapi_getenv("DFA_SMT_DEBUG");

#ifdef DBG
#undef DBG
#endif

#define DBG if(dfa_smt_debug)

static ddElement* dfa_get_dd(astNode * node)
{
  ddElement*dd = NULL;
  tree_code code = (tree_code)node->get_code();
  if(code == (tree_code)DFA_REF_DD)
    dd = ((dfa_ref_dd*)node)->get_dd();
  else if(code == (tree_code)DFA_REF_DFA) {
    dfa_decl * dcl = ((dfa_ref_dfa*)node)->get_decl();
    if (dcl)  
      dd = dcl->get_dd(); 
  }
  return dd;
}


static smtTree* dfa_smt_decl(smtHeader*h, dfa_ref_dfa* node)
{
  dfa_decl * dcl = ((dfa_ref_dfa*)node)->get_decl();
  if (!dcl) // for now if we use "this"
      return NULL;
  astNode* ch = dcl->get_first();
  if (!ch)
    return NULL;
  int off = ch->get_start();
  smtTree*tok = h->cn_na(off);
  smtTree*decl = tok;
  while(decl && decl->type != SMT_decl && decl->type != SMT_pdecl)
    decl = decl->get_parent();
 
  return decl;
}


static void  dfa_tok_report(smtHeader*h, astNode*cur, smtTree*tok)
{
  if(tok) {
    cur->set_start(tok->tbeg);
    cur->set_length(tok->tlth);
  } else DBG {
    msg("dfa_tok_report: ** smt NOT found\n") << eom;
    dfa_print_tree(h, cur, cout);
  }
}

static void  dfa_smt_decompose(int start, int end, Obj& smts, objArr*in, objArr*out = 0)
{
  Obj*el;
  ForEach(el, smts){
    smtTree*tok = (smtTree*) el;
    int ts = tok->istart();
    if(ts >= start  && ts < end){
      if(in)
	in->insert_last(tok);
    } else {
      if(out)
	out->insert_last(tok);
    }
  }
}

static void  dfa_smt_decompose(astNode*par, Obj& smts, objArr*in, objArr*out = 0)
{
  int start = ST(par);
  int end = EN(par);
  dfa_smt_decompose(start, end, smts, in, out);
}

static Obj*dfa_tokens(smtHeader*h, astNode* cur)
{
  Obj*tokens = NULL;    
  int code = (int) cur->get_code();    
  if(code==DFA_REF_DD) {
    ddElement * dd = dfa_get_dd(cur);
    if(dd) 
      tokens= get_relation(ref_smt_of_dd, dd);
  } else if (code == DFA_REF_DFA){
    smtTree*decl = dfa_smt_decl(h, (dfa_ref_dfa*) cur);
    if(decl)
      tokens = get_relation(reference_of_declaration, decl);
  }
  return tokens;
}


typedef astNode *dfaPtr;
genArr(dfaPtr);

bool dfa_degenerate_expr(smtHeader*h, astNode*id1, astNode*id2, astNode*par)
{
  bool res = false;
  smtTree*tok1 = NULL;
  smtTree*tok2 = NULL;
  Obj* tokens = NULL;
  int code = (int) id1->get_code();    
  if(code != id2->get_code())
    return res;

  if(code==DFA_REF_DD) {
    ddElement * dd = dfa_get_dd(id1);
    ddElement * dd2 = dfa_get_dd(id2);
    if(dd && dd == dd2)
      tokens= get_relation(ref_smt_of_dd, dd);
  } else if (code == DFA_REF_DFA){
    smtTree*dec1 = dfa_smt_decl(h, (dfa_ref_dfa*) id1);
    smtTree*dec2 = dfa_smt_decl(h, (dfa_ref_dfa*) id2);
    if(dec1 && dec1 == dec2)
      tokens = get_relation(reference_of_declaration, dec1);
  }

  if(tokens) {
    objArr tokin;
    dfa_smt_decompose(par, *tokens, &tokin);
    int no_tok = tokin.size();
    if(no_tok == 2) {
      tok1 = (smtTree*) tokin[0];
      tok2 = (smtTree*) tokin[1];
      if(tok1->start() > tok2->start()){
	smtTree*tmp = tok1;
	tok1 = tok2;
	tok2 = tmp;
      }
      res = true;
    } else if (no_tok == 1) {
      tok1 = (smtTree*) tokin[0];
      res = true;
    }
  }
      
  dfa_tok_report(h, id1, tok1);
  dfa_tok_report(h, id2, tok2);

  return res;
}

static int dfa_sort_tokens(const void * p1, const void * p2)
{
   smtTree *smt1 = *(smtTree**)p1;
   smtTree *smt2 = *(smtTree**)p2;
   int s1 = smt1->istart();
   int s2 = smt2->istart();
   
   return s1 - s2; 
}
 
static bool dfa_smt_in_range(smtHeader * h, astNode*cur, int start, int end, dfaPtrArr*subs, int which)
{
  // which is used if more then one candidate, 
  // if which = -1; returns fail if more then one candidate

  int res = true;

  smtTree*tok = NULL;
  int subs_sz = 0;
  if(subs)
    subs_sz = subs->size();

  Obj* tokens = dfa_tokens(h, cur);
  if(tokens){
    objArr tokin;
    dfa_smt_decompose(start, end, *tokens, &tokin);
    int no_tok = tokin.size();
    if (no_tok > 1){
      tokin.sort(dfa_sort_tokens);
      tok = (smtTree*) tokin[0];  // just in case it will vanish
      // filter out subexpressions
      for(int jj=0; jj < subs_sz; ++jj){
	astNode* cur_out = *((*subs)[jj]);
        objArr tmp;
	dfa_smt_decompose(cur_out, tokin, NULL, &tmp); 
	tokin = tmp;
      }
    }
    no_tok = tokin.size();
    if(no_tok == 1) {
      tok = (smtTree*) tokin[0];
    } else if(which>= 0 && no_tok > which) {
      tok = (smtTree*) tokin[which];
    } else {
      res = false;
    }
  }
  dfa_tok_report(h, cur, tok);
  return res;
}

static void dfa_process_tokens(smtHeader*h, astNode* par, astNode*block)
{
  dfaPtrArr ids;
  dfaPtrArr subs;
  int no_dd = 0;
  int no_dfa = 0;
  if(par->get_code() == MC_COMPOUND_EXPR)
    block = par;

  astNode*cur = par->get_first();
  // classify children
  for(; cur; cur = cur->get_next()){
    int code = (int) cur->get_code();
    if(code==DFA_REF_DD) {
      ++ no_dd;
      if(!cur->get_length())
        ids.append(&cur);
    } else if (code == DFA_REF_DFA){
      ++ no_dfa;
      if(!cur->get_length())
        ids.append(&cur);
    } else {
        subs.append(&cur);
    }
  }
  int ids_sz = ids.size();
  int subs_sz = subs.size();

// degenerate binary expr: x+x

  if(ids_sz == 2 && subs_sz == 0 && (no_dd != no_dfa)){
     bool res = dfa_degenerate_expr(h, *ids[0], *ids[1], par);
     if(res)
       return;
  }

  int ii;
  int start = ST(par);
  int end = EN(par);
  for(ii = 0; ii < ids_sz; ++ ii){
    cur = *ids[ii];
    dfa_smt_in_range(h, cur, start, end, &subs, 0);
  }

  // recursion
  for(ii = 0; ii < subs_sz; ++ ii){
    cur = *subs[ii];
    dfa_process_tokens(h, cur, block);
  }
}

void dfa_process_tokens(smtHeader*h, astNode* par)
{
  dfa_process_tokens(h, par, NULL);
}

bool dfa_smt_in_range(smtHeader * h, astNode*cur, int start, int end)
{
  return dfa_smt_in_range(h, cur, start, end, NULL, -1);

}

