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
// INCLUDE FILES
#include <cLibraryFunctions.h>
#include <symbolPtr.h>
//#include <browserRTL.h>
#include <genError.h>
#include <ddKind.h>
#include <symbolArr.h>
#include <symbolSet.h>
#include <objArr.h>
#include <xref.h>
#include <proj.h>
#include <ParaCancel.h>
#include <symbolScope.h>
#ifndef _RTL_externs_h
#include <RTL_externs.h>
#endif
#include <scopeClone.h>
#include <scopeMgr.h>
#include <scopeLeaf.h>

int get_remote_flag();

symbolScope::symbolScope() : servers(0)
{
    updated = 0;
    xref_notifier_add(this);
}

// group is a array of project Nodes.
symbolScope::symbolScope(objArr& group) : servers(0), domain(group)
{
    updated = 0;
    xref_notifier_add(this);
}

void symbolScope::set_domain(objArr& group)
{
    Initialize(symbolScope::set_domain);

    if(group != domain){
	domain  = group;
	updated = 0;
    }
}

void symbolScope::get_all_modules(projNode *pn, symbolSet& mod)
{
    Initialize(symbolScope::get_all_modules);
    if (!pn) return;
    scopeUnit *unit = scope_mgr.unit (pn);
    pn->refresh();
    symbolArr& proj_contents = pn->rtl_contents();
    symbolPtr  sym;

    ForEachS(sym, proj_contents)
    {
	if(ParaCancel::is_cancelled())
	    return;
        if (sym.get_kind() == DD_MODULE)
        {
	    //boris: Replace hidden module with visible module
	    projModule *vis_mod = (unit) ? scope_mgr.find_visible_module(sym.get_name(), unit) : NULL;
	    if (vis_mod)
		sym = vis_mod;
	    symbolPtr xref = sym.get_xrefSymbol();
	    if(xref.xrisnotnull() && !mod.includes(xref)){
		mod.insert_last(xref);
		modules.insert_last(xref);
	    }
        }  else if (sym.get_kind() == DD_PROJECT) {
	    appPtr app_head = sym.get_def_app();
	    if(app_head && is_projHeader(app_head)) {
		projNode *child_proj = projNodePtr(app_head->get_root());
		get_all_modules(child_proj, mod);
            }
        }
    }
}

int symbolScope::pmod_on_top(projNode *pn)
{
    Initialize(symbolScope::pmod_on_top);

    projNode* curproj = pn;
    if(curproj->get_xref(1)){
	Xref* xr = (curproj) ? curproj->get_xref(1) : 0;
	if (xr && xr->get_lxref() && !xr->is_unreal_file())
	    return 0;   // This project node has it\'s own pmod.
    }
    while (curproj && !curproj->get_xref(1))
	curproj = checked_cast(projNode, curproj->find_parent());
    Xref* xr = (curproj) ? curproj->get_xref(1) : 0;
    if (xr && xr->get_lxref() && !xr->is_unreal_file())
	return 1;
    return 0;
}

// boris: Assumes that projNode pn either has Xref or it's children have Xref
void symbolScope::query_pmod_for_modules(projNode *pn, symbolSet& mod)
{
    Initialize(symbolScope::query_pmod_for_modules__projNode*__symbolSet&);
    if (!pn) return;
    scope_mgr.get_pmod_modules (mod, modules, pn);
    return;
}

void symbolScope::query_pmod_for_modules(scopeNode *sc, symbolSet& mod)
// sc suppose to be leaf - with pmod
{
    Initialize(symbolScope::query_pmod_for_modules__scopeNode*__symbolSet&);
    if (!sc) return;
    scope_mgr.get_pmod_modules (mod, modules, sc);
    return;
}

void symbolScope::update_modules(void)
{
    Initialize(symbolScope::update_modules);

    modules.removeAll();
    symbolSet modset;
    int size = domain.size();
    for (int i = 0;  i < size; i++){
	Relational *rel = (Relational*) domain[i];
	if (is_projNode(rel)) {
	    projNode *pn = (projNode*)rel;
	    if(pmod_on_top(pn))
		get_all_modules(pn, modset);
	    else
		query_pmod_for_modules(pn, modset);
	} else if (is_scopeNode(rel)) {
	    scopeNode * sc = (scopeNode*)rel;
	    query_pmod_for_modules(sc,modset);
	} else {
	    IF(!is_projNode(rel) && !is_scopeNode(rel))
		;
	}
	if(ParaCancel::is_cancelled())
	    break;
    }
    if(ParaCancel::is_cancelled())
	updated = 0;
    else
	updated = 1;
}

void symbolScope::update_modules(symbolPtr symbol)
{
    Initialize(symbolScope::update_modules);

    if(!symbol.get_has_def_file())
	return;
    symbol = symbol->get_def_file();
    if(symbol.xrisnull())
	return;
    if(modules.includes(symbol))
	return;
    update_modules();
    if(ParaCancel::is_cancelled())
	updated = 0;
    else
	updated = 1;
}

void symbolScope::add_module(symbolPtr symbol)
{
    Initialize(symbolScope::add_module);

//    if(!symbol->get_has_def_file())
//	return;
    symbol = symbol->get_def_file();
    if(symbol.xrisnull())
	return;
    if(modules.includes(symbol))
	return;
    modules.insert_last(symbol);
}

struct subproj
{
  objArr*  scope;
};
genArr(subproj);

struct proscopArr : public subprojArr
{
   ~proscopArr();
   void query(symbolArr& symbols, ddSelector& selector);
};

proscopArr::~proscopArr()
{
  int sz = size();
  for(int ii=0; ii<sz; ++ii){
     subproj *pp = (*this)[ii];
     delete pp->scope;
  }
}

void proj_query_local(objArr&arr, symbolArr& symbols, ddSelector& selector)
{
  symbolScope scope(arr);
  scope.update_modules();
  scope.query_local(symbols, selector);
}

void proscopArr::query(symbolArr& symbols, ddSelector& selector)
{
  int sz = size();
  for(int ii=0; ii<sz; ++ii){
     subproj *pp = (*this)[ii];
     objArr*scope = pp->scope;
       proj_query_local(*scope, symbols, selector);
   }
}

symbolScope::~symbolScope()
{
   if(servers)
	delete servers;
   xref_notifier_rem(this);
}

void symbolScope::query(symbolArr& symbols, ddSelector& selector)
{
    int is_empty = selector.empty();
    if(is_empty)
      return;

    query_local(symbols, selector);

    if(servers)
       servers->query(symbols, selector);

}

void symbolScope::query_local(symbolArr& symbols, ddSelector& selector)
{
    Initialize(symbolScope::query_local);
    
    if(!updated){ 
      update_modules();
    }

    if(ParaCancel::is_cancelled())
	return;
    symbolPtr sym;
    symbolArr non_unique_tmp;
    ForEachS(sym, modules){
	sym.get_define(symbols, selector);
	if(selector.selects(DD_STRING) || selector.selects(DD_LOCAL)){
	    ddSelector sel;
	    sel.want_used = 1;
	    if(selector.selects(DD_STRING))
		sel.add(DD_STRING);
	    if(selector.selects(DD_LOCAL))
		sel.add(DD_LOCAL);
	    sym.get_define(non_unique_tmp, sel);
	}
	if(ParaCancel::is_cancelled()){
	    symbols.removeAll();
	    return;
	}
    }
    non_unique_tmp.remove_dup_syms();
    symbols.insert_last(non_unique_tmp);
}

int symbolScope::is_in_scope(symbolPtr symbol)
{
    Initialize(symbolScope::is_in_scope);

    symbolPtr module = symbol->get_def_file();
    if (!module->xrisnull()){
	symbolPtr sym;
	ForEachS(sym, modules){
	    if(module == sym)
		return 1;
	};
    };
    return 0;
}

void symbolScope::update_proj(projNode* pr) {
   Initialize(symbolScope::update_proj);

   if (domain.includes(pr)) {
      updated = 0;
   }
}
