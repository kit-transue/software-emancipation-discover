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
#include <msg.h>
#include <scopeMgr.h>
#include <genError.h>
#include <proj.h>
#include <scopeRoot.h>
#include <scopeClone.h>
#include <path.h>
#include <format.h>
#include <scopeClone.h>
#include <ParaCancel.h>
#include <scopeUnit.h>
#include <scopeMaster.h>
#include <scopeLeaf.h>
#include <mpb.h>

void proj_get_modules(projNode *proj, objArr& modules);

int Scope_DebugLevel = customize::getIntPref("DIS_internal.Scope_DebugLevel");

int scope_get_global_mode();

int scopeMgr_proj_is_special (projNode *proj)
{
    int ret = 0;
    if (proj) {
	char const *name = proj->get_name();
	if( (name[0] != '/' ) || !strcmp(name,"/ParaDOCS") || !strncmp(name,"/__", 3))
	    ret = 1;
    }
    return ret;
}

const char *projNode_get_right_name (projNode *pn)
{
    Initialize(projNode_get_right_name);
    if (!pn) return NULL;
    if (pn->fake_name.length() == 0) {
	genString phy;
	pn->ln_to_fn_imp(pn->get_ln(), phy, DIR_TYPE, 0, 1);
	pn->fn_to_ln(phy, pn->fake_name, DIR_TYPE, 0, 1);
    }

    return (const char *)pn->fake_name;
}

scopeMgr scope_mgr;
void scope_mgr_report_clone(scopeClone*clone)
{
  scope_mgr.clones.insert(clone);
}
scopeMgr::scopeMgr () :
    mode (PROJ_SCOPE_LESS), updated_ok (false), slash_unit(NULL)
{
    the_trash = new scopeTrash ();
}

scopeMgr::~scopeMgr ()
{
    cleanup();
}

void scopeMgr::update (boolean force)
{
    Initialize(scopeMgr::update);
    if (!force && updated_ok)
	return;

    update_units ();
    update_scopes ();
    updated_ok = true;
}

void scopeMgr::update_scopes ()
{
    Initialize(scopeMgr::update_scopes);
    delete_scopes();
}

void scopeMgr::delete_scopes ()
{
    Initialize(scopeMgr::delete_scopes);
    Obj *cur_sc;
    ForEach(cur_sc,scopes) {
	scopeRoot *sc = (scopeRoot *)cur_sc;
	delete (sc);
    }
    scopes.removeAll();
}

struct applyData {
  objArr*units;
  scopeUnit*punit;
};

static int lname_apply_callback(symbolArr&arr,int arr_index, int group_index,int group_size,char*value,Interpreter*in,void*data)
{
    Initialize(lname_apply_callback);
    applyData *ad = (applyData*)data;

    if(group_index == 0){
	ad->punit = new scopeUnit(value);
	ad->punit->updated = 1;
	ad->units->insert_last(ad->punit);

	// add slash unit project(s)
	
	if (scope_mgr.slash_unit) {
	    for (scopeClone *cl = checked_cast(scopeClone,scope_mgr.slash_unit->get_first_leaf()) ;
		 cl ;
		 cl = cl->get_next_leaf())
		{
		    if (is_scopeLeaf(cl)) {
			scopeLeaf *lf = (scopeLeaf *)cl;
			projNode *pn = clone_get_proj (lf);
			if (pn) {
			    ad->punit->add_project(pn, 1);
			}
		    }
		}
	}
    }

    projNode*pn = (projNode*)(Relational*)arr[arr_index];
    ad->punit->add_project(pn);
    return 0;
}

void symbolPtr_reset_last_scope();

void scopeMgr::cleanup()
{
    Initialize(scopeMgr::cleanup);
    clones.removeAll();
    symbolPtr_reset_last_scope();
    delete_scopes();
    delete_units();
    updated_ok = 0;
    mode = PROJ_SCOPE_LESS;
}


void scopeMgr::delete_units ()
{
    Initialize(scopeMgr::delete_units);
    Obj *cur_unit;
    ForEach(cur_unit,units) {
	scopeUnit *punit = (scopeUnit *)cur_unit;
	punit->cleanup();
	the_trash->add (punit);
    }
    units.removeAll();
    if (slash_unit) {
	slash_unit->cleanup();
	the_trash->add (slash_unit);
    }
    slash_unit = NULL;
}

void scopeMgr::update_units ()
{
    Initialize(scopeMgr::update_units);
    delete_units();
    symbolArr projs;
    projNode *pn;

    int break_val = 5, val = 1,incr_val = 1, num_incs = 0;
    mpb_incr_values_init(break_val, val, incr_val);

    for (int i=1; pn = projList::search_list->get_proj(i); i++) {
	const char *cur_nm = NULL;
	if (!scopeMgr_proj_is_special(pn) && (cur_nm = projNode_get_right_name(pn)) )
	    if (strcmp(cur_nm,"/") == 0) {
		if (!slash_unit) {
		    slash_unit = new scopeUnit("/");
		    slash_unit->updated = 1;
		}

		slash_unit->add_project(pn);
	    } else
		projs.insert_last(pn);

	mpb_step(break_val, incr_val, val, num_incs,i);
    }
    applyData ad;
    ad.units = &units;
    ad.punit = 0;
    api_sort_apply(projs, "lname", 0, lname_apply_callback, 0, &ad);
}

scopeRoot *scopeMgr::create (const char *lname, scopeRootKind knd)
{
    Initialize(scopeMgr::create);
    scopeRoot *found = find (lname);
    if (found)
	return NULL;

    switch (knd) {
    case SCOPE_SLL:
	found = new scopeSLL(lname);
	break;
    case SCOPE_DLL:
	found = new scopeDLL(lname);
	break;
    case SCOPE_EXE:
	found = new scopeEXE(lname);
	break;
    default:
	found = NULL;
    }

    if (found)
	scopes.insert_last(found);

    return found;
}

static scopeRoot *scopeMgr_find_by_name (const char *lname, objArr& ob_arr)
{
    Initialize(scopeMgr_find_by_name);
    scopeRoot *found = NULL;
    char *comp_name = (char *)lname;
    if (comp_name && comp_name[0] == '/')
	comp_name ++ ;
    
    if (comp_name == 0 || comp_name[0] == 0)
	return found;

    // Units
    Obj *cur;
    ForEach(cur,ob_arr) {
	scopeRoot *sh = checked_cast(scopeRoot,cur);
	char *cur_name = (char *)sh->lname;
	if (cur_name && cur_name[0] == '/')
	    cur_name ++  ;

	if (cur_name == 0 || cur_name[0] == 0)
	    continue;

	if (strcmp(comp_name, cur_name) == 0) {
	    found = sh;
	    break;
	}
    }
    return found;
}

scopeRoot *scopeMgr::find (const char *lname)
{
    Initialize(scopeMgr::find);
    scopeRoot *found = scopeMgr_find_by_name (lname, units);
    if (!found)
	found = scopeMgr_find_by_name (lname, scopes);
    return found;
}

extern "C" void scopeMgr_fill_units (symbolArr &sarr)
{
    Initialize(scopeMgr_fill_units__symbolArr&);
    scope_mgr.update();
    Obj *cur;
    ForEach(cur,scope_mgr.units){
	symbolPtr sym((Relational *)cur);
	sarr.insert_last (sym);
    }
}

extern "C" void scopeMgr_fill_scopes (symbolArr &sarr)
{
    Initialize(scopeMgr_fill_scopes__symbolArr&);
    scope_mgr.update();
    Obj *cur;
    ForEach(cur,scope_mgr.scopes){
	symbolPtr sym((Relational *)cur);
	sarr.insert_last (sym);
    }
}

extern "C" void scopeMgr_fill_root_scopes (symbolArr &sarr, int filter)
{
    Initialize(scopeMgr_fill_root_scopes__symbolArr&);
    scope_mgr.update();
    Obj *cur;
    ForEach(cur,scope_mgr.scopes){
	scopeRoot *sr = checked_cast(scopeRoot,cur);
	if (!master_get_clones(sr)) {
	    if (!filter 
		|| (filter == 1 && !sr->is_hidden())
		|| (filter == 2 && sr->is_hidden())) {
		symbolPtr sym((Relational *)cur);
		sarr.insert_last (sym);
	    }
	}
    }
}

extern "C" void scopeMgr_find (const char *lname, symbolPtr& sym)
{
     Initialize(scopeMgr_find);
     scope_mgr.update();
     Relational *found = scope_mgr.find (lname);
     if (!found)
	found = scope_mgr.clones.lookup(lname);

     sym = found;
}

extern "C" void scopeMgr_new_sll (const char *lname, symbolPtr& sym)
{
     Initialize(scopeMgr_new_sll);
     scope_mgr.update();
     scopeRoot *found = scope_mgr.create (lname,SCOPE_SLL);
     sym = found;
}

extern "C" void scopeMgr_new_dll (const char *lname, symbolPtr& sym)
{
     Initialize(scopeMgr_new_dll);
     scope_mgr.update();
     scopeRoot *found = scope_mgr.create (lname,SCOPE_DLL);
     sym = found;
}

extern "C" void scopeMgr_new_exe (const char *lname, symbolPtr& sym)
{
     Initialize(scopeMgr_new_exe);
     scope_mgr.update();
     scopeRoot *found = scope_mgr.create (lname,SCOPE_EXE);
     sym = found;
}

extern "C" void scopeMgr_get_scopes_of (Relational *obj, symbolArr& res_arr)
{
    Initialize(scopeMgr_get_scopes_of);
    objArr arr;
    scopeMgr::get_scopes_of(obj, arr);
    res_arr = arr;
}

int scopeMgr::get_scopes_of (Relational *obj, objArr& res_arr)
{
    Initialize(scopeMgr::get_scopes_of);
    if (!obj)
	return 0;

    scope_mgr.update();

    objSet os;
    if (is_projNode(obj)) {
	projNode *pn = (projNode *)obj;
	Obj *all_clones = proj_get_clones(pn);
	Obj *cur;
	ForEach(cur,*all_clones) {
	    scopeClone *sclone  = checked_cast(scopeClone,cur);
	    if (sclone->get_id() == SCOPE_TRASH) continue;
	    scopeRoot *scope    = checked_cast(scopeRoot,sclone->get_root());
	    scope->scopes_where_used(res_arr,os);
	}
    } else if (is_scopeNode(obj)) {
	scopeNode *t = (scopeNode *)obj;
	t->scopes_where_used(res_arr,os);
    }

    return res_arr.size();
}

extern "C" void scopeMgr_get_scopes_in (scopeRoot *scope, symbolArr& res_arr)
{
    Initialize(scopeMgr_get_scopes_in);
    if (scope)
	scope->scopes_cloned(res_arr);
}

extern "C" void scopeMgr_get_imported_in (scopeRoot *scope, symbolArr& res_arr)
{
    Initialize(scopeMgr_get_imported_in);
    if (scope)
	scope->scopes_imported(res_arr);
}

extern "C" void scopeMgr_get_exported_in (scopeRoot *scope, symbolArr& res_arr)
{
    Initialize(scopeMgr_get_imported_in);
    if (scope)
	scope->symbols_exported(res_arr);
}

bool objNameSet::isEqualObjects (const Object& ob1, const Object& ob2) const
{
  const char* nm1 = name(&ob1);
  const char* nm2 = name(&ob2);
  bool ret = (strcmp (nm1,nm2) == 0);
  return ret;
}

const char* objNameSet::name(const Object*oo) const
{
  return RelationalPtr(oo)->get_name();
}
Relational* objNameSet::lookup(const char*name) const
{
    Object*cur = 0;
    int idx = 0; 
    find(name, idx, cur); 
    return (Relational*) cur;
}

static int already_reported = 0;

static void report_cm_command_callback(void*)
{
    Initialize(report_cm_command_callback);
    Obj *cur;
    ForEach(cur,scope_mgr.units){
	scopeUnit *un = checked_cast(scopeUnit,cur);
	un->invalidate_hidden_symbols();
    }
    already_reported = 0;
}

// This function immediately sets "update_hidden" 
// for all the units.
void scopeMgr_report_cm_command ()
{
    report_cm_command_callback((void *)NULL);
}

void pset_send_own_callback (void (*func)(void *), void *dt);

// This function ping DISCOVER with the callback report_cm_command_callback
// that sends event to itself.
void scopeMgr::report_cm_command (CM_COMMAND, projNode *src_proj, projNode *dst_proj, const char *)
{
    Initialize(scopeMgr::report_cm_command);
    if (already_reported == 1)
	return;

    already_reported = 1;
    if (Scope_DebugLevel > 0) {
	const char *nm = NULL;
	if (src_proj)
	    nm = src_proj->get_name();
	else if (dst_proj)
	    nm = dst_proj->get_name();
	else
	    nm = "(NULL)";
	    
	msg("$1: for project $2\n") << _my_name << eoarg << nm << eom;
    }
    pset_send_own_callback(report_cm_command_callback,(void*)0);
}

void scopeMgr::invalidate_unit (projNode *pn)
{
    Initialize(scopeMgr::invalidate_unit);
    if (!pn) return;

    if (Scope_DebugLevel > 0) {
	msg("$1: for project \"$2\".\n") << _my_name << eoarg << pn->get_name() << eom;
    }

    Obj *cur;
    Obj *clone_set = proj_get_clones(pn);
    ForEach(cur,*clone_set){
	scopeProj *clone    = checked_cast(scopeProj,cur);
	if (clone->get_id() == SCOPE_TRASH) continue;
	if (clone->punit && clone->punit != slash_unit)
	    clone->punit->invalidate();
	break;
    }
}


// Returns real unit or slash_unit for the slash project
scopeUnit *scopeMgr::unit (projNode *pn)
{
    Initialize(scopeMgr::unit);
    scopeUnit *un = NULL;
    Obj *sc_projs;
    while (pn) {
	if ( (sc_projs = proj_get_clones(pn)) ) {
	    Obj *cur;
	    ForEach(cur,*sc_projs){
		scopeProj *sp = checked_cast(scopeProj,cur);
		if (is_scopeSlashLeaf(sp))
		    un = slash_unit;
		else
		    un = sp->punit;
		break;
	    }
	    break;
	}
	pn = childProject_get_parentProject(pn);
    }
    return un;
}

scopeUnit* scopeMgr::unit(symbolPtr& sym)
{
    Initialize(scopeMgr::unit);
    scopeUnit *un = NULL;
    if (sym.isnull())
	return un;

    if (sym.get_kind() == DD_PROJECT) {
	projNode *pn = checked_cast(projNode,sym);
	un = unit(pn);
	return un;
    }
	
    if (sym.get_kind() == DD_MODULE && sym.relationalp()) {
	projModule *mod = checked_cast(projModule,sym);
	projNode *pn = mod->get_project();
	un = unit(pn);
	return un;
    }

    Relational * sc = sym.get_scope();
    if (!sc)
	return un;

    if (is_scopeProj(sc)) {
	scopeProj *sp = (scopeProj *)sc;
	un          = sp->punit;
    } else if (is_Xref(sc)) {
	Xref * Xr    = (Xref*)sc;
	projNode *pn = Xr->get_projNode();
	if (pn) 
	    un = scope_mgr.unit (pn);
    }

    if ((!un || un == scope_mgr.slash_unit) && sym.get_has_def_file()) {
	xrefSymbol *xsym = sym.operator->();
	symbolArr as;
	Xref* Xr = sym.get_xref();
	int sz = xsym->get_local_link_internal (is_defined_in, as, Xr->get_lxref());
	if (sz) {
	    symbolPtr& def_file = as[0];
	    Obj *cur;
	    ForEach(cur,units){
		scopeUnit *cur_un = checked_cast(scopeUnit,cur);
		if ( !cur_un->is_hidden_symbol(def_file) ) {
		    un = cur_un;
		    break;
		}
	    }
	}
    }

    return un;
}

projModule *scopeMgr::find_visible_module  (const char *nm, scopeUnit *un)
{
    Initialize(find_visible_module);
    if (!nm || !nm[0]) return NULL;
    projModule *mod = NULL;
    if (un)
	mod = un->find_visible_module(nm);
    else {
	Obj *cur_unit;
	ForEach(cur_unit,units){
	    un = checked_cast(scopeUnit,cur_unit);
	    if ( (mod = un->find_visible_module (nm)) )
		break;
	}
    }
    return mod;
}

void get_uncloned_proj_modules (symbolSet& modset, symbolArr& modules, projNode *pn)
{
    Initialize(get_uncloned_proj_modules);
    if (!pn || scopeMgr_proj_is_special(pn)) return;
    
    const char *cur_nm = projNode_get_right_name(pn);
    if (cur_nm && strcmp(cur_nm,"/") == 0) {
	symbolArr& proj_contents = pn->rtl_contents();
	symbolPtr p_sym;
	ForEachS(p_sym, proj_contents) {
	    if (p_sym.get_kind() == DD_PROJECT) {
		projNode *pn = checked_cast(projNode,p_sym);
		cur_nm =  projNode_get_right_name(pn);
		scopeRoot *found = scopeMgr_find_by_name (cur_nm, scope_mgr.units);
		if (found == 0) {
		    objArr arr;
		    proj_get_modules (pn, arr);
		    Obj *m;
		    ForEach(m,arr) {
			symbolPtr sym = checked_cast(projModule,m);
			if (!modset.includes(sym)) {
			    modset.insert(sym);
			    modules.insert_last(sym);
			}
		    }
		}
	    }
	}
    }
}

// projNode is above Xref
void scopeMgr::get_pmod_modules (symbolSet& modset, symbolArr& modules, projNode *pn, scopePmodQuery qmode)
{
    Initialize(scopeMgr::get_pmod_modules__symbolSet&_symbolArr&_projNode*_scopePmodQuery);
    if (!pn) return;
    if (qmode == PMOD_ALL) {
	xref_get_all_pmod_modules (modset, modules, pn);
	return;
    }

    scopeProj *sp = NULL;
    // Update scopeMgr and the project unit and after this look for the scopeProj
    scope_mgr.update();

    Obj *pr_clones = proj_get_clones (pn);
    Obj *cur_clone;
    int found = 0;
    ForEach(cur_clone,*pr_clones){
	scopeProj *sp = checked_cast(scopeProj,cur_clone);
	if (sp->punit == slash_unit || !clones.includes(*sp))
	    continue;

	scopeRoot *rt = checked_cast(scopeRoot,sp->get_root());
	if (is_scopeUnit(rt)) {
	    sp->punit->update();
	    sp->get_pmod_modules (modules, modset, qmode);
	    found = 1;
	}
    }

    if (found == 0)
	xref_get_all_pmod_modules (modset, modules, pn);
    else 
	get_uncloned_proj_modules (modset, modules, pn);
    return;
}

// projNode is above Xref
void scopeMgr::get_pmod_modules (symbolSet& modset, symbolArr& modules, scopeNode *nd, scopePmodQuery qmode)
{
    Initialize(scopeMgr::get_pmod_modules__symbolSet&_symbolArr&_scopeNode*_scopePmodQury);
    if (!nd) return;

    // save the project and scopeNode name to restore it 
    genString copy_nm = nd->get_name(); 
    scope_mgr.update();

    symbolPtr sym;
    scopeMgr_find (copy_nm, sym);
    nd = checked_cast(scopeNode,sym);

    if (!nd) return;
    
    // Find parent unit to update if the project is under scopeUnit
    if (is_scopeProj(nd)) {
	scopeProj *sproj = (scopeProj *)nd;
	sproj->punit->update();
    } 
    nd->get_pmod_modules (modules, modset, qmode);
}

void xref_get_all_pmod_modules (symbolSet& mod, symbolArr& modules, projNode *pn, scopeNode *sc, int nomodify)
{
    Initialize(xref_get_all_pmod_modules);
    symbolArr  symbols;
    ddSelector selector;
    
    Xref *xr = pn->get_xref(1);
    if(xr && xr->get_lxref() && !xr->is_unreal_file()){
	selector.add(DD_MODULE);
	pn->get_xref(1)->add_symbols(symbols, selector);
	for(int i = symbols.size() - 1; i >= 0; i--){
	    symbolPtr & sym = symbols[i];
	    if(ParaCancel::is_cancelled())
		return;

	    if(!mod.includes(sym)){
		if (!nomodify) mod.insert_last(sym);
		if (sc) sym.put_scope(sc);
		modules.insert_last(sym);
	    }
	}
    }else {
	pn->refresh_projects();

	//boris 241097, Process projects in the PDF order
	symbolArr& proj_contents = pn->rtl_contents();
	symbolPtr sym;
	ForEachS(sym, proj_contents) {
	    if (sym.get_kind() != DD_PROJECT)
		continue;
	    
	    appPtr app_head = sym.get_def_app();
	    if( !(app_head && is_projHeader(app_head)) )
		continue;

	    if (ParaCancel::is_cancelled())
		return;
	    
	    projNode *pr = projNodePtr(app_head->get_root());
	    xref_get_all_pmod_modules(mod, modules, pr, sc);
	}
    }
}

void scopeMgr_get_proj_contents (projNode *pn, symbolArr& sarr)
{
    Initialize(scopeMgr_get_proj_contents);
    if (!pn) return;
    symbolArr& tmp = RTLNodePtr(pn)->rtl_contents();
    symbolPtr sym;
    scopeUnit *un = NULL;
    ForEachS(sym,tmp){
	ddKind knd = sym.get_kind();
	if (knd == DD_MODULE) {
	    if (!un && !(un = scope_mgr.unit (pn)))
		un = scope_mgr.slash_unit;
		    
	    if (un == NULL || un == scope_mgr.slash_unit)
		sarr.insert_last(sym);
	    else {
		char const *nm = sym.get_name();
		projModule *found = un->find_visible_module (nm);
		if (found)
		    sarr.insert_last(found);
	    }
	} else
	    sarr.insert_last(sym);
    }
}


