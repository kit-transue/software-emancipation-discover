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
#include "dfa.h"
#include "dfa_db.h"
#include "dfa_graph.h"
#include "Set.h"
#include "symbolArr.h"
#include "ddict.h"
//#include <ddSelector.h>
#include "xref_queries.h"

astTokenHash::astTokenHash(astRoot *r)
{
  insert(r->get_astNode());
}

static ddElement *
dfa_get_dd(astNode *node)
{
  ddElement *dd = NULL;
  int code = node->get_code();

  if (code == DFA_REF_DD)
    dd = ((dfa_ref_dd *) node)->get_dd();
  else if (code == DFA_REF_DFA) {
    dfa_decl *dcl = ((dfa_ref_dfa *) node)->get_decl();
    if (dcl)  
      dd = dcl->get_dd(); 
  }
  return dd;
}

void astTokenHash::insert(astNode *node)
{
  int code = node->get_code();

  switch (code) {
  case DFA_REF_DD:
  case DFA_REF_DFA:
    if (node->get_start() > 0)
      add(*(Object *) node);
    break;

  default:
    for (node = node->get_first(); 
	 node; 
	 node=node->get_next())
      insert(node);
    break;
  }
}

unsigned 
astTokenHash::hash_element(const Object *ob) const
{
  astNode *node = (astNode *) ob;
  return 
    node->get_start();
}

bool 
astTokenHash::isEqualObjects(const Object &o1, 
			     const Object &o2) const
{
  return 
    &o1 == &o2;
}

int 
astTokenHash::find(unsigned hash, 
		   int &idx, 
		   Object *&ob) const
{
  register int found = 0;
  register int i;
  
  for (i = h(hash); 
       (contents[i] != nil && !found); 
       i = (i - 1) & mask) {
    Object *oo= (Object *) contents[i];
    if (oo && (hash_element(oo)) == hash) {
      idx = i;
      ob = oo;
      found = 1;
    }
  }
  return found;
}

//returns 1 if found, 0 otherwise
int 
astTokenHash::next(unsigned hash, 
		   int &idx, 
		   Object *&ob) const
{
  register int found = 0;
  register int i;

  for (i = (idx - 1) & mask; 
       (contents[i] != nil && !found); 
       i = (i - 1) & mask) {
    Object *oo= (Object *) contents[i];
    if (oo && (hash_element(oo)) == hash) {
      idx = i;
      ob = oo;
      found = 1;
    }
  }
  return found;
}

int 
astTokenHash::lookup(astXref *axr, 
		     int off, 
		     symbolArr &res)
{
  Object *cur = 0;
  int idx = 0;
  int no_found = 0;

  for (int found = find(off, idx, cur); 
       found; 
       found = next (off, idx, cur)) {
    astNode *node = (astNode *) cur;
    symbolPtr sym(node, axr);
    res.insert_last(sym);
    ++no_found;
//    int the_ind = idx;
  }
  return no_found;
}

void 
astRoot::get_tokens(int off, 
		    symbolArr &results)
{
  if (!tokens)
    tokens = new astTokenHash(this);  // NULL constr; delete unload/destr of astRoot

  tokens->lookup(xref, off, results);
}

int 
instance_get_ast(const symbolPtr &sym, 
		 symbolArr &results)
{
  Initialize(instance_get_ast);
  IF(!sym.is_instance())
    return 0;
  
  int off = ((symbolPtr &) sym).char_offset();
  symbolPtr scope = ((symbolPtr &) sym).scope();
  symbolPtr xsym = sym.get_xrefSymbol();

  app *head = scope.get_def_app();
  IF(!head)
    return 0;

  astRoot *ar = (astRoot *) get_relation(astRoot_of_smtHeader,head);
  if (!ar)
    return 0;

  symbolArr all;
  ar->get_tokens(off, all);
  int sz = all.size();
  int no_ast = 0;

  for (int ii = 0; ii < sz; ++ii) {
    symbolPtr cur = all[ii].get_xrefSymbol();
    if (cur.sym_compare(xsym) == 0) {
       results.insert_last(all[ii]);
       ++no_ast;
    }
  }
  return no_ast;
}

int astXref_get_astNodes(astXref *,ddElement *, symbolArr &);

int dataCell_get_astNode(const symbolPtr &, symbolPtr &);

symbolPtr 
symbolPtr::get_astNode()
{
  symbolPtr ast;
  int doc_flag;
  genString moduleName;

  if (isnull())
    ;
  else if (is_ast())
    ast = *this;
  else if (is_dataCell())
    dataCell_get_astNode(*this, ast);
  else if (is_instance()) {
    symbolArr arr;
    int no_ast = instance_get_ast(*this, arr);
    if (no_ast > 0)
      ast = arr[0];
  } else if (is_xrefSymbol()) {
    symbolArr as;
    ddElement *dd = NULL;
    app *head = NULL;

    switch (get_kind()) {
    case DD_MODULE:
    case DD_FUNC_DECL:
    case DD_VAR_DECL:
    case DD_CLASS:
	case DD_TEMPLATE:
      head = get_def_app();

      if (head) {
	astXref *axr = app_get_astXref(head);

	if (axr) { 
	  ddElement *dd = get_def_dd();

	  if (dd)
	    astXref_get_astNodes(axr, dd, as);

	  if (as.size())
	    ast = as[0];
	}
      }
    }
  }
  return ast;
}

int astXref_get_offset(astNode *,astXref *);

#define NOTXREFSYM 0x00000000

symbolPtr::symbolPtr(astNode *nd, 
		     astXref *axr)
{
  Initialize(symbolPtr::symbolPtr(astNode *, astXref *));

  if (nd && axr) {
    un.ap = axr;
    offset = astXref_get_offset(nd,axr);
    offset |= 0x40000000;  // mark offset as ast, not xref
  } else {
    un.cp = NULL;
    offset=NOTXREFSYM;
  }
}
