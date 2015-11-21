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
#include <cLibraryFunctions.h>
#include <scopeNode.h>
#include <objArr.h>
#include <genString.h>
#include <proj.h>
#include <scopeRoot.h>
#include <scopeClone.h>
#include <symbolSet.h>
#include <xrefSymbolSet.h>
#include <ParaCancel.h>
#include <scopeMgr.h>
#include <scopeLeaf.h>
#include <scopeUnit.h>

extern "C" void scopeMgr_find (const char *lname, symbolPtr& sym);


init_relational(scopeNode,appTree);

void scopeClone::construct_prefix_name(genString& st) const
{
    Initialize(scopeClone::construct_prefix_name__genString&);
    scopeNode *prnt = get_parent();
    if(!prnt){
      st = "no name";
      return;
    }

    prnt->construct_prefix_name(st);
    scopeRoot*master = clone_get_master(this);
    if (master) {
	st += master->get_name();
    }
}

void scopeProj::construct_prefix_name(genString& st) const
{
    Initialize(scopeProj::construct_prefix_name__genString&);
    scopeNode *prnt = get_parent();
    if(prnt)
      prnt->construct_prefix_name(st);

    return;
}

void scopeRoot::construct_prefix_name(genString& st) const
{
    Initialize(scopeNode::construct_prefix_name__genString&);
    st = get_name();
    return;
}

void scopeRoot::construct_name(genString& st) const
{
    st = get_name();
}
void scopeClone::construct_name(genString& st) const
{
    construct_prefix_name(st);
}

void scopeProj::construct_name(genString& st) const
{
    Initialize(scopeProj::construct_name__genString&);
    construct_prefix_name(st);
    projNode*proj = clone_get_proj(this);
    if(proj)
	st += proj->get_ln();

}

void scopeNode::send_string(ostream& os) const
{
    Initialize(scopeNode::send_string);
    genString st;
    construct_name (st);
    os << (char *)st;
}

extern void gen_print_indent(ostream&, int);

void scopeNode::print(ostream& os, int level) const 
{
    Initialize(scopeNode::print);
    gen_print_indent(os, level);
    send_string (os);
}

extern int proj_has_real_xref (projNode *pn);

scopeNode *scopeNode::extract_node (projNode *cloned_pn, int slash_flag)
{
    Initialize(scopeNode::extract_node__projNode*);
    scopeNode *sc = NULL;

    if (proj_has_real_xref(cloned_pn)) {
	Xref*Xr = cloned_pn->get_xref();
	if (slash_flag)
	    sc = new scopeSlashLeaf(cloned_pn, Xr);
	else
	    sc = new scopeLeaf(cloned_pn, Xr);
    } else {
        sc = new scopeProj(cloned_pn);
	cloned_pn->refresh();

	// boris, 102497. Extract projects in the PDF order
	symbolArr& proj_contents = cloned_pn->rtl_contents();
	symbolPtr sym;
	ForEachS(sym, proj_contents) {
	    if (sym.get_kind() != DD_PROJECT)
		continue;

	    appPtr app_head = sym.get_def_app();
	    if ( !(app_head && is_projHeader(app_head)) )
		continue;

	    projNode *child = projNodePtr(app_head->get_root());
	    scopeNode *sibl = NULL;
	    scopeNode *child_sc = extract_node(child);
	    if (sibl)
		sibl->put_after(child_sc);
	    else
		sc->put_first(child_sc);
	    
	    sibl = child_sc;
	}
    }
    return sc;
}

scopeNode *scopeNode::extract_node (scopeRoot *cloned)
{
    Initialize(scopeNode::extract_node(scopeRoot*));
    scopeClone *new_sc = NULL;
    if (cloned){
      new_sc = checked_cast(scopeClone,obj_tree_copy(cloned));
    }
    return new_sc;
}

void scopeNode::scopes_where_used (objArr& arr, objSet & os) const
{
     Initialize(scopeNode::scopes_where_used);
     scopeRoot *rt = checked_cast(scopeRoot,get_root());
     rt->scopes_where_used(arr,os);
}

void scopeClone::scopes_cloned (symbolArr& sarr) const
{
    Initialize(scopeClone::scopes_cloned);
    scopeRoot* master = clone_get_master(this);
    if (!master)
      return;

    sarr.insert_last(master);

    if (is_scopeUnit(master));
	return;

    for (scopeClone *cur = get_first() ; cur ; cur = cur->get_next()) {
	cur->scopes_cloned(sarr);
    }
}

void scopeRoot::scopes_cloned (symbolArr& sarr) const
{
    Initialize(scopeRoot::scopes_cloned);

    for (scopeClone *cur = (scopeClone*)get_first() ; cur ; cur = cur->get_next()) {
	cur->scopes_cloned(sarr);
    }
}
void scopeProj::scopes_cloned (symbolArr& sarr) const
{
   return;
}

void scopeNode::get_def_symbols (symbolPtr& sym, symbolArr& sarr, xrefSymbol *def_file)
{
    Initialize(scopeNode::get_def_symbols);
    get_symbols (sym, sarr, def_file, 1);
}

void scopeNode::lookup_all (symbolPtr& sym, symbolArr& sarr)
{
    Initialize(scopeNode::lookup_all);
    get_symbols (sym, sarr, (xrefSymbol *)0, 0);
}

void scopeNode::get_symbols (symbolPtr& sym, symbolArr& sarr, xrefSymbol *def_file, int is_def)
{
    Initialize(scopeNode::get_def_symbols);
    if (sym.isnull())
	return;

    for (scopeNode *cur = checked_cast(scopeNode,get_first()) ;
	 cur ;
	 cur = checked_cast(scopeNode,cur->get_next()) )
	{
	    cur->get_symbols(sym, sarr, def_file, is_def);
	}
}

symbolPtr scopeNode::get_def_symbol (ddKind knd, const char *nm, xrefSymbol *dfs)
{
    Initialize(scopeNode::get_def_symbols);
    symbolPtr retval = NULL_symbolPtr;
    objArr all_leaves;
    pmod_scopes_cloned (all_leaves);
    Obj *cur;
    ForEach(cur,all_leaves){
	scopeLeaf *lf = checked_cast(scopeLeaf,cur);
	retval = symbolPtr::get_def_symbol_for_leaf(lf, knd, nm, dfs);
	if (retval.isnotnull())
	    break;
    }
    return retval;
}

Relational *   scopeNode::rel_copy(void) const {return NULL;}
void   scopeNode::construct_prefix_name(genString&) const {}
void   scopeNode::construct_name(genString&) const {}
void   scopeNode::scopes_cloned(symbolArr&) const {}

void scopeNode::pmod_scopes_cloned (objArr& oarr) const
{
    Initialize(scopeNode::pmod_scopes_cloned);
    for (scopeNode *cur = checked_cast(scopeNode,get_first()) ; 
	 cur ; 
	 cur = checked_cast(scopeNode,cur->get_next()) )
	cur->pmod_scopes_cloned(oarr);
}

void scopeNode::get_pmod_modules (symbolArr& modules, symbolSet& mod, scopePmodQuery qmode)
{
    Initialize(scopeNode::get_pmod_modules);
    if(ParaCancel::is_cancelled())
	return;

    for (scopeNode *cur = checked_cast(scopeNode,get_first ())
	 ; cur ;
	 cur = checked_cast(scopeNode,cur->get_next()))
	{
	    cur->get_pmod_modules(modules, mod, qmode);
	}
}

objTree *scopeNode::get_root() const
{
    Initialize(scopeNode::get_root);
    scopeRoot *rt = checked_cast(scopeRoot,appTree::get_root());
    if ((scopeRoot *)scope_mgr.the_trash == rt) {
	const char *nm = get_name();
	symbolPtr sym;
	scopeMgr_find (nm, sym);
	scopeNode *nd = checked_cast(scopeNode,sym);
	if (nd && nd != this)
	    rt = checked_cast(scopeRoot,nd->get_root());
	else
	    rt = NULL;
    }
    return rt;
}
