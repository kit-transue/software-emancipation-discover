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
#include <scopeClone.h>
#include <cLibraryFunctions.h>
#include <ParaCancel.h>
#include <scopeMgr.h>
#include <scopeUnit.h>

init_relational(scopeClone,scopeNode);
init_relational(scopeProj,scopeClone);

init_rel_or_ptr(scopeClone,clone,0,scopeRoot,master,0);
init_rel_or_ptr(scopeClone,clone,0,projNode,proj,0);

extern void proj_get_modules(projNode *proj, objArr& modules, int depth); 
extern int scope_get_global_mode();

scopeClone::scopeClone(const scopeClone&other)
{
   proot  = other.proot;
   scopeRoot*master = clone_get_master(&other);
   clone_put_master(this,master);
}

scopeClone::scopeClone(const scopeRoot *master)
{
   proot  = NULL;
   clone_put_master(this, (scopeRoot*)master);
}


char const *scopeClone::get_name() const {return lname;}

int scopeClone::construct_symbolPtr(symbolPtr& sym) const
{
    sym = (Relational *)this;
    return 0;
}

ddKind scopeClone::get_symbolPtr_kind() const
{
    return DD_SCOPE;
}


int scopeClone::is_symbolPtr_def () const
{
    return 1;
}


app *scopeClone::get_symbolPtr_def_app ()
{
    return NULL;
}

void scopeClone::print(ostream& os, int lev) const
{
//  send_string(os);
  os << get_name();
  os << " root " << proot;
}
void scope_mgr_report_clone(scopeClone*clone);
void scopeClone::set_root(scopeRoot *rp, char const *pname)
{
  Initialize(scopeClone::set_root);
  proot = rp;
  appTree* master = clone_get_master(this);
  lname = pname;
  char const *mname = master->get_name();
  lname += mname;
  pname = lname;
  scope_mgr_report_clone(this);
  for(scopeClone*cur=get_first(); cur; cur=cur->get_next()){
      cur->set_root(rp, pname);
  }
}

void scopeProj::set_root(scopeRoot *rp, char const *pname)
{
  Initialize(scopeClone::set_root);
  proot = rp;
  appTree* master = clone_get_proj(this);
  lname = pname;
  char const *mname = master->get_name();
  lname += mname;
  pname = lname;
  scope_mgr_report_clone(this);
  for(scopeClone*cur=get_first(); cur; cur=cur->get_next()){
      cur->set_root(rp, pname);
  }
}

implement_copy_member(scopeClone);

void scopeClone::get_pmod_modules (symbolArr& modules, symbolSet& modset, scopePmodQuery qmode)
{
    Initialize(scopeClone::get_pmod_modules);
    scopeRoot *rt = clone_get_master(this);
    if (rt && is_scopeUnit(rt)) {
	scopeUnit *un = (scopeUnit *)rt;
	un->update();
    }

    for (scopeClone *cur = get_first () ; cur ; cur = cur->get_next())
	cur->get_pmod_modules (modules, modset, qmode);
}

scopeProj::scopeProj(const projNode *proj)
{
   clone_put_proj(this,(projNode*)proj);
}

scopeProj::scopeProj(const scopeProj&other) : punit(other.punit){
  projNode*proj = clone_get_proj((scopeProj*)&other);
  clone_put_proj(this,proj);
}

void scopeProj::set_unit(scopeUnit *up)
{
  punit = up;
  for(scopeProj*cur=get_first(); cur; cur=cur->get_next()){
      cur->set_unit(up);
  }
}

void scopeClone::get_modules (objArr& modules, int depth)
{
    Initialize(scopeRoot::get_modules);
    scopeRoot*master = clone_get_master(this);
    master->get_modules(modules, depth);
}

void scopeProj::get_modules (objArr& modules, int depth)
{
    Initialize(scopeRoot::get_modules);
    projNode*proj = clone_get_proj(this);
    objArr tmp;
    proj_get_modules(proj, tmp, depth);
    Obj *cur;

    ForEach(cur,tmp){
	projModule *mod = checked_cast(projModule,cur);
	projModule *found = punit->find_visible_module (mod->get_name());
	if (found)
	    modules.insert_last(found);
    }
}

implement_copy_member(scopeProj);
