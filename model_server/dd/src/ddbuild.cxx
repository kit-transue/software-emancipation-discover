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
#include <stdlib.h>
#include <assert.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
namespace std {};
using namespace std;
#include <cassert>
#endif /* ISO_CPP_HEADERS */
#ifndef ISO_CPP_HEADERS
#include <memory.h>
#else /* ISO_CPP_HEADERS */
#include <memory>
#endif /* ISO_CPP_HEADERS */
#ifndef ISO_CPP_HEADERS
#include "ctype.h"
#else /* ISO_CPP_HEADERS */
#include <cctype>
#endif /* ISO_CPP_HEADERS */

#ifndef PROTO
#define PROTO(ARGS) ARGS
#endif

#include "RTL.h"
#include "RTL_apl_extern.h"
#include "_Xref.h"
#include "astTree.h"
#include "ast_cplus_tree.h"
#include "ast_hook.h"
#include "astext.h"
#include "cLibraryFunctions.h"
#include "customize_util.h"
#include "ddHeader.h"
#include "ddict.h"
#include "genArr.h"
#include "gen_hash.h"
#include "objArr.h"
#include "objOper.h"
#include "objRelation.h"
#include "representation.h"
#include "smt.h"
#include "xref.h"
#include "RTL_externs.h"
#define BINFO_TYPE(NODE) ((NODE)->TREE_TYPE ())

#define DD_BUILD_BUFFER_SIZE 4096
/* replace lang_flag_2 by vindex
   macro?
*/
#define TYPE_CONTEXT(node) ((node)->context)
#define DD_TYPE_STRING_SIZE 4096
#define MAX_TMPL_SIZE 100
objSet sh_set;

static objSet c_d_set;
static objArr c_d_arr;

static objSet td_set;
static objArr td_arr;

typedef char *chp;

int dd_type_string( ddElement*, astDecl *decl, astType *dtype,
		    char *ps, int *tl, int tmax, objArr_Int *);

genArr(typeDef) tda;
genHash(astDecl,ddElement);
genHashOf(astDecl,ddElement) *ast_to_dd = NULL;

genHashOf(astDecl,ddElement) *get_ast_to_dd() {return ast_to_dd;}

ddElement *lookup_ast_to_dd(astDecl * decl, ddRoot *dr)
  /* if dr, return ep of dr, otherwise, the current ep */
{
  ddElement *ep = NULL;
  if (!decl) return ep;
  if (get_ast_to_dd() && (ep = get_ast_to_dd()->lookup(decl))) {
    if (dr && (ep->get_dd_root() != dr))
      ep = 0;
  }
  return ep;
}

static struct opTable{
  const char *in;
  const char *out;
} optable[] = {
  "nw", "operator new",			/* new (1.92, ansi) */
  "dl", "operator delete",		/* new (1.92, ansi) */
  "new", "operator new",		/* old (1.91) */
  "delete", "operator delete",		/* old (1.91) */
  "ne", "operator !=",			/* old, ansi */
  "eq", "operator ==",			/* old, ansi */
  "ge", "operator >=",			/* old, ansi */
  "gt", "operator >",			/* old, ansi */
  "le", "operator <=",			/* old, ansi */
  "lt", "operator <",			/* old, ansi */
  "plus", "operator +",			/* old */
  "pl", "operator +",			/* ansi */
  "apl", "operator +=",			/* ansi */
  "minus", "operator -",			/* old */
  "mi", "operator -",			/* ansi */
  "ami", "operator -=",			/* ansi */
  "mult", "operator *",			/* old */
  "ml", "operator *",			/* ansi */
  "aml", "operator *=",			/* ansi */
  "convert", "operator +",		/* old (unary +) */
  "negate", "operator -",		/* old (unary -) */
  "trunc_mod", "operator %",		/* old */
  "md", "operator %",			/* ansi */
  "amd", "operator %=",			/* ansi */
  "trunc_div", "operator /",		/* old */
  "dv", "operator /",			/* ansi */
  "adv", "operator /=",			/* ansi */
  "truth_andif", "operator &&",		/* old */
  "aa", "operator &&",			/* ansi */
  "truth_orif", "operator ||",		/* old */
  "oo", "operator ||",			/* ansi */
  "truth_not", "operator !",		/* old */
  "nt", "operator !",			/* ansi */
  "postincrement", "operator ++",	/* old */
  "pp", "operator ++",			/* ansi */
  "postdecrement", "operator --",	/* old */
  "mm", "operator --",			/* ansi */
  "bit_ior", "operator |",		/* old */
  "or", "operator |",			/* ansi */
  "aor", "operator |=",			/* ansi */
  "bit_xor", "operator ^",		/* old */
  "er", "operator ^",			/* ansi */
  "aer", "operator ^=",			/* ansi */
  "bit_and", "operator &",		/* old */
  "ad", "operator &",			/* ansi */
  "aad", "operator &=",			/* ansi */
  "bit_not", "operator ~",		/* old */
  "co", "operator ~",			/* ansi */
  "call", "operator ()",			/* old */
  "cl", "operator ()",			/* ansi */
  "cond", "operator ?:",			/* old */
  "alshift", "operator <<",		/* old */
  "ls", "operator <<",			/* ansi */
  "als", "operator <<=",			/* ansi */
  "arshift", "operator >>",		/* old */
  "rs", "operator >>",			/* ansi */
  "ars", "operator >>=",			/* ansi */
  "component", "operator ->",		/* old */
  "rf", "operator ->",			/* ansi */
  "indirect", "operator *",		/* old */
  "method_call", "operator ->()",	/* old */
  "addr", "operator &",			/* old (unary &) */
  "array", "operator []",		/* old */
  "vc", "operator []",			/* ansi */
  "compound", "operator ,",		/* old */
  "cm", "operator ,",			/* ansi */
  "nop", "operator ",			/* old (for operator =) */
  "as", "operator =",			/* ansi */
  "rm", "operator ->*",
  "vn", "operator new []",
  "vd", "operator delete []",
  "opb", "operator bool",
  0, 0,
};

const char *get_optable(char *in)
{
  while (in && (*in == '_')) in++;
  if ((in == 0) || (*in == 0)) return 0;
  opTable * ot = optable;
  while (ot->in) {
    if (strcmp(in, ot->in) == 0) return ot->out;
    ot ++;
  }
  return 0;
}

static int is_paraset_name(const char *ns)
{
  if (!ns) return 0;
  int ret = 1;
  const char * p = (const char *) strchr(ns, JOINER);
  if (p && p[1] && (p[1] != JOINER))
    ret = 0;
  return ret;
}

int dd_has_func_arg_rel(ddElement* de)
{
  Initialize(dd_has_func_arg_rel);

  RTLNode* arg_root;
  arg_root= checked_cast(RTLNode,get_relation(FuncArgListOf,de));
  if (arg_root)
    return 1;
  return 0;
}

int dd_is_structure(ddKind k)
{
  return (k == DD_CLASS || k == DD_UNION || k == DD_INTERFACE);
}

//
// returns: 0: not a scoped entity, !=0: scoped entity
//
int
dd_is_scoped_entity(ddKind kind)
{
    return (kind==DD_CLASS || kind==DD_UNION || kind==DD_INTERFACE ||
            kind==DD_ENUM || kind==DD_TYPEDEF || kind==DD_PACKAGE);
}

ddRoot* dd_sh_get_dr(app* sh)
{
  Initialize(dd_sh_get_dr);

  if (sh == 0) return 0;
  ddRoot* dr = checked_cast(ddRoot, get_relation(ddRoot_of_smtHeader, sh));
  if (dr == NULL) {
    dr = new ddRoot(sh);
    put_relation(ddRoot_of_smtHeader, sh, dr);
  }
  if (!is_projHeader(sh))
    sh_set.insert(sh);
  return dr;
}

ddRoot* dd_sh_get_dr(smtHeader* sh)
{
  Initialize(dd_sh_get_dr);

  if (sh == 0) return 0;
  ddRoot* dr = checked_cast(ddRoot, get_relation(ddRoot_of_smtHeader, sh));
  if (dr == NULL) {
    dr = new ddRoot(sh);
//    put_relation(ddRoot_of_smtHeader, sh, dr);
  }
  sh_set.insert(sh);
  return dr;
}

void put_func_arg_rtl_1(ddElement* ep)
{
  Initialize(put_func_arg_rtl_1);

  ddElement* de;
  for(de = ep ; de ;
      de = checked_cast(ddElement, de->get_next())) {
    if (de->get_kind()==DD_FUNC_DECL && de->get_is_def() &&
	!dd_has_func_arg_rel(de) ) {
      RTLPtr arg_RTL= new RTL((char *)NULL);
      RTLNode* arg_root= checked_cast(RTLNode,arg_RTL->get_root());
      put_relation(FuncArgListOf, de, arg_root);
      for (ddElement* child = checked_cast(ddElement,de->get_first());
	   child; child = checked_cast(ddElement,child->get_next()))
	rtl_add_obj(arg_root, child);
    }
    else if (de->get_kind() == DD_CLASS ||
	     de->get_kind() == DD_INTERFACE ||
	     de->get_kind() == DD_FUNC_DECL_LIST ||
	     de->get_kind() == DD_TYPEDEF_LIST)
      put_func_arg_rtl_1(checked_cast(ddElement, de->get_first()));
  }
}

char * get_type_name_from_struct(astDecl * tdecl)
{
  if (tdecl == 0) return 0;
  astIdentifier* id = tdecl->DECL_NAME();
  if (!id ||  strchr(id->IDENTIFIER_POINTER(), JOINER)) {
    astType * ttype = tdecl->TREE_TYPE();
    if (ttype && (ttype->TREE_CODE() == MC_ERROR_NODE)) return 0;
    int notdone = 1;
    astDecl *ccdecl = tdecl;
    while (notdone) {
      ccdecl = (astDecl*) ccdecl->TREE_CHAIN();
      astType * ctype = (ccdecl)?ccdecl->TREE_TYPE():0;
      if (ctype && (ctype->TREE_CODE() == MC_ERROR_NODE)) return 0;
      if ((ctype == 0) || (ctype->TYPE_MAIN_VARIANT() != ttype))
	notdone = 0;
      else if (ccdecl->TREE_CODE() == MC_TYPE_DECL) {
	id = ccdecl->DECL_NAME();
	notdone = 0;
      }
    }
  }
  return id?id->IDENTIFIER_POINTER():0;
}



void get_qualify_type_name(astType *type, genString &name_str)
{

  if (type && (type->TREE_CODE() == MC_RECORD_TYPE ||
	       type->TREE_CODE() == MC_ENUMERAL_TYPE ||
	       type->TREE_CODE() == MC_UNION_TYPE )) {
    get_qualify_type_name((astType *) TYPE_CONTEXT(type), name_str);
    astDecl * ccdecl = (astDecl*)type->TREE_CHAIN();  /* typedecl of type */
    astTree * at = type->TYPE_NAME();
    const char *name = 0;
    if (at == 0) return;
    else if (at->TREE_CODE() == MC_IDENTIFIER_NODE)
      name = ((astIdentifier*) at)->IDENTIFIER_POINTER();
    else
      name = (type)->TYPE_NAME_STRING();
    if (name && !is_paraset_name(name) && ccdecl && (ccdecl->TREE_TYPE() == type))
      name = get_type_name_from_struct(ccdecl); 
    if (!is_paraset_name(name))  {
      if (type->TREE_CODE() == MC_RECORD_TYPE)
	name = "NAMELESS_STRUCTURE";
      else if (type->TREE_CODE() == MC_UNION_TYPE)
	name = "NAMELESS_UNION";
      else if (type->TREE_CODE() == MC_ENUMERAL_TYPE)
	name = "NAMELESS_ENUM";
      else
	name = 0;
    }
    if (name) {
      if (name_str.str())
	name_str += "::";
      name_str += name; 
    }
  }
  return;
}


#if 0 /* XXX: believe this is old AST-stuff */
int get_full_name_from_class_template(astDecl *decl, genString & name)
// get full name from class template, decl has to be class template
//   
{
  Initialize(get_full_name_from_class_template);

  astIdentifier *dname= get_ast_identifier(decl);
  name = "template <";
  astVec* vec = (astVec *)decl->DECL_ARGUMENTS();
  int vec_len = (vec)? vec->TREE_VEC_LENGTH() : 0;
  astTree *el;
  astType *dtype;
  for (int i = 0; i < vec_len; ++i) {
    if (i)
      name += ",";
    el = vec->TREE_VEC_ELT(i);
    if (el->TREE_CODE() == MC_TREE_LIST)
      el = ((astList*)el)->TREE_VALUE();
    if ((el->TREE_CODE() == MC_IDENTIFIER_NODE) ||
	(el->TREE_CODE() == MC_TYPE_DECL))
      {
      name += "class";
      char num[MAX_TMPL_SIZE];
      OSapi_sprintf(num, "%d", i);
      name += num;
    }
    else {
      char string_buffer[DD_BUILD_BUFFER_SIZE];	
      dtype =  ((astDecl*)el)->TREE_TYPE();
      int tl;
      dd_type_string(NULL, (astDecl*) el, dtype, string_buffer, &tl, DD_BUILD_BUFFER_SIZE, 0);
      name += string_buffer;
    }
  }
  name += "> ";
  return name.length();
}
#endif /* XXX: old AST */

#ifdef XXX_get_identifier_namen
const char *get_identifier_name(astIdentifier *id)
  /* only meaning full for id from DECL_NAME */
{
  if (!id) return 0;
  const char *idn = id->IDENTIFIER_POINTER();
  if (idn && idn[0] == '_' && idn[1] == '_') {
    const char *out = get_optable((char *) idn);
    if (out)
      idn = out;
  }
  return idn;
}
#endif
#ifdef XXX_smt_tok_get_def
ddElement *smt_tok_get_def(smtTree* tok)
{
  Initialize(smt_tok_get_def);

  smtTree* smt = checked_cast(smtTree, get_relation(decl_of_id, tok));
  if (smt)
    return (checked_cast(ddElement, get_relation(def_dd_of_smt, smt)));
  return NULL;
}
#endif
