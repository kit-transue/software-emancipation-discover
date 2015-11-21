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

// File smt.C - SMODE 
//
#include "smt.h"
#include "machdep.h"

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <strstream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#include <sstream>
#endif /* ISO_CPP_HEADERS */

#include <ddict.h>
#include <dd_or_xref_node.h>
#include <cmd_enums.h>
#include <cmd.h>
#include <fileEntry.h>
#include <scopeMaster.h>
#include <scopeUnit.h>
#include <genStringPlus.h>

int get_display_string_from_symbol(symbolPtr sym, genString &txt);

smtTree * smt_find_stmt_node(smtHeader * h, int tn)
  {
   Initialize(smt_find_stmt_node);

  smtTree * n = h->tn_na(tn);		// find token node
  if(n == 0) return 0;
  while(n && (n->type == SMT_token || n->type == SMT_group))
    {
    n = checked_cast(smtTree,n->get_parent());
    } 
  return n;
  }

// xref_not_header_file() Checks header file ext against those listed in preferences.
int smt_check_file_ext(char * fname)
  {
  if(fname == 0)
    return 0;
  return xref_not_header_file(fname);
}

extern symbolPtr get_semtype_symbol(symbolPtr);
//extern int get_c_proto(const symbolPtr&, genString&);
extern int scope_get_global_mode();
extern const char * ATT_rscope(symbolPtr & sym);
extern const char* ATT_attribute(symbolPtr& sym);

//this functionality is not provided by els/src/PrintSymbol.C 
//these functions are used only for compatibility with old tests
static const char* scope_type(scopeNode*sc)
{
  const char* nm = "SCOPE";
  if(is_scopeUnit(sc))
    nm = "UNIT";
  else if(is_scopeDLL(sc))
    nm = "DLL";
  else if(is_scopeEXE(sc))
    nm = "EXE";
  else if(is_scopeSLL(sc))
    nm = "SLL";
  return nm;
}

void smt_get_token_definition (smtTree *token, genString& txt)
{
    Initialize(smt_get_token_definition);

    ostringstream os;
    ddElement * ddel;
    smtTreePtr token_def = 0;
  
    if (!(ddel = smt_get_dd(token))) {
	token_def = checked_cast(smtTree, get_relation(declaration_of_reference, token));
        if (token_def) {
	  ddel = checked_cast(ddElement , get_relation(def_dd_of_smt, token_def));      
	  if (!ddel)
	    token_def = token_def->get_parent();
	}
	else
	    token_def = checked_cast(smtTree, get_relation(decl_of_id, token));
    }

    if (!ddel) {
      if (token_def)
	token_def->send_string(os);
      os << ends;
      txt = os.str().c_str();
      return;
    }
    else {
      ddKind kind = ddel->get_kind();
      symbolPtr sym = ddel;
      sym = sym.get_xrefSymbol();
      if (sym.xrisnotnull())
	get_display_string_from_symbol(sym, txt);
      else {			/* something abnormal here */
	const char * nm = (const char *)0;
	if(kind != DD_FUNC_DECL) {
	  if (nm = ddKind_name(kind)) {
	    if (strlen(nm) > 3) {
	      os << nm+3 << ' ';
	    }
	  }
	}
	if (nm = ddel->find_def()) {
	  os << nm << ' ';
	}
	if (nm = ddel->get_ddname()) {
	  os << nm;
	}
	// -- filename
	genString phys_name;
	char const *def_file = ddel->get_def_file();
	if (ddel->get_kind() == DD_MODULE && def_file && *def_file) {
	  projHeader::ln_to_fn(def_file, phys_name);
	  if (phys_name.str())
	    def_file = realOSPATH( (char*)phys_name.str() );
	}
	if (def_file) {
	  os << " ==> " << def_file;
	}
	os << ends;
	txt = os.str().c_str();
      }
    }

}

static void check_args_for_strings(smtTree *smt, ddElement *dd_func)
{
    smtTree *func_token = smt;
    while(smt){
	smt = (smtTree *)smt->get_next();
	if(smt && smt->type == SMT_expr){
	    smtTree *tok = (smtTree *)smt->get_first();
	    if(tok->type == SMT_token && tok->extype == SMTT_string){
		ddElement *dd_str = (ddElement *)get_relation(ref_dd_of_smt, tok);
		if(dd_str){
		    put_relation(has_friend, dd_func, dd_str);
		}
	    }
	}
    }
}

static void check_functions_args(ddElement *func)
{
    Initialize(check_args_for_strings);
    
    Obj *smts = get_relation(ref_smt_of_dd, func);
    if(smts == NULL)
	return;
    Obj *ob;
    ForEach(ob, *smts) {
	smtTree *smt = (smtTree *)ob;
	check_args_for_strings(smt, func);
    }
}


/*
//------------------------------------------
// $Log: smt.cxx  $
// Revision 1.36 2001/07/25 20:42:58EDT Sudha Kallem (sudha) 
// Changes to support model_server.
Revision 1.2.1.6  1993/08/31  21:41:22  boris
Fixed bug #4646  with Crash in smt_get_token_definition()

Revision 1.2.1.5  1993/08/28  00:07:15  boris
Fixed bug #4518 with strange message on double click

Revision 1.2.1.4  1993/08/05  22:29:58  boris
Added smt_get_token_definition () function


*/




