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
#include <scopeRoot.h>
#include <scopeUnit.h>
#include <scopeMaster.h>
#include <scopeMgr.h>
#include <symbolArr.h>
#include <scopeNode.h>
#include <xrefSymbolSet.h>
#include <scopeClone.h>
#include <scopeLeaf.h>
#include <xrefSymbolSet.h>
#include <symbolSet.h>
#include <XrefTable.h>

init_relational(scopeRoot,scopeNode);
init_relational(scopeTrash,scopeRoot);

void scopeRoot::invalidate () 
{
    Initialize(scopeRoot::invalidate);
    if (Scope_DebugLevel > 0) msg("$1: unit $2\n") << _my_name << eoarg << (char *)lname << eom;
    updated = 0;
}

Relational *scopeRoot::rel_copy () const
{
    scopeClone *sc = new scopeClone(this);
    return sc;
}

void scopeRoot::update (boolean force)
{
    Initialize(scopeRoot::update);
    if (force) {
	if (Scope_DebugLevel > 0) msg("$1: unit $2\n") << _my_name << eoarg << (char *)lname << eom;
	updated = true;
    }
}


char const *scopeRoot::get_name () const
{
    Initialize(scopeRoot::get_name);
    return lname;
}

void scopeRoot::symbols_exported (symbolArr&) const
{
    return;
}

scopeRoot *scopeRoot::promote_to_scope (projNode *pn, ostream& os) const
{
    Initialize(scopeRoot::promote_to_scope);
    scopeRoot *sh = NULL;
    // only root project that can be promoted to Unit is accepted
    if (pn->get_parent()) {
	os << "Warning: component \"" << pn->get_name() << "\" is not root project. Ignored." << endl;
    } else {
	Obj *nodes = proj_get_clones(pn);
	Obj *cur1;
	ForEach(cur1,*nodes) {
	    scopeClone *t = checked_cast(scopeClone,cur1);
	    if (t->get_id() == SCOPE_TRASH) continue;
	    scopeRoot *cur_sh = checked_cast(scopeRoot,t->get_root());
	    if (is_scopeUnit(cur_sh)) {
		sh = cur_sh;
		os << "Message: scope unit is used instead of root project \"" << pn->get_name() << "\"." << endl;
		break;
	    }
	}
    }
    return sh;
}

scopeNode *scopeRoot::add_component_project (projNode *pn, ostream& os)
{
    Initialize(*scopeRoot::add_component);
    scopeNode *t = NULL;
    if (pn) {
	scopeRoot *sh = promote_to_scope (pn, os);
	if (sh == NULL)
	    os << "Warning: project \"" << pn->get_name() << "\" cannot be promoted to unit. Ignored." << endl;
	else
	    t = add_component_scope(sh, os);
    }
    return t;
}

boolean scopeRoot::add_import (scopeDLL *, ostream& os)
{
    Initialize(scopeRoot::add_import);
    os << "Error: dll can be imported into either dll or exe." << endl;
    return false;
}

scopeNode *scopeRoot::add_component_scope (scopeRoot *component, ostream& os)
{
    Initialize(scopeRoot::add_component_scope);
    if (!component) {
	os << "Warning: (null) component." << endl;
	return NULL;
    }

    if (is_scopeDLL(component)) {
	os << "Warning: DLL \"" << component->get_name() << "\" cannot be a component. Ignored." << endl;
	return NULL;
    }

    if (is_scopeEXE(component)) {
	os << "Warning: executable \"" << component->get_name() << "\" cannot be a component. Ignored." << endl;
	return NULL;
    }

    if (component == this) {
	os << "Warning: component \"" << (char *)lname << "\" cannot be added to itself. Ignored." << endl;
	return NULL;
    }

    scopeNode *t = NULL;
    if (contains_component(component)) {
	os << "Warning: scope \"" << (char *)lname << "\" already contains \"" << component->get_name() << ". Ignored." << endl;
	return NULL;
    }

    if (component->contains_component(this)) {
	os << "Warning: component \"" << component->get_name() << "\" already contains scope \"" << (char *)lname <<". Ignored." << endl;
	return NULL;
    }
    
    symbolArr cloned;
    scopes_cloned (cloned);
    symbolPtr cur_sym;
    ForEachS(cur_sym,cloned) {
	Relational *child = cur_sym;
	if (is_scopeRoot(child) && component->contains_component((scopeRoot *)child)) {
	    os << "Error: component \"" << component->get_name() << "\" and scope \"" << (char *)lname 
	       << "\" contain child \"" << child->get_name() << endl;
	    return NULL;
	}
    }

    t = add_component_internal (component);
    return t;
}

scopeClone *scopeRoot::get_first() const
{
  return (scopeClone*) objTree::get_first();
}

void scopeRoot::put_last(scopeClone*t)
{
  scopeClone *prev = NULL;
  for ( scopeClone *cur = get_first () ; cur ; cur = cur->get_next() )
    prev = cur;

  if (prev)
    prev->put_after (t);
  else
    put_first (t);

  t->set_root(this, this->get_name());
}
scopeNode *scopeRoot::add_component_ipu(projNode *proj, scopeUnit*su, int slash_flag)
{
  scopeProj*sp = (scopeProj*) scopeNode::extract_node (proj, slash_flag);
  if(sp){
    put_last(sp);
    sp->set_unit(su);
  }
  return sp;
}

scopeNode *scopeRoot::add_component_internal (scopeRoot* master)
{
  scopeClone*    t = (scopeClone*)scopeNode::extract_node (master);

  if (t){
    put_last(t);
  }
  return t;
}

// Check whether scope already contains the component
boolean scopeRoot::contains_component (scopeRoot *component) const
{
    Initialize(scopeRoot::contains_component);
    if (component == NULL)
	return false;

    boolean contains = false;

    Obj *nodes = master_get_clones(component);
    Obj *cur;
    ForEach(cur,*nodes){
	scopeClone *t = checked_cast(scopeClone,cur);
	scopeRoot *scope = checked_cast(scopeRoot,t->get_root());
	if (scope == this) {
	    contains = 1;
	    break;
	}
    }
    return contains;
}

extern int scope_get_global_mode();

void scopeNode::get_contents (symbolArr& sarr)
{
    Initialize(scopeNode::get_contents__symbolArr&);
    int mode = scope_get_global_mode();
    appTree* comp;
    for ( scopeClone *cur = (scopeClone*)get_first(); cur; cur = cur->get_next()) {
	if(/* mode == 2 || mg 7/12/99 */ is_scopeSlashLeaf(cur))
	    comp = cur;
	else {
	    if(is_scopeProj(cur))
		comp = clone_get_proj(scopeProjPtr(cur));
	    else
		comp = clone_get_master(cur);
	}
	if (comp) sarr.insert_last(comp);
    }
}

int scopeRoot::construct_symbolPtr(symbolPtr& sym) const
{
    sym = (Relational *)this;
    return 0;
}

ddKind scopeRoot::get_symbolPtr_kind() const
{
    return DD_SCOPE;
}


int scopeRoot::is_symbolPtr_def () const
{
    return 1;
}


app *scopeRoot::get_symbolPtr_def_app ()
{
    return NULL;
}

void scope_get_modules(scopeNode *sc, objArr& modules, int depth)
{
    Initialize(scope_get_modules);
    sc->get_modules (modules, depth);
}

extern void proj_get_modules(projNode *proj, objArr& modules, int depth); 

void scopeRoot::get_modules (objArr& modules, int depth)
{
    Initialize(scopeRoot::get_modules);
    for ( scopeClone *cur = get_first () ; cur ; cur = cur->get_next()) {
      	cur->get_modules (modules, depth);
    }
}

void scopeRoot::internal_create (const char *right_name, scopeRootKind knd)
{
    Initialize(scopeRoot::internal_create);
    if (right_name[0] != '/')
	lname.printf("%c%s",'/',right_name);
    else
	lname = right_name;

    scope_kind = knd;
}

void scopeRoot::scopes_where_used (objArr& arr, objSet & os) const
{
     Initialize(scopeRoot::scopes_where_used);
     if (!os.includes(this)) {
	 Obj *all_clones = master_get_clones(this);
	 int has_clones = 0;
	 Obj *cur;
	 if (all_clones)
	     ForEach(cur,*all_clones){
		 scopeRoot *scope = checked_cast(scopeRoot,(checked_cast(objTree,cur))->get_root());
		 scope->scopes_where_used(arr,os);
		 has_clones++;
	     }
	 if (!has_clones) {
	     os.insert((Obj*)this);
	     arr.insert_last(this);
	 }
     }
     return;
}

void scopeRoot::scopes_imported (symbolArr&) const
{
    return;
}

boolean scopeTrash::contains (scopeNode *rt) const
{
    Initialize(scopeTrash::contains);
    boolean ret = false;
    if (rt && rt->get_id() == SCOPE_TRASH)
	ret = true;

    return ret;
}

void scopeTrash::add (scopeNode *nd)
{
    Initialize(scopeTrash::add);
    if (!nd || contains(nd)) return;

    if (Scope_DebugLevel > 0) {
	msg("$1: adding $2\n") << _my_name << eoarg << nd->get_name() << eom; 
    }

    if (current_last)
	current_last->put_after (nd);
    else
	put_first (nd);

    current_last = nd;
    nd->put_id(SCOPE_TRASH);
    if (scope_mgr.clones.size()) {
	if (is_scopeClone(nd))
	    scope_mgr.clones.remove(*nd);

	for (scopeClone *cln = checked_cast(scopeClone,nd->get_first()) ; cln ; cln = cln->get_next()) {
	    cln->put_id(SCOPE_TRASH);
	    scope_mgr.clones.remove(*cln);
	}
    }
}

scopeTrash::scopeTrash (const char *nm) : current_last(0)
{
    scope_kind = SCOPE_COMPONENT;

    if (nm && nm[0])
	lname = nm;
    else
	lname = "/trash";
}

scopeTrash::~scopeTrash ()
{
    Initialize(scopeTrash::~scopeTrash);
    //cout << _my_name << ": should not be called" << endl;
}

objTree *  scopeTrash::get_root() const
{
    return (objTree *)this;
}
