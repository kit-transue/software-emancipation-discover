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

//////////////////////////   FILE dd_do_refs.C  //////////////////////////
//
//
//
// History:   02/12/92     John Cameron   Initial coding
//-------------------------------------------------------
// synopsis:    links calls with function definitions in the SMT
//              call -- function pairs are collected by ast_call_hook
//                      and processed here
//------------------------------------------


#include "objOper.h"

#include "astTree.h"
#include "smt.h"
#include <ddict.h>
#include "xref.h"
#include "genArr.h"
#include "ast_cplus_tree.h"
#include "ast_hook.h"
#include "customize_util.h"
#include "smt_categories.h"

#include <dd_or_xref_node.h>

smtTree* dd_get_smt(ddElement*);
ddRoot* dd_sh_get_dr(smtHeader*);
int dd_is_structure(ddKind k);

int not_title(smtTree*);

smtTreePtr dd_get_token_def( smtTreePtr tok )
  {
  Initialize(dd_get_token_def);
  ddElementPtr ep;
  smtTreePtr smt;
  ep = checked_cast(ddElement, get_relation(ref_dd_of_smt, tok));
  if (ep == checked_cast(ddElement, NULL))
    {
    smt = checked_cast(smtTree, get_relation(declaration_of_reference, tok));
    if (smt && smt->get_node_type()==SMT_decl)
      smt = checked_cast(smtTree,smt->get_parent());
    ReturnValue(smt); // should be an error
    }
  ddElement* def_dd = ep->get_definition();
  if (def_dd == 0) return tok;
  smt = checked_cast(smtTree, get_relation(def_smt_of_dd, def_dd));
  if (smt && ((smt->get_node_type()==SMT_decl) ||
              (ep->get_kr() && smt->get_node_type() ==SMT_pdecl))) {
    smt = checked_cast(smtTree,smt->get_parent());
  }
  ReturnValue(smt);
  }

appTreePtr dd_get_loaded_def_or_file( smtTreePtr tok )
{
  Initialize(dd_get_loaded_def_or_file);
  ddElementPtr ep;
  smtTreePtr smt;
  ep = checked_cast(ddElement, get_relation(ref_dd_of_smt, tok));
  if (ep == checked_cast(ddElement, NULL))
  {
    smt = checked_cast(smtTree, get_relation(declaration_of_reference, tok));
    if (smt && smt->get_node_type()==SMT_decl)
      smt = checked_cast(smtTree,smt->get_parent());
    // AlexZ changes : opening definition of function from flowchart does not work
    if(smt != NULL)
	ReturnValue(smt);		// should be an error
    ep = checked_cast(ddElement, get_relation(def_dd_of_smt, tok));    
    if(ep == checked_cast(ddElement, NULL)){
	smt = checked_cast(smtTree,tok->get_parent());
	if(smt)
	    ep  = checked_cast(ddElement, get_relation(def_dd_of_smt, smt));    
	if(ep == checked_cast(ddElement, NULL))
	    return NULL;
    }
}

  symbolPtr dfs  = ep->get_definition_file(); 
  if (dfs->xrisnull()) return 0;
  projNode * pmod_proj = projHeader::get_pmod_project_from_ln(dfs.get_name());
  app * ah = 0;
  if (pmod_proj)
    ah = pmod_proj->restore_file_wo_module(dfs.get_name());
  ddElement *def_dd = 0;
  if (ah) {
    ddRoot* dr = dd_sh_get_dr(ah) ;
    if (dr == 0) return 0;
    def_dd = dr->lookup(ep->get_kind(), ep->get_ddname());
  }
  if (def_dd == 0 && ah) {
    return (appTreePtr(ah->get_root()));
  }
  if (ah == 0 && pmod_proj) {
    genString fn;
    pmod_proj->ln_to_fn(dfs.get_name(), fn);
    if (fn.str()) {
      ah = projHeader::get_file(fn);
      if (ah)
	return (appTreePtr(ah->get_root()));
    }
  }
  if (ah == 0) return 0;
    
  smt = checked_cast(smtTree, get_relation(def_smt_of_dd, def_dd));
  if (smt && ((smt->get_node_type()==SMT_decl) ||
              (ep->get_kr() && smt->get_node_type() ==SMT_pdecl))) {
    smt = checked_cast(smtTree,smt->get_parent());
  }
  ReturnValue(smt);
}

appTreePtr dd_get_loaded_definition (symbolPtr sym)
{
    Initialize(dd_get_loaded_definition__symbolPtr);
    appTree *def = NULL;
    
    // This can happen when attempting to open the definition (via inheritance view) 
    // of a constructor automatically generated by the parser.
    if(!sym.isnotnull()) return def;

    if (sym.relationalp ()) {
        symbolPtr nod = sym.get_xrefSymbol ();
        if (nod.isnotnull())
	    sym = nod;
    }

    if (sym.relationalp()) {
	Relational *obj = sym;
	if (obj && is_smtTree(obj))
	    def = checked_cast(appTree,dd_get_loaded_def_or_file((smtTree *)obj));
	else if (obj && is_ddElement(obj)) {
	    ddElementPtr de = checked_cast(ddElement,obj);
	    switch (de->get_kind ()) 
	    {
	        case DD_SOFT_ASSOC: 
		    {
			symbolPtr	sym = de->get_xrefSymbol ();
			symbolArr	arr;
			
			sym->get_link (is_defined_in, arr);
			
			if (arr.size () > 0)
			    def = dd_get_loaded_definition  (arr[0]);
		    }
		    break;
		    
		default:
		    def = dd_get_loaded_smt (de);
		    break;
	    }
	}
    } else {
	if (sym.is_loaded_or_cheap ()) {
	    appTree *nd = sym;
	    if (nd) {
		if (is_smtTree(nd)) {
		    if (sym->get_kind() == DD_MODULE) 
			def = nd;
		    else
			def = NULL;
		} else if (is_ddElement(nd)) {
		    ddElement *de = checked_cast(ddElement,nd);
		    def = dd_get_loaded_smt (de);
		} else if (is_dd_or_xref_node (nd)) {
		    nd = ((dd_or_xref_nodePtr)nd)->get_dd();
		    if (is_ddElement (nd)) {
			Obj* smts = get_relation (ref_smt_of_dd, nd);
			Obj* cur;
			
			ForEach (cur, *smts) {
			    def = checked_cast(appTree,cur);
			    break;
			}
		    }
		}
	    }
	}
    }
    return def;
}



/*
$Log: dd_do_refs.cxx  $
Revision 1.38 2001/07/25 20:41:27EDT Sudha Kallem (sudha) 
Changes to support model_server.
Revision 1.2.1.52  1994/08/04  22:09:10  trung
Bug track: 0
7942, 7955, 7972, 7974, 7978

Revision 1.2.1.51  1994/07/28  22:57:49  trung
Bug track: 7785
7785

Revision 1.2.1.50  1994/07/28  21:14:18  trung
Bug track: 0
bugs: 7785, 7852, 7820, 7716, 7676

Revision 1.2.1.49  1994/07/08  15:22:04  trung
Bug track: 0
7279, 7285, 7372, 7585, 7715, 7582, demangling name for template related
putting attributes for xrefSymbol,

Revision 1.2.1.48  1994/06/25  17:27:43  trung
Bug track: 0
fixes for templates

Revision 1.2.1.47  1994/06/20  16:34:35  trung
Bug track: 0
template submission

Revision 1.2.1.46  1994/05/12  15:20:28  trung
Bug track: 0
fix projModule::need_to_update, delete empty header for shallow parsing, fix opt bug in dd

Revision 1.2.1.45  1994/04/11  14:47:09  trung
bugs 6869, 6871, 6937 & 6909, 6939

Revision 1.2.1.44  1994/03/21  19:40:54  trung
Bug track: 6625
submitting for cpp_open_file, etc.

Revision 1.2.1.43  1994/03/15  15:36:09  davea
Bug track: n/a
Add :



-----Better late than never---
*/





