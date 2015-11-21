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
#include <scopeLeaf.h>
#include <cLibraryFunctions.h>
#include <msg.h>
#include <ParaCancel.h>
#include <xrefSymbolSet.h>
#include <scopeMgr.h>
#include <scopeUnit.h>
#include <scopeClone.h>

init_relational(scopeLeaf,scopeProj);
init_relational(scopeSlashLeaf,scopeLeaf);

symbolPtr scopeLeaf::get_visible_def_symbol (symbolPtr& sym, xrefSymbol* dfs)
{
    Initialize(scopeLeaf::get_visible_def_symbol);
    symbolPtr retval = NULL_symbolPtr;
    objArr all_leaves;

    scopeClone *cl   = get_unit_clone ();
    if (cl)
	cl->pmod_scopes_cloned(all_leaves);
    else {
	scopeUnit *un = unit();
	un->pmod_scopes_cloned(all_leaves);
    }

    char const *nm   = sym.get_name();
    ddKind knd = sym.get_kind();

    Obj *cur;
    ForEach(cur,all_leaves){
	scopeLeaf *lf = checked_cast(scopeLeaf,cur);
	if (lf == this)
	    break;
	    
	retval = symbolPtr::get_def_symbol_for_leaf (lf, knd, nm, dfs);
	if (retval.isnotnull())
	    break;
    }
    return retval;
}

static void add_symbol (symbolArr& to, symbolSet& modset, symbolPtr& sym, scopeNode *sn)
{
    Initialize(add_symarr);
    if (sym.isnotnull()) {
	if (sn) sym.put_scope (sn);
	if (!modset.includes(sym)) {
	    modset.insert(sym);
	    to.insert_last(sym);
	}
    }
    return;
}

void scopeLeaf::get_pmod_modules (symbolArr& modules, symbolSet& modset, scopePmodQuery qmode)
{
    Initialize(scopeLeaf::get_pmod_modules);
    projNode *pn = clone_get_proj(this);
    if (!pn) return;

    scopeUnit *un = unit();
    if (!un || qmode == PMOD_ALL) {
	xref_get_all_pmod_modules (modset, modules, pn, this);
	return;
    }
    symbolArr all_modules;
    xref_get_all_pmod_modules (modset, all_modules, pn, NULL, 1);
    switch (qmode) {
    case PMOD_VISIBLE: 
	{
	    symbolPtr sym;
	    ForEachS(sym,all_modules){
		if(ParaCancel::is_cancelled()) return;
		if (!un->is_hidden_symbol(sym))
		    add_symbol (modules, modset, sym, this);
	    }
	    break;
	}
    case PMOD_SLASH:
	msg("Not implemented yet!\n") << eom;
	break;
    case PMOD_VISIBLE_SLASH:
	{
	    symbolPtr sym;
	    ForEachS(sym,all_modules){
		if(ParaCancel::is_cancelled()) return;
		if (!un->is_hidden_symbol(sym))
		    add_symbol (modules, modset, sym, this);
		else {
		    xrefSymbol *def_file = NULL;
		    symbolArr as;
		    sym.get_local_link (is_defined_in,as);
		    if (as.size())
			def_file = as[0].operator->();
		    symbolPtr subst = get_visible_def_symbol (sym, def_file);
		    add_symbol (modules, modset, subst, NULL);
		}
	    }
	    break;
	}
    }
}

void scopeLeaf::pmod_scopes_cloned (objArr& oarr) const
{
    Initialize(scopeLeaf::pmod_scopes_cloned);
    projNode *pn = clone_get_proj(this);
    if (pn) {
	Xref * Xr;
	if ( (Xr = pn->get_xref(1)) )
	    if (Xr->get_lxref() && !Xr->is_unreal_file())
		oarr.insert_last(this);
    }
}

void scopeLeaf::print(ostream& os, int ) const
{
  send_string(os);
  os << " unit " << unit() << " Xref " << xref();
}

implement_copy_member(scopeLeaf);
implement_copy_member(scopeSlashLeaf);

Xref*        scopeLeaf::xref() const{ return  pxref;}
scopeLeaf*   scopeLeaf::leaf() const{ return  (scopeLeaf*)this;}
scopeRoot*   scopeLeaf::root() const{ return  proot;}
scopeUnit*   scopeLeaf::unit() const{ return punit ;}
bool      scopeLeaf::is_xref() const{ return false;}
bool      scopeLeaf::is_leaf() const{ return true ;}

void scopeLeaf::get_symbols (symbolPtr& sym, symbolArr& sarr, xrefSymbol *dfs, int is_def)
{
    Initialize(scopeLeaf::get_symbols);
    if (sym.isnull()) return;

    char const *nm      = sym.get_name();
    ddKind knd    = sym.get_kind();
    projNode *pr  = clone_get_proj (this);
    scopeUnit *un = unit();

    if (!pr || !un)
	return;

    symbolArr tmp_arr;
    symbolPtr::lookup_all_for_project(pr, tmp_arr, knd, nm, dfs);

    symbolPtr cur;
    ForEachS(cur,tmp_arr){
	if (!un->is_hidden_symbol(cur)) {
	    if ( !is_def || cur.get_has_def_file() ) {
		cur.put_scope(this);
		sarr.insert_last(cur);
	    }
	}
    }
}

void scopeLeaf::get_contents (symbolArr& sarr)
{
    Initialize(scopeLeaf::get_contents);
    int depth = 0;
    objArr tmp;

    get_modules(tmp, depth);

    Obj *cur;
    ForEach(cur,tmp){
	sarr.insert_last((Relational *)cur);
    }
}

scopeClone *scopeLeaf::get_unit_clone ()
{
    Initialize(scopeLeaf::get_unit_clone);
    scopeClone *unit_clone = NULL;
    for (scopeClone *cl = get_parent() ;
	 cl ;
	 cl = cl->get_parent())
	{
	    scopeRoot *master = clone_get_master (cl);
	    if (master && is_scopeUnit(master)) {
		unit_clone = cl;
		break;
	    }
	}

    return unit_clone;
}
