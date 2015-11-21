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
#include <msg.h>
#include <scopeUnit.h>
#include <scopeMgr.h>
#include <scopeClone.h>
#include <scopeLeaf.h>
#include <xrefSymbolSet.h>

init_relational(scopeUnit,scopeRoot);

extern void proj_get_modules(projNode *proj, objArr& modules, int depth); 

void scopeUnit::lookup_all (symbolPtr& sym, symbolArr& sarr)
{
    Initialize(scopeUnit::lookup_all);
    get_symbols (sym, sarr, (xrefSymbol *)0, 0);
}

void scopeUnit::invalidate () 
{
    Initialize(scopeUnit::invalidate);
    if (Scope_DebugLevel > 0) msg("$1: unit $2\n") << _my_name << eoarg << (char *)lname << eom;
    updated = 0; updated_hidden = false;
}

scopeUnit::scopeUnit (const char *right_name) : updated_hidden(false), hidden_syms(NULL)
{
    Initialize(scopeUnit::scopeUnit);
    internal_create (right_name, SCOPE_UNIT);
}

void scopeUnit::update (boolean force)
{
    Initialize(scopeUnit::update);
    if (force)
	updated = 0;

    if (updated == 0) {
	if (Scope_DebugLevel > 0) msg("$1: unit $2\n") << _my_name << eoarg << (char *)lname << eom;
	regenerate ();
	updated = 1;
    }
    update_hidden_symbols (force);
}

void scopeUnit::regenerate ()
{
    Initialize(scopeUnit::regenerate);
    if (Scope_DebugLevel > 0) msg("$1: unit $2\n") << _my_name << eoarg << (char *)lname << eom;
    cleanup();
    objArr projs;
    scopeClone *last_child = NULL;
    for (scopeClone *cl = get_first() ; cl ; cl = cl->get_next()) {
	if (is_scopeProj(cl)) {
	    scopeProj *sp = (scopeProj *)cl;
	    projNode *pn = clone_get_proj(sp);
	    if (pn)
		projs.insert_last(pn);
	}
	last_child = cl;
    }

    while (last_child) {
	scopeClone *cur = last_child->get_prev();
	last_child->remove_from_tree();
	scope_mgr.the_trash->add(last_child);
	last_child = cur;
    }

    Obj *cur;
    ForEach(cur,projs) {
	projNode *pn = checked_cast(projNode,cur);
	scopeNode *sn = add_project(pn);
    }
}

void scopeUnit::cleanup()
{
    Initialize(scopeUnit::cleanup);
    if (hidden_syms)  {
      delete hidden_syms;
      hidden_syms = 0;
    }
    visible_mod_hash.removeAll();
}

static boolean accept_ln (const char *ln, objArr& leaves)
{
    Initialize(accept_ln);
    boolean res = false;
    if (ln && ln[0]) {
	Obj *cur;
	ForEach(cur,leaves){
	    Relational * rel = (Relational *) cur;
	    projNode * pn = NULL;
	    if (is_scopeSlashLeaf(rel))
		continue;
	    else if (is_projNode(rel))
		pn = checked_cast(projNode,cur);
	    else {
		scopeLeaf * leaf = checked_cast(scopeLeaf,cur);
		pn = clone_get_proj(leaf);
	    }
	    genString fn;
	    pn->ln_to_fn (ln, fn);
	    if (fn.length()) {
		res = true;
		break;
	    }
	}
    }
    return res;
}

void scopeUnit::hide_symbols (projModule *mod)
{
    Initialize(scopeUnit::hide_symbols);
    if (!mod) return;
    symbolPtr mod_sym(mod);
    if (mod_sym.isnotnull()) {
	symbolArr nsa;
	mod_sym.get_local_link(is_defining,nsa);
	symbolPtr s;
	ForEachS(s,nsa) {
	    hidden_syms->insert(s.operator->());
	}
    }
}

void scopeUnit::update_hidden_in_projs ()
{
    Initialize(scopeUnit::update_hidden_in_projs);
    objArr all_leaves;

    pmod_scopes_cloned (all_leaves);

    Obj *cur;
    ForEach(cur,all_leaves){
	scopeLeaf *leaf = checked_cast(scopeLeaf,cur);
	projNode *pn = clone_get_proj(leaf);
	if (!pn) continue;
	
	objArr modules; 
	
	proj_get_modules (pn, modules, -1);
	Obj *cur_mod;
	ForEach(cur_mod,modules) {
	    projModule *mod = checked_cast(projModule,cur_mod);
	    genString fn;
	    if (visible_mod_hash.includes(*mod))
		hide_symbols(mod);
	    else if (is_scopeSlashLeaf(leaf) && !accept_ln(mod->get_name(),all_leaves))
		hide_symbols(mod);
	    else
		visible_mod_hash.add(*mod);
	}
    }
}

void scopeUnit::update_hidden_symbols (boolean force)
{
    Initialize(scopeUnit::update_hidden_symbols);
    if (force)
	updated_hidden = false;

    if (updated_hidden) return;
    cleanup ();
    hidden_syms = new xrefSymbolSet;
    update_hidden_in_projs ();
    updated_hidden = true;
}

boolean scopeUnit::is_hidden_symbol (symbolPtr& sym)
{
    Initialize(scopeUnit::is_hidden_symbol);
    boolean res = false;
    if (sym.isnotnull() && sym.is_xrefSymbol()) {
	update();
	res = (hidden_syms) ? hidden_syms->includes(sym.operator->()) : false;
    }
    return res;
}

scopeUnit::~scopeUnit()
{
    cleanup();
}

scopeNode *scopeUnit::add_component_scope (scopeRoot *, ostream& os)
{
    Initialize(scopeUnit::add_component_scope);
    os << "Error: cannot add scope to the unit \"" << (char *)lname << "\"." << endl;
    return (scopeNode *)NULL;
}

scopeNode *scopeUnit::add_project (projNode *pn, int slash_flag)
{
    Initialize(scopeUnit::add_project__projNode*);
    scopeNode *t = add_component_ipu (pn, this, slash_flag);
    return t;
}

scopeNode *scopeUnit::add_component_project (projNode *pn, ostream& )
{
    Initialize(scopeUnit::add_component_project__projNode*);
    scopeNode *t = add_component_ipu (pn, this);
    return t;
}

void scopeUnit::get_modules (objArr& modules, int /* depth */)
{
    Initialize(scopeUnit::get_modules);
    update();
    DO(visible_mod_hash,Object,cur)
	modules.insert_last((Obj *)cur);
    OD
}

void scopeUnit::scopes_cloned (symbolArr& sarr) const
{
    Initialize(scopeUnit::scopes_cloned);
    symbolPtr sym = (Relational *)this;
    sarr.insert_last(sym);
    return;
}

const char *uniqNameHash::name (const Object *ob) const
{
    Relational *rel = (Relational *)ob;
    const char *nm = NULL;
    if (rel) nm = rel->get_name();
    return nm;
}

bool uniqNameHash::isEqualObjects(const Object& ob1, const Object& ob2) const
{
    bool res = false;
    Relational *rel1 = (Relational *)&ob1;
    Relational *rel2 = (Relational *)&ob2;
    if (rel1 && rel2) {
	const char *nm1= rel1->get_name();
	const char *nm2= rel2->get_name();
	res = (nm1 && nm2 && (strcmp(nm1, nm2) == 0)) ? true : false;
    }
    return res;
}

int xrefSymbolSet::includes(const xrefSymbol* xr) const
{
  Object*oo = findObjectWithKey(*((Object*)xr));
  return (oo != 0);
}
unsigned int xrefSymbolSet::hash_element(const Object* obj) const
{
  return (unsigned int) obj;
}
bool xrefSymbolSet::isEqualObjects(const Object&o1, const Object&o2) const
{
  return ((void*)&o1) == ((void*)&o2);
}


void scopeUnit::get_pmod_modules (symbolArr& modules, symbolSet& modset, scopePmodQuery qmode)
{
    Initialize(scopeUnit::get_pmod_modules);
    update();
    for (scopeClone *cur = get_first() ; cur ; cur = cur->get_next())
	cur->get_pmod_modules (modules, modset, qmode);
}

projModule *scopeUnit::find_visible_module (const char *ln)
{
    Initialize(scopeUnit::find_visible_module);
    projModule *mod = NULL;
    if (!ln || !ln[0]) return mod;
    update();

    Object *cur = 0;
    int idx     = 0;

    visible_mod_hash.find (ln, idx, cur);

    mod = checked_cast (projModule,cur);
    return mod;
}
