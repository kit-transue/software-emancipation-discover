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
/*
  File: xref_dd.C
  
  misc utilities for handling hooks function

  functions of form:
  
   handle_FOO(.....): will be invoked when relation FOO
   is added, removed or modified.
  
   Example relations covered this way are
    ref_smt_of_dd, def_smt_of_dd, superclassof
   
*/
#include "RTL_externs.h"
#include "cLibraryFunctions.h"
#include "ddict.h"
#include "genError.h"
#include "groupHdr.h"
#include "groupTree.h"
#include "machdep.h"
#include "transaction.h"
#include "xref.h"
#include "xrefPair.h"
#include "xref_priv.h"

extern linkType get_linkType(RelType *);
ddElement* xrefSymbol_get_def_dd(symbolPtr that);
const int  customize_get_write_flag();

extern int start_call_rel_hooks;
extern hashPair * mhp;
extern int xref_hook_turn_off;

ddElement* dd_lookup_or_create(char const *ns, appTree* node, int xref_type, 
	               ddKind k, RelType* rel, char const *as_name)
{
  Initialize(dd_lookup_or_create);
  app* h = (is_appTree(node))? node->get_main_header(): checked_cast(app, node);
  ddRoot* dr = dd_sh_get_dr(h);
  ddElement* dd = dr->lookup(k, ns);
  if (dd == 0)
    dd = new ddElement(ns, node, xref_type, k, rel, as_name);
  else
    if (rel)
      put_relation(rel, node, dd);
  return dd;
}
ddElement* dd_lookup_or_create_many_to_one(char const *ns, appTree* node, int xref_type, 
	               ddKind k, RelType* rel, char const *as_name)
{
  Initialize(dd_lookup_or_create);
  app* h = (is_appTree(node))? node->get_main_header(): checked_cast(app, node);
  ddRoot* dr = dd_sh_get_dr(h);
  ddElement* dd = dr->lookup(k, ns);
  if (dd == 0)
    dd = new ddElement(ns, node, xref_type, k, rel, as_name);
  else {
    appTree* trg = checked_cast(appTree, get_relation(rel, node));
    if (trg == 0) 
      put_relation(rel, node, dd);
  }
  return dd;
}

// get symbol for file that contain this appTree *
symbolPtr get_file_sym(appTree * assoc_dd)
{
  Initialize(get_file_sym);
  if (assoc_dd == 0) return NULL_symbolPtr;
  app * ah = assoc_dd->get_main_header();
  ddElement * dd_f = (ah)? checked_cast(ddElement, get_relation(dd_of_appHeader, ah)) : 0;
  symbolPtr file_sym ;
  if (dd_f) file_sym = dd_f->get_xrefSymbol();
  return file_sym;
}

static bool has_more_rel (Relational* user, Relational* used, Obj* smt)
{
    Obj* first  = get_relation(used_of_user,user);
    Obj* second = get_relation(ref_smt_of_dd,used);
    Obj* el;
    if (!first || !second)
	return 0;
    ForEach(el,*first) {
	if (el == smt)
	    continue;
	Obj* el1;
	ForEach(el1,*second)
	    if (el1 == el)
		return 1;
    }
    return 0;
}

symbolPtr Xref_insert_symbol(ddSymbol* dd, app* h)
{
    return symbolPtr(Xref_insert_symbol_priv(dd,h));
}

// private version
Xref* app_get_xref(app*ah);
xrefSymbol* Xref_insert_symbol_priv(ddSymbol* dd, app* h)
{
  Initialize(Xref_insert_symbol);
  char const *name = dd->get_ddname();
  if ((name == 0) || (*name == 0)) return 0;
  Xref* xr;
  projNode* cpr = projNode::get_control_project();
  Xref* cxr = cpr? cpr->get_xref():0;
  if (is_projHeader(h))
    xr = cxr;
  else {
    xr = h ? app_get_xref(h) : 0;
    if (xr == 0)
      xr = get_XREF();
  }
  if (xr == 0) return 0;
  XrefTable* xrt = xr->get_lxref();
  if (xrt == 0) {
    xr->init(WRITE_ONLY);
    xrt = xr->get_lxref();
    IF (xrt == 0) return 0;
  }

  if (xr->get_perm_flag() == READ_ONLY) return 0;
  if ((cxr == xr) && dd->get_is_def())
    dd->set_def_file_with_good_name(h->get_filename());
  
  if (cxr == xr) /* extend control projects, just in case */
    cxr->make_room(500000);
  
  xrefSymbol* file_sym = xrt->insert_symbol(h);
  xrefSymbol* sym = xrt->insert_symbol(dd);

  if (sym && file_sym) {
    if (dd->get_is_def() || (cxr == xr)) {
      if (sym != file_sym)
	 file_sym->add_link(is_defining, sym, get_remove_hashPair(), 1);
    } else {
      if (!(h && is_smtHeader(h) && (sym->get_kind() == DD_MODULE)))
	 file_sym->add_link(file_ref, sym, get_remove_hashPair(), 1);
    }
  }

  return sym;
}

ddElementPtr new_dd_find_definition (ddKind kind, char const *name)
{
  symbolPtr sym = lookup_xrefSymbol(kind, name);
  return sym.get_def_dd();
}

// lookup Xref symbol, and self-insert if not found
void ddElement::set_xrefSymbol(xrefSymbol* sy)
{
  Initialize(ddElement::set_xrefSymbol);
  if (sy == 0) return;
  dd_sym = sy;
}

symbolArr* ddElement::get_files()
{
  Initialize(ddElement::get_files);
  symbolArr* as = new symbolArr();
  symbolPtr sym = get_xrefSymbol();
  sym.get_files(*as);
  return as;
}

symbolPtr ddElement::get_def_file_from_proj(projNode * pr)
{
    Initialize(ddElement::get_def_file_from_proj__projNode*);
    symbolPtr ret_sym = NULL_symbolPtr;
    // Had to cast NULL_symbolPtr to satisfy SGI compiler
    // [03/17/97, abover]:
    symbolPtr sym = (pr) ? pr->lookup_symbol (get_kind(), get_ddname()) : (symbolPtr)NULL_symbolPtr;
    if (sym.isnotnull()) {
	xrefSymbol *xsym = sym;
	if (xsym)
	    ret_sym = sym->get_def_file();
    }

    return ret_sym;
}

// 2 pass thru projects, 1. to check for loaded 2. to retrieve unloaded definition.
ddElement* ddElement::get_definition()
{
  Initialize(ddElement::get_definition);

  if (get_is_def())
    return this;

  symbolPtr sym = find_xrefSymbol();

  ddElement *_def = xrefSymbol::get_def_dd(sym);
  return _def;
}

symbolPtr ddElement::get_definition_file()
{
  Initialize(ddElement::get_definition_file);

  symbolPtr dfs = NULL_symbolPtr;
  symbolPtr sym = this;
  sym = sym.get_xrefSymbol();
  if (sym.xrisnotnull()) {
    dfs = sym->get_def_file();
  }
  return dfs;
}

ddElement* app_get_dd(app*ah, ddKind k, char const *n)
{
  if(!ah) return NULL;

  ddRoot* dr = ddRootPtr( get_relation(ddRoot_of_smtHeader, ah));

  if(!dr)
	return NULL;

  ddElement* dd = dr->lookup(k, n);

  return dd;
}

ddElement* symbolPtr_get_dds (const symbolPtr sym, objSet& os)
{
  Initialize(xrefSymbol_get_dds); 
  ddElement* def = NULL;
  if (! sym.isnull()) {
    ddKind dd_ki = sym.get_kind();
    char const *dd_nm  = sym.get_name();
    symbolArr as;
    sym->get_files(as, sym.get_xref()->get_lxref());
    for (int i = 0; i < as.size(); ++i){
      symbolPtr sym = as[i];
      char const *nm = sym.get_name();
      app*  ah = (nm && nm[0]) ? projHeader::load_module(nm) : NULL;
      if (!ah)
	continue;

      ddElement * dd = app_get_dd(ah, dd_ki, dd_nm);
      if(!dd)
	continue;

      if(dd->get_is_def())
	def = dd;

      os.insert(dd);
    }
  }
  return def;
}

Obj* ddElement::get_dds()
{
  Initialize(ddElement::get_dds);
  objSet* os = new objSet;
  symbolPtr_get_dds (this, *os);
  if(os->size() == 0){
    delete os;
    os = NULL;
  }

  return os;
}

Obj* ddElement::get_refs()
{
  Initialize(ddElement::get_refs);

  objSet * os = new objSet;

  ddElement * _def = symbolPtr_get_dds(this, *os);
  if(_def)
    os->remove(_def);

  if (os->size() == 0) {
    delete os;
    os = NULL;
  }
  return os;
}

static void switcht_rel_to(ddElement* de, appTree* at, RelType* rt, ddKind k)
{
  Initialize(switcht_rel_to);
  objSet os;
  Obj* ob;
  ddElement* dd;
  os = get_relation(rt, at);
  ForEach (ob, os) {
    if (!is_ddElement(RelationalPtr(ob)))
    {
      continue;
    }
    dd = checked_cast(ddElement,ob);
    if ((dd != de) && (dd->get_kind() == k))
      rem_relation(rt, at, dd);
  }
}

static int is_dd_type(ddKind k)
{
  return (k == DD_CLASS || k == DD_INTERFACE || k == DD_TYPEDEF || k == DD_ENUM);
}

// oper <= 0 means remove
void handle_user_of_used(ddElementPtr user, smtTreePtr smt, int oper)
{
  Initialize(handle_user_of_used(ddElementPtr, smtTreePtr, int));
//  ddElementPtr used = checked_cast(ddElement, get_relation(ref_dd_of_smt, smt));
  ddElementPtr used;
  if (is_ddElement(smt)) {
      used = ddElementPtr(smt);
  }
  else used = smt_get_dd(smt);
  if (used == 0 || ddKind_is_local(used->get_kind())) return;
  if (oper > 0) {
    if (user->get_kind() == DD_SEMTYPE)
      switcht_rel_to(user, smt, user_of_used, DD_SEMTYPE);
    symbolPtr used_sym = used->get_xrefSymbol();
    symbolPtr user_sym = user->get_xrefSymbol();
    if (used_sym.xrisnotnull() && user_sym.xrisnotnull()) {
      hashPair *rhps = get_remove_hashPair();
      used_sym->add_link(used_by, user_sym,rhps, 1);      
    }	
  }
  else if (!xref_hook_turn_off) {
    if ((user->get_kind() == DD_SEMTYPE) && is_dd_type(used->get_kind())) {
      symbolPtr used_sym = used->find_xrefSymbol();
      symbolPtr user_sym = user->find_xrefSymbol();
      if (used_sym.xrisnotnull() && user_sym.xrisnotnull())
	used_sym->rm_link(used_by, user_sym); // bi-dir removing
    }
    if (has_more_rel (user, used, smt) == 0)
      {
	symbolPtr used_sym = used->find_xrefSymbol();
	symbolPtr user_sym = user->find_xrefSymbol();
	if (used_sym.xrisnotnull() && user_sym.xrisnotnull())
	  used_sym->rm_link(used_by, user_sym); // bi-dir removing
      }
  }
}

void handle_used_of_user(ddElementPtr used, smtTreePtr smt, int oper)
{
  Initialize(handle_used_of_user(ddElementPtr, smtTreePtr, int));

  ddElement *user;
  if (is_ddElement(smt))
    user = checked_cast(ddElement, smt);
  else
    user = smt_get_dd(smt);
  
  if (user == 0 || ddKind_is_local(user->get_kind())) return;
  if (oper > 0) {
    symbolPtr user_sym = user->get_xrefSymbol();
    symbolPtr used_sym = used->get_xrefSymbol();
    if (used_sym.xrisnotnull() && user_sym.xrisnotnull()) {
      hashPair *rhps = get_remove_hashPair();
      user_sym->add_link(is_using, used_sym, rhps, 1);
    }
  }
  else if (!xref_hook_turn_off) {
    if (has_more_rel (user, used, smt) == 0)
    {
      symbolPtr user_sym = user->find_xrefSymbol();
      symbolPtr used_sym = used->find_xrefSymbol();
      if (used_sym.xrisnotnull() && user_sym.xrisnotnull())
	user_sym->rm_link(is_using, used_sym); // bi-dir removing
    }
  }
}

void handle_used_of_user(ddElementPtr subsys, ddElementPtr mbr, int oper)
{
  Initialize(handle_used_of_user);

  if (oper > 0) {
     symbolPtr subsys_xr = subsys->get_xrefSymbol();
     symbolPtr mbr_xr = mbr->get_xrefSymbol();
     if (subsys_xr.xrisnotnull() && mbr_xr.xrisnotnull()) {
       hashPair *rhps = get_remove_hashPair();       
       subsys_xr->add_link(is_using, mbr_xr, rhps, 1);
       if (mbr->get_kind() == DD_SUBSYSTEM) {
	 subsys_xr->add_link(has_subclass, mbr_xr, rhps, 1);	  
       }
       if (!mbr->get_is_def()) {
	 app * ah = mbr->get_main_header();
	 ddElement * dd_f = checked_cast(ddElement, get_relation(dd_of_appHeader, ah));
	 if (dd_f) {
	   symbolPtr user_sym = dd_f->get_xrefSymbol();
	   if (user_sym.isnotnull())
	     mbr_xr->add_link(ref_file, user_sym, rhps, 1);
	 }      
       }
     }
  }
}

void handle_template_of_instance(Relational* src, Relational* trg, int oper)
{
  Initialize(handle_template_of_instance);

  if (oper > 0) {
    ddElement* tmplt = checked_cast(ddElement, trg);
    ddElement* instance = checked_cast(ddElement, src);
     symbolPtr template_xr = tmplt->get_xrefSymbol();
     symbolPtr instance_xr = instance->get_xrefSymbol();
     if (template_xr.xrisnotnull() && instance_xr.xrisnotnull()) {
       hashPair *rhps = get_remove_hashPair();       
       instance_xr->add_link(has_template, template_xr, rhps, 1);
     }
  }
}

void handle_property_of_possessor(ddElementPtr src, ddElementPtr prop, int oper)
{
  Initialize(handle_property_of_possessor);

  symbolPtr prop_sym = prop;
  symbolPtr prop_xr = prop_sym.get_xrefSymbol();
  symbolPtr src_sym = src;
  symbolPtr src_xr = src_sym.get_xrefSymbol();
  if (prop_xr.xrisnotnull() && src_xr.xrisnotnull()) {
    hashPair *rhps = get_remove_hashPair();       
    if (oper > 0) {
      src_xr->add_link(has_property, prop_xr, rhps, 1);
    }
    else {
      src_xr->rm_link(has_property, prop_xr); // bi-dir removing
    }
  }
}

void handle_includes(ddElementPtr user, ddElementPtr used, int oper)
{
  Initialize(handle_includes);

  if (used == 0 || ddKind_is_local(used->get_kind())) return;
  if (oper > 0) {
    symbolPtr used_sym = used;
    used_sym = used_sym.get_xrefSymbol();
    symbolPtr user_sym = user;
    user_sym =  user_sym.get_xrefSymbol();
    if (used_sym.xrisnull() || user_sym.xrisnull()) return;
    hashPair *rhps = get_remove_hashPair();
    used_sym->add_link(included_by, user_sym,rhps,1);
  } else {
    symbolPtr used_sym = used;
    used_sym = used_sym.find_xrefSymbol();
    symbolPtr user_sym = user;
    user_sym =  user_sym.find_xrefSymbol();
    if (used_sym.xrisnull() || user_sym.xrisnull()) return;
    used_sym->rm_link(included_by, user_sym); // bi-dir removing
  }
}

#ifdef XXX_assoc
void handle_assoc (ddElementPtr assoc_dd, ddElementPtr entity_dd, int oper)
{
    Initialize (handle_assoc);

    if (oper == 0)
	return;
    if (assoc_dd == NULL || entity_dd == NULL)
	return;

    symbolPtr assoc_sym = assoc_dd->get_xrefSymbol ();
    symbolPtr entity_sym = entity_dd->get_xrefSymbol ();
    symbolPtr file_sym;

    if (assoc_sym.xrisnull () || entity_sym.xrisnull ())
	return;

    if (oper > 0) {
	hashPair* rhps = get_remove_hashPair();
	entity_sym->add_link (assoc_of_sym, assoc_sym, rhps, (int) true);
	linkType lt = (assoc_dd->get_is_def()) ? is_defined_in : ref_file;
	symbolPtr app_sym = assoc_dd->get_main_header();
	if (app_sym.isnull()) return;
	app_sym = app_sym.get_xrefSymbol();
	if (app_sym.xrisnull()) return;
	assoc_sym->add_link(lt, app_sym, rhps, 1);
	lt = (entity_dd->get_is_def()) ? is_defined_in : ref_file;
	entity_sym->add_link(lt, app_sym, rhps, 1);
    }
    else {
        file_sym = get_file_sym (assoc_dd);
        if (file_sym.xrisnull())
	    return;
        if (assoc_sym->get_kind () == DD_SOFT_ASSOC)
	    entity_sym->rm_link (assoc_of_sym, assoc_sym);
    }
}
#endif

void handle_struct_decl_of_ref(ddElementPtr fp, ddElementPtr cp, int oper)
{
  Initialize(handle_struct_decl_of_ref);
  if (fp->get_kind() == DD_PARAM_DECL)
    fp = checked_cast(ddElement, fp->get_parent());
  if (oper > 0) {
    symbolPtr cp_sym = cp;
    cp_sym = cp_sym.get_xrefSymbol();
    symbolPtr fp_sym = fp;
    fp_sym =  fp_sym.get_xrefSymbol();
    if (cp_sym.xrisnull() || fp_sym.xrisnull()) return;
    hashPair *rhps = get_remove_hashPair();
    cp_sym->add_link(arg_type_of, fp_sym,rhps,1);    
  }
  else if (!xref_hook_turn_off) {
    symbolPtr cp_sym = cp;
    cp_sym = cp_sym.find_xrefSymbol();
    symbolPtr fp_sym = fp;
    fp_sym =  fp_sym.find_xrefSymbol();
    if (cp_sym.xrisnull() || fp_sym.xrisnull()) return;
    cp_sym->rm_link(arg_type_of, fp_sym);
  }
}

void handle_has_friend(ddElementPtr used, smtTreePtr smt, int oper)
{
  Initialize(handle_has_friend);
  ddElement *f_dd = 0;
  if (is_ddElement(smt))
    f_dd = checked_cast(ddElement, smt);
  else
    f_dd = checked_cast(ddElement, get_relation(ref_dd_of_smt, smt));
  if (!f_dd) return;
  if (oper > 0) {
    symbolPtr c_sym = used->get_xrefSymbol();
    symbolPtr f_sym = f_dd->get_xrefSymbol();
    if (c_sym.xrisnull() || f_sym.xrisnull()) return;
    hashPair *rhps = get_remove_hashPair();
    c_sym->add_link(have_friends, f_sym,rhps, 1);      
  }
  else if (!xref_hook_turn_off) {
    symbolPtr c_sym = used->find_xrefSymbol();
    symbolPtr f_sym = f_dd->find_xrefSymbol();
    if (c_sym.xrisnull() || f_sym.xrisnull()) return;
    c_sym->rm_link(have_friends, f_sym);
  }


}


void handle_instance_of_assocType(ddElementPtr at, ddElement *in, int oper)
{
  Initialize(handle_instance_of_assocType);
  if (!at || !in) return;
  if (oper > 0) {
    symbolPtr at_sym = at->get_xrefSymbol();
    symbolPtr in_sym = in->get_xrefSymbol();
    if (at_sym.xrisnull() || in_sym.xrisnull()) return;
    hashPair *rhps = get_remove_hashPair();
    at_sym->add_link(instances_of_assocType, in_sym,rhps, 1);      
  }
  else if (!xref_hook_turn_off) {
    symbolPtr at_sym = at->find_xrefSymbol();
    symbolPtr in_sym = in->find_xrefSymbol();
    if (at_sym.xrisnull() || in_sym.xrisnull()) return;
    at_sym->rm_link(instances_of_assocType, in_sym);
  }
}

void handle_ifl_targ_of_src(ddElementPtr at, ddElement *in, int oper)
{
  Initialize(handle_ifl_targ_of_src);
  if (!at || !in) return;
  if (oper > 0) {
    symbolPtr at_sym = at->get_xrefSymbol();
    symbolPtr in_sym = in->get_xrefSymbol();
    if (at_sym.xrisnull() || in_sym.xrisnull()) return;
    hashPair *rhps = get_remove_hashPair();
    at_sym->add_link(ifl_targ_of_src, in_sym,rhps, 1);      
  }
  else if (!xref_hook_turn_off) {
    symbolPtr at_sym = at->find_xrefSymbol();
    symbolPtr in_sym = in->find_xrefSymbol();
    if (at_sym.xrisnull() || in_sym.xrisnull()) return;
    at_sym->rm_link(ifl_targ_of_src, in_sym);
  }

}

void handle_ref_smt_of_dd(ddElementPtr used, smtTreePtr smt, int oper)
{
  Initialize(handle_ref_smt_of_dd);
  if (oper < 0) {     // remove relation
    smtTree* def_smt = checked_cast(smtTree, get_relation(decl_of_id, smt));
    int node_type = (def_smt)? def_smt->get_node_type():-1;
    if (node_type == SMT_fdef) {
      if (!xref_hook_turn_off) {
	used->unset_is_def();
	rem_relation(def_smt_of_dd, used, 0);
	rem_relation(used_of_user, used, 0);
      }
    }
    else if (node_type == SMT_fdecl && !xref_hook_turn_off)
      rem_relation(smt_of_dd, used, def_smt);
    if (!xref_hook_turn_off) {
      if (smt->get_id())
	rem_relation(user_of_used, smt, 0);
    }
  }
  else if (used && !ddKind_is_local(used->get_kind())){ // put relation
    symbolPtr used_sym = used->get_xrefSymbol();
    linkType lt = used->get_is_def()? is_defined_in: ref_file;
    app * ah = used->get_main_header();
    ddElement * dd_f = checked_cast(ddElement, get_relation(dd_of_appHeader, ah));
    if (dd_f) {
      symbolPtr user_sym = dd_f->get_xrefSymbol();
      hashPair *rhps = get_remove_hashPair();
      if (used_sym.isnotnull() && user_sym.isnotnull()) {
	used_sym->add_link(lt, user_sym,rhps,1);
        if (lt == is_defined_in) {
          Obj * smts1 = get_relation(smt_of_dd, used);
          Obj * smts2 = get_relation(ref_smt_of_dd, used);
          if ((smts1 && smts1->collectionp() &&
	      ((objSet *)smts1)->size()) ||
              (smts2 && smts2->collectionp() &&
	      (((objSet *)smts2)->size() > 1)))
            used_sym->add_link(ref_file, user_sym,rhps,1);
        }
      }
    }      
  }
}

static ddElement * dd_get_scope (smtTree * smt, ddElement* dd)
{
    Initialize(dd_get_scope);
    
    ddElement * retval = NULL;

    for(smtTree *scope=smt; (!retval || retval == dd) && scope; scope = scope->get_parent())
	switch (scope->get_node_type()) {
	case SMT_fdef:
	case SMT_cdecl:
	    if (retval=checked_cast(ddElement,get_relation(def_dd_of_smt,scope)))
		break;

	case SMT_file:
            {
	    app * ah = scope->get_header();
	    retval = checked_cast(ddElement, get_relation(dd_of_appHeader, ah));
            }
	    break;
		
	default:
	    break;
	}
    
    return retval;
}

void handle_decl_smt_of_dd(ddElementPtr used, smtTreePtr smt, int oper)
{
  Initialize(handle_decl_smt_of_dd);

  symbolPtr used_sym = used->get_xrefSymbol();
  ddElement * decl_scope = dd_get_scope (smt, used);
  linkType lt = declared_in;

  if (decl_scope)
    if (oper < 0) {     // remove relation
      symbolPtr user_sym = decl_scope->get_xrefSymbol();
      if (used_sym.isnotnull() && user_sym.isnotnull())
	used_sym->rm_link(lt, user_sym);
    } else if (used && !ddKind_is_local(used->get_kind())){ // put relation
      symbolPtr user_sym = decl_scope->get_xrefSymbol();
      hashPair *rhps = get_remove_hashPair();
      if (used_sym.isnotnull() && user_sym.isnotnull())
	used_sym->add_link(lt, user_sym,rhps,1);
    }      
}

void handle_ref_app_of_dd(ddElementPtr used, appTree* smt, int oper)
{
  Initialize(handle_ref_smt_of_dd);
  if (!used || !smt) return;
  if (oper < 0) {     // remove relation
    Obj *os = get_relation(ref_app_of_dd, used);
    int has_node = 0;
    if (os && os->collectionp() && ((objSet *)os)->includes(smt))
	has_node = 1;
    linkType lt = used->get_is_def()? is_defined_in: ref_file;    
    if (!os || (has_node && os->size() == 1) ) {
      symbolPtr fsym = get_file_sym(used);
      symbolPtr sym = used->get_xrefSymbol();
      if (sym.isnull() || fsym.isnull()) return;
      hashPair *rhps = get_remove_hashPair();      
      sym->rm_link(lt, fsym);
    }
  }
  else {
    linkType lt = used->get_is_def()? is_defined_in: ref_file;        
    symbolPtr fsym = get_file_sym(used);
    symbolPtr sym = used->get_xrefSymbol();
    if (sym.isnull() || fsym.isnull()) return;
    hashPair *rhps = get_remove_hashPair();      
    sym->add_link(lt, fsym, rhps, 1);
  }

}

void handle_def_smt_of_dd(ddElementPtr used, smtTreePtr , int oper)
{
  Initialize(handle_def_smt_of_dd);
  if (ddKind_is_local(used->get_kind())) return;
  if (oper < 0) {     // remove relation
    symbolPtr used_sym = used->find_xrefSymbol();      
    if (used_sym.xrisnull()) return;

    if (used->get_kind() == DD_FIELD) {
      used_sym.rm_link(used_by);
      ddElement* pd = checked_cast(ddElement, used->get_parent());
      symbolPtr psym;
      if (pd)
	psym = pd->find_xrefSymbol();
      if (psym.isnotnull())
	xref_notifier_report(0, psym);
      xref_notifier_report(-1, used_sym);
    }
    if (!xref_hook_turn_off) {
      used->unset_is_def();
      rem_relation(used_of_user, used, 0);
    }
  }
  else {  // add rel
    used->set_is_def();
    if (!start_call_rel_hooks) {
      symbolPtr used_sym = used->get_xrefSymbol();      
      if (used_sym.xrisnull()) return;
      XrefTable *Xt = used_sym->get_XrefTable();
      Xt->put_def_file_for_sym(used, used_sym);
    }
  }
}

// smt is decl
void handle_smt_decl_of_semtype(ddElementPtr used, smtTreePtr smt, int oper)
{
  Initialize(handle_smt_decl_of_semtype);
  ddElement *dd_id = 0;
  if (is_ddElement(smt))
    dd_id = checked_cast(ddElement, smt);
  else {
    return ;
  }
  if ((dd_id == 0) || ddKind_is_local(dd_id->get_kind())) return; // local decl
  if (oper > 0) {
    if (!dd_has_type(dd_id->get_kind())) return ;
    symbolPtr used_sym = used->get_xrefSymbol();
    symbolPtr user_sym = dd_id->get_xrefSymbol();
    if (!(used_sym.xrisnotnull() && user_sym.xrisnotnull())) return;
    hashPair *rhps = get_remove_hashPair();
    user_sym->add_link(has_type, used_sym,rhps,1);
  }
  else if (!xref_hook_turn_off) {
    symbolPtr used_sym = used->find_xrefSymbol();
    symbolPtr user_sym = dd_id->find_xrefSymbol();
    if (!(used_sym.xrisnotnull() && user_sym.xrisnotnull())) return;
    user_sym->rm_link(has_type, used_sym); // bi-dir removing
  }
}

void handle_ref_file_of_dd(ddElementPtr used, smtTreePtr , int oper)
{
  Initialize(handle_ref_file_of_dd);
  
  if (ddKind_is_local(used->get_kind())) return;
  // oper < 0 means remove rel, >= 0 means add rel
  if (oper >= 0) {
    symbolPtr used_sym = used->get_xrefSymbol();
    if (used_sym.xrisnull()) return;
    symbolPtr dfs;
    XrefTable *Xt = used_sym->get_XrefTable();
    if (Xt)
      Xt->insert_def_file_link(used, dfs);
  }
}

void handle_superclassof(ddElementPtr user, smtTreePtr smt, int oper)
{
  Initialize(handle_superclassof);
  ddKind k;
  ddElement *used;
  if (is_ddElement(smt))
    used = checked_cast(ddElement, smt);
  else
    used = checked_cast(ddElement, get_relation(ref_dd_of_smt, smt));
  while (used && used->get_kind() == DD_TYPEDEF) {
    Obj *obs = get_relation(struct_decl_of_ref, used);
    Obj *ob;
    ddElement *targ = 0;
    ForEach(ob, *obs) {
      targ = checked_cast(ddElement, ob);
      if (targ->get_kind() == DD_CLASS)
	break;
      else
	targ = 0;
    }
    if (targ) {
      used = targ;
    }
    else break;
  }
  if (used == 0 || ((k = used->get_kind()) != DD_CLASS && k != DD_INTERFACE) ||
      user == 0 || ((k = user->get_kind()) != DD_CLASS && k != DD_INTERFACE))
    return;
  if (oper > 0) {
    symbolPtr used_sym = used->get_xrefSymbol();
    symbolPtr user_sym = user->get_xrefSymbol();
    if (used_sym.xrisnotnull() && user_sym.xrisnotnull() 
        && (used_sym != user_sym)) {
      hashPair *rhps = get_remove_hashPair();
      used_sym->add_link(has_subclass, user_sym,rhps,1);
    }
  }
  else if (!xref_hook_turn_off) {
    symbolPtr used_sym = used->find_xrefSymbol();
    symbolPtr user_sym = user->find_xrefSymbol();
    if (used_sym.xrisnotnull() && user_sym.xrisnotnull())
      user_sym->rm_link(has_superclass, used_sym); // bi-dir removing
  }
}

void handle_superclassof_att(ddElementPtr user, ddElement *used, int oper, RelType *rel)
{
  Initialize(handle_superclassof);

  if (used == 0 || used->get_kind() != DD_CLASS ||
      user->get_kind() != DD_CLASS)
    ;
  if (oper > 0) {
    symbolPtr used_sym = used->get_xrefSymbol();
    symbolPtr user_sym = user->get_xrefSymbol();
    if (used_sym.xrisnotnull() && user_sym.xrisnotnull() 
        && (used_sym != user_sym)) {
      linkType lt = get_linkType(rel);
      hashPair *rhps = get_remove_hashPair();
      user_sym->add_link(lt, used_sym,rhps,1);
    }
  }
  else if (!xref_hook_turn_off) {
    symbolPtr used_sym = used->find_xrefSymbol();
    symbolPtr user_sym = user->find_xrefSymbol();
    linkType lt = get_linkType(rel);
    if (used_sym.xrisnotnull() && user_sym.xrisnotnull())
      user_sym->rm_link(lt, used_sym); // bi-dir removing
  }
}

//------------------------------------------
// handle_grp_rel: insert/remove links for relations among groups/subsystems and
// their members.
//------------------------------------------

void handle_grp_rel(ddElementPtr src, ddElementPtr trg, int oper, RelType* rel) {
   Initialize(handle_group_rel);

   if (oper && src && trg) {
      symbolPtr srcxr = src->get_xrefSymbol();
      symbolPtr trgxr = trg->get_xrefSymbol();
      if (srcxr.xrisnotnull() && trgxr.xrisnotnull()) {
	 Xref* xr;
	 linkType lt = get_linkType(rel);
	 if (oper > 0) {
	    // Auto-clone seems not to be working any more (960925).  Let's
	    // make sure we get the correct processing by cloning the symbol
	    // explicitly if necessary.
	    xr = srcxr->get_Xref();
	    if (xr && !trgxr->is_symbol_in(xr)) {
	       XrefTable* xrt = xr->get_lxref();
	       if (xrt) {
		  xrt->insert_symbol(trg);
	       }
	    }
	    srcxr->add_link(lt, trgxr, 1, trgxr.get_xref()->get_lxref());
	 }
         else {
            // Adding a link automatically clones a symbol in a read-only pmod
            // into the home pmod; unfortunately, the same is not true for
            // removing a link, so we have to do it manually here.
            projNodePtr homep = projNode::get_home_proj();
            xr = (homep) ? homep->get_xref() : 0;
            if (xr && !trgxr->is_symbol_in(xr)) {
               symbolArr results;
               symbolPtr new_trg = xr->find_symbol(results, trg->get_kind(),
                                                   trg->get_ddname(),
                                                   trg->get_def_file());
               if (new_trg.xrisnotnull()) {
                  trgxr = new_trg;
               }
            }
            srcxr->rm_link(lt, trgxr);
         }
      }
   }
}

// src lt trg
void xref_add_link(ddElement* src, ddElement* trg, linkType lt)
{
    Initialize(xref_add_link);
    start_transaction() {
	symbolPtr src_sym;
	if (src) {
	    src_sym = src;
	    src_sym = src_sym.get_xrefSymbol();
	}

	symbolPtr trg_sym;
	if (trg) {
	    trg_sym = trg;
	    trg_sym = trg_sym.get_xrefSymbol();
	    if (trg_sym.xrisnotnull())
	    {
		trg_sym->set_att(trg);
// set attributes here
		xrefSymbol* symbol = trg_sym.operator->();
		if (symbol)
		    symbol->set_attribute(trg, trg_sym.get_xref()->get_lxref());
	    }	    
	}

	if (src_sym.xrisnotnull() && trg_sym.xrisnotnull())
	    src_sym->add_link(lt, trg_sym, 1, trg_sym.get_xref()->get_lxref());
    }end_transaction();
}

//
// callback for setting up links for relation "scope_of_entity"
// 	e.g., struct A { struct B {} }, A is "parent_of" B, B is "member_of" A
//            (existing, yet unused link)
//
void
handle_scope_of_entity(ddElement * src, ddElement * trg, int oper)
{
    Initialize(handle_scope_of_entity);
    
    //if oper > 0  : add links
    //if oper == 0 : do nothing, oper is zero while restoring file
    //if oper < 0  : remove links

    if (oper == 0) return;

    if (src!=0 && trg!=0) {
	// -- add links
	if (oper > 0) {
	    symbolPtr src_sym = src->get_xrefSymbol();
	    symbolPtr trg_sym = trg->get_xrefSymbol();
	    if (src_sym.xrisnotnull() && trg_sym.xrisnotnull()) {
		hashPair *rhps = get_remove_hashPair();
		src_sym->add_link(parent_of, trg_sym.operator->(), rhps, 1);
	    }
	} else {
	// -- remove links
	    symbolPtr src_sym = src->find_xrefSymbol();
	    symbolPtr trg_sym = trg->find_xrefSymbol();
	    if (src_sym.xrisnotnull() && trg_sym.xrisnotnull())
		src_sym->rm_link(parent_of, trg_sym);
	}
    }
}

void loadedFiles_get_apps(objArr& app_headers);
ddElement* dd_get_loaded_dds(ddElement * dd, objSet& os)
{
  ddElement*def = NULL;
  ddKind kd = dd->get_kind();
  char const *nm = dd->get_name();

  objArr apps;
  loadedFiles_get_apps(apps);
  Obj*obj;
  ForEach(obj, apps){
    app* ah = appPtr(obj);
    ddElement*dd = app_get_dd(ah, kd, nm);
    if(dd){
      if(dd->get_is_def())
	def = dd;
      os.insert(dd);
    }
  }

  return def;
}

Obj* ddElement::get_loaded_dds()
{
  Initialize(ddElement::get_loaded_dds);
  
  objSet * os = new objSet;

  dd_get_loaded_dds(this, *os);

  if(os->size() == 0){
    delete os;
    os = NULL;
  }
  return os;
}

// caller must delet set
Obj* ddElement::get_loaded_refs()
{
  objSet * os = new objSet;

  ddElement * _def = dd_get_loaded_dds(this, *os);
  if(_def) 
    os->remove(_def);

  if(os->size() == 0){
    delete os;
    os = NULL;
  }
  return os;

}


ddElement* ddElement::get_loaded_definition()
{
  Initialize(get_loaded_definition);
  if (get_is_def()) return this;

  symbolPtr sym = get_xrefSymbol();
  if(sym.is_loaded())
    return get_definition();
  else
    return NULL;
}


void dd_replace_string(ddElement* de, char const *on, char const *nn);
void ddElement::update_name(char const *on, char const *nn)
{
  Initialize(ddElement::update_name);
  genString mon;
  symbolPtr sym = NULL_symbolPtr;
  if (nn == 0)
    mon = on;
  else {
    mon = get_ddname();
    sym = find_xrefSymbol();
    dd_replace_string(this, on, nn);
  }
  if (strcmp(mon, get_ddname())) {
    ddRoot* dr = get_dd_root();
    dr->update_hash(this, mon, get_kind());
  }

  if (sym.isnotnull() && nn && strcmp(sym.get_name(), get_ddname()))
    sym->update_name(get_ddname()); 
}

int ddElement::get_loaded_refs_same_file(objArr& os, ddKind k)
{
  Initialize(ddElement::get_loaded_refs_same_file);
  symbolArr as;
  symbolPtr sym = get_xrefSymbol();
  if (sym.xrisnull()) return 0;
  sym->get_refs(as, k);
  app* ah = get_main_header();
  ddRoot *dr = dd_sh_get_dr(ah);
  for (int i = 0; i < as.size(); ++i) {
    sym = as[i].get_xrefSymbol();
    ddElement* dd = dr->lookup(sym->get_kind(), sym.get_name());
    if (dd) os.insert_last(dd);

  }
  return os.size();
}

// need to be DD_MODULE
app* xrefSymbol_load_file(xrefSymbol * that, XrefTable* that_xr)
{
  Initialize(xrefSymbol_load_file);
  Assert(that->get_kind() == DD_MODULE);
  genString fname = that->get_name(that_xr);
  app * ah =  projHeader::load_module(fname);
  return ah;
}

// static version of get_def_dd, which can be invoked without an xrefSymbol
ddElement * symbolPtr_get_def_dd(symbolPtr & that)
{
  ddElement * def = NULL;
  if (that.relationalp()){
    Relational * rel = that;
    if (is_ddElement(rel)) {
      ddElement *dd = (ddElement*)rel;
      return  dd->get_definition ();
    }
  } 

  def = xrefSymbol_get_def_dd(that);
  return def;
}




void Xref::crash_recovery_pmod_from_pset()
/* This function is called upon first reference to a pmod, to redo any
    operation that may have been in progress when the last copy of paraset
    crashed.  This assumes that two copies do not both run at the same time,
    with the same home.
    The *.pmod.outdated_pset file, if it exists, contains a list of
    operations that need to be performed before paraset begins using the
    pmod.
*/
{
  Initialize(Xref::crash_recovery_pmod_from_pset);
  get_lxref();
  if (my_proj == 0) return;
  if (crash_recovery) return;
  if (!customize_get_write_flag())      // skip crash recovery if we are not going to write to pmod
      return;                           //  (for example, -checkpmod, -print, ...)
  genString crash_file_name = xref_name;
  crash_file_name += ".outdated_pset";

  int crash_file = OSapi_open(crash_file_name, O_RDONLY, 0644);
  if (crash_file < 0) return;
  FILE *fp = OSapi_fdopen(crash_file, "r");
  if (fp == 0) {
    OSapi_unlink(crash_file_name.str());
    return;
  }

  objSet pms;
  symbolArr fsa;
  char filename[512];
  projModule *pm;
  symbolPtr fs;
  start_transaction() {
  while (OSapi_fgets(filename, 512, fp)) {
    if (filename[0] == 0) continue;
    int fl = strlen(filename);
    if (fl < 1) continue;
    if (filename[fl-1] == '\n')
      filename[fl-1] = '\0';
    pm = my_proj->find_module(filename);
    if (pm)
      pms.insert(pm);
    else {
      fs = lookup_module(filename);
      if (fs.xrisnotnull() && !fsa.includes(fs))
	fsa.insert_last(fs);
    }
  }
  Obj* ob;
  ForEach(ob, pms) {
    pm = checked_cast(projModule, ob);
    
    if (pm && !pm->is_loaded() && pm->is_good_paraset_file()) {
      pm->restore_module(1);
      pm->unload_module();
    }
  }
  ForEachS(fs, fsa) {
    remove_module(fs.get_name());
  }
  }end_transaction();
  OSapi_fclose(fp);
  crash_recovery = 1;
}

void Xref::save_lxref_by_module(char const *filename)
{
    Initialize(Xref::save_lxref_by_module);

    if (my_proj == 0) {
      IF (SharedXref::get_cur_shared_xref())
	return;

      XrefTable *lxr = get_lxref();
      if (filename && lxr && !(lxr->nosaveflag)) {
	XrefTable *Temp = lxr;
	XrefTable *Perm = Temp->get_backup ();
	SharedXref SXr (Perm);

	if (Perm) {
	  // these conditionals are only for performance purposes
	  if (SXr.lookup_module_priv(filename))	// If module is in permanent,
	    Perm->remove_module(filename);	// remove module from perm
	  if (lookup_module_priv(filename))	// If module is in temporary
	    Temp->copy_module1(filename, Perm);	// copy from temporary to permanent
	  delete Temp->backup;
	  Temp->backup=0;
	}
      }
    }
    else if (get_perm_flag() != READ_ONLY) {
      // register for err rec. save
      register_file_for_crash_recovery(filename);
    }
      
}

// add file name to backup file
void Xref::register_file_for_crash_recovery(char const *filename)
{
  Initialize(Xref::register_file_for_crash_recovery);
  if (get_perm_flag() == READ_ONLY) return;
  genString crash_file_name = xref_name;
  crash_file_name += ".outdated_pset";
  int crash_file = OSapi_open(crash_file_name, O_RDWR|O_CREAT|O_APPEND, 0644);
  if (crash_file < 0) return;
  OSapi_write(crash_file, filename, strlen(filename));
  OSapi_write(crash_file, "\n", 1);
  OSapi_close(crash_file);
}
