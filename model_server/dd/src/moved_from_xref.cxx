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
#ifndef _WIN32
#include <sys/param.h>
#endif
#include <msg.h>
#include <genError.h>
#include <gen_hash.h>
#include <xrefPair.h>
#include <symbolPtr.h>
#include <symbolSet.h>
#include <symbolArr.h>
#include <RTL_externs.h>
#include <proj.h>
#include <fileCache.h>
#include <ddSymbol.h>
#include <linkTypes.h>
#include <link_type_converter.h>
#include <linkType_selector.h>
#include <Link.h>
#include <xrefSymbol.h>
#include <xrefSymbolSet.h>
#include <_Xref.h>
#include <XrefTable.h>
#include <groupHdr.h>
#include <messages.h>
#include <cLibraryFunctions.h>
#include <db_intern.h>
#include <metric.h>
#include <driver_mode.h>

extern hashPair* rhp;
extern int not_call;
void file_rm_dup_links(symbolPtr sym);
int compute_dd_size(app *, int *);
int Xref_size_limit_exceed(headerInfo * hi, int size);
void set_cache_app(XrefTable*, char const *, symbolPtr);
extern bool is_forgiving();
symbolPtr xref_header_file_included_from(char const *fn);
bool do_delete_pset();

#define SAVING_INTERVAL 75

void copy_home_pmod_to_perm();

static int num_files_inserted_in_xref = 0; // when this reach 50, call copy_home_pmod_to_perm

void  inc_num_files_inserted_in_xref()
{
  num_files_inserted_in_xref ++;
}



RelType* get_rel_type(linkType lt)
{
  switch(lt) {
  case included_by:
    return smt_included;
  case is_including:
    return smt_includes;
  case is_using:
    return used_of_user;
  case used_by:
    return user_of_used;
  case is_defining:
    return elements_of;
  case has_superclass:
    return superclassof;
  case has_subclass:
    return subclassof;
  case assocType_of_instances:
    return assocType_of_instance;
  case instances_of_assocType:
    return instance_of_assocType;
  case have_friends:
    return has_friend;
  case friends_of:
    return friend_of;
  case has_type:
    return smt_decl_of_semtype;
  case type_has:
    return semtype_of_smt_decl;
    
    case has_vpub_subclass: return vpub_subclassof ;
    case has_vpri_subclass: return vpri_subclassof ;
    case has_pub_subclass: return pub_subclassof ;
    case has_pri_subclass: return pri_subclassof ;
    case has_vpro_subclass: return vpro_subclassof ;
    case has_pro_subclass: return pro_subclassof ;
    
    case has_pro_superclass: return pro_baseclassof ;
    case has_vpub_superclass: return vpub_baseclassof ;
    case has_vpri_superclass: return vpri_baseclassof ;
    case has_pub_superclass: return pub_baseclassof ;
    case has_pri_superclass: return pri_baseclassof ;
    case has_vpro_superclass: return vpro_baseclassof ;

  default:
    ;
  }
  return 0;
}

class astRoot *dfa_build(smtHeader *);

void
XrefTable_insert_module(XrefTable *that,
			app *sh)
{
  Initialize(XrefTable_insert_module(XrefTable*, app *));

  if (sh == 0)
    return;

  if (is_smtHeader(sh) && ((checked_cast(smtHeader,sh)->srcbuf == 0)))
    return;

  int num_syms = 0;
  int sh_size = compute_dd_size(sh, &num_syms);
#ifdef _PSET_DEBUG
  OSapi_fprintf (stdout, "Size of %s  %d\n", sh->get_filename(), sh_size);
#endif
#ifdef _WIN32
#ifdef _DEBUG
  OSapi_fprintf (stdout, "Size of %s  %d\n", sh->get_filename(), sh_size);
#endif
#endif 

  if (Xref_size_limit_exceed(that->get_header(), num_syms))
    msg("Pmod file $1 may exceed the 16 million symbol size limit. Please partition your project further.", warning_sev) << that->indfile_name.str() << eom;
  
  that->make_room(250000 + sh_size);	// move the table if not big enough for this addition
  Xref *xr = last_Xref =  that->get_Xref();
  // clean up any unreferenced xrefEntry values, anywhere
  
  rhp = new hashPair(1000);

  xrefSymbol *file_sym = that->lookup_module(sh->get_filename());

  /* force build_remove_module to always process this file */
  if (file_sym && !file_sym->get_has_def_file(that))
    file_sym->add_link(is_defined_in, file_sym,rhp,1);  
  
  that->build_remove_module( sh->get_filename(), rhp);

  sh->get_root();
  // dealing will file first
  if (file_sym == 0)
    file_sym = that->insert_symbol(sh);

  if (!file_sym) return;
  
  if (file_sym)
      file_sym->add_link(is_defined_in, file_sym,rhp,1);
  
  ddRoot *dr = checked_cast(ddRoot, 
			    get_relation(ddRoot_of_smtHeader, sh));
  if (dr == 0) {
    that->init_link(file_sym, sh);    
    return;
  }

  that->init_link(file_sym, sh);
  Xref *Xr = file_sym->get_Xref();
  char const *file_sym_name = file_sym->get_name(that);
  set_cache_app(that, sh->get_filename(), symbolPtr(xr, file_sym));

  not_call = 0;
  that->call_rel_hooks(dr);

  if (is_model_build() && do_delete_pset())
    dr->setup_SWT_Entity_Status(0);

  // removing old links
  that->remove_old_links(rhp);

  bool is_smt = is_smtHeader(sh);
  if (is_smt)
    file_rm_dup_links(symbolPtr(xr, file_sym));
  
  char const *fn = sh->get_phys_name();

  if (fn) {
     time_t mod_date = get_last_modified_date(fn);

     if (mod_date) {
	file_sym->set_last_mod_date(mod_date, that);
     }
  }

  delete rhp;
  rhp = 0;
  set_cache_app(0, 0, NULL_symbolPtr);

  if (!is_smt)
    return;

  dfa_build(checked_cast(smtHeader,sh));
  MetricSet::set_metrics_of_smtHeader(checked_cast(smtHeader,sh), true);
}

void XrefTable_insert_SWT_Entity_Status (app* head)
{
  Initialize(XrefTable_insert_SWT_Entity_Status);

  if (head == 0) return;
  ddRoot *dr = checked_cast(ddRoot, get_relation(ddRoot_of_smtHeader, head));
  if (dr == 0) return;
  ddElement* dd;  
  for (int i = 0; i < MAX_HASH; ++i ) {
    hash_element* he = dr->dd_hash[i];
    while (he) {
      dd = he->el;
      if (dd->get_is_def()) {
	symbolPtr sym = dd->find_xrefSymbol();
	if (sym.isnotnull())
	  sym->set_attribute(dd->get_sw_tracking(), SWT_Entity_Status, 2);
      }
      he = he->next;
    }
  }
}

void XrefTable_call_rel_hooks(XrefTable * that, ddRoot* dr)
{
  Initialize(XrefTable_call_rel_hooks);

  app *ah = checked_cast(app, get_relation(smtHeader_of_ddRoot, dr));
  if (!ah) return;
  symbolPtr dfs = that->lookup_module(ah->get_filename());
  ddElement* dd;  
  for (int i = 0; i < MAX_HASH; ++i ) {
    hash_element* he = dr->dd_hash[i];
    while (he) {
      dd = he->el;
// Edited following code to call rel hooks for groups/subsystems only
// for the definition ddElement -- otherwise, you get duplicate links
// when the hooks are called for the relation and for its inverse.
// (Apparently this problem does not afflict non-group modules because
// the version of xrefSymbol::add_link that takes a hashPair* argument
// avoids duplicates, but groups can't use that entry because references
// are often to symbols in other pmods that must be cloned into the home
// pmod.  -- wmm, 960401
      if (dd ) {
	dd->call_rel_hooks(1);
      }
      if (dd /* && need_to_set_att(dd) */) {
	symbolPtr sym = dd->find_xrefSymbol();
	if (sym.isnotnull()) {
	  if (dd->get_kind() == DD_SEMTYPE)
	    sym.set_datatype(dd->get_datatype());
	  sym->set_attribute(dd, sym.get_xref()->get_lxref());
	}
      }
      he = he->next;
    }
  }
}

//  This function only inserts *certain* information about the module, not the whole thing
//    According to Trung, the rest of the information has already been updated on the
//    fly, while the person was editing.
void XrefTable_insert_module(XrefTable *that, app *sh, int)
{
  Initialize(XrefTable_insert_module(app*,int));
  that->make_room(250000);	// move the table if not big enough for this addition
  Xref* xr =  last_Xref = that->get_Xref();

  // clean up any unreferenced xrefEntry values, anywhere

  if (sh == 0) return;

  // dealing with file first
  xrefSymbol *file_sym = that->lookup_module(sh->get_filename());
  if (file_sym == 0)
    file_sym = that->insert_symbol(sh);
  if (!file_sym) return;
    file_sym->add_link(is_defined_in, file_sym, xr);
  that->add_link(file_sym, is_including, sh); // add many links
  that->add_link(file_sym, included_by, sh);  // add many links

  char const *fn = sh->get_phys_name();

  if ((char const *) fn) 
	file_sym->set_last_mod_date(get_last_modified_date(fn),that);
  file_sym->set_language(checked_cast(app, sh)->get_language(), that);  
  return;
}

linkType get_linkType(RelType * lt)
{
    if (lt ==  smt_included)
      return included_by;
    if (lt ==  smt_includes)
      return is_including;
    if (lt ==  used_of_user)
      return is_using;
    if (lt ==  user_of_used)
      return used_by;
    if (lt ==  elements_of)
      return is_defining;
    if (lt ==  superclassof)
      return has_superclass;
    if (lt ==  subclassof)
      return has_subclass;
    if (lt ==  assocType_of_instance)
      return assocType_of_instances;
    if (lt ==  instance_of_assocType)
      return instances_of_assocType;
    if (lt ==  has_friend)
      return have_friends;
    if (lt ==  friend_of)
      return friends_of;
    if (lt ==  smt_decl_of_semtype)
      return has_type;
    if (lt ==  semtype_of_smt_decl)
      return type_has;
    
    if (lt ==  vpub_subclassof )
      return has_vpub_subclass;
    if (lt ==  vpri_subclassof )
      return has_vpri_subclass;
    if (lt ==  pub_subclassof )
      return has_pub_subclass;
    if (lt ==  pri_subclassof )
      return has_pri_subclass;
    if (lt ==  vpro_subclassof )
      return has_vpro_subclass;
    if (lt ==  pro_subclassof )
      return has_pro_subclass;

    if (lt ==  pro_baseclassof )
      return has_pro_superclass;
    if (lt ==  vpub_baseclassof )
      return has_vpub_superclass;
    if (lt ==  vpri_baseclassof )
      return has_vpri_superclass;
    if (lt ==  pub_baseclassof )
      return has_pub_superclass;
    if (lt ==  pri_baseclassof )
      return has_pri_superclass;
    if (lt ==  vpro_baseclassof )
      return has_vpro_superclass;

  return (linkType)0;
}

projNode* app_get_proj(app* ah)
// given an appheader, get the projNode forit
{
  Initialize(app_get_proj);
  projNode *pr = appHeader_get_projectNode(ah);
  projModule* mod = appHeader_get_projectModule(ah);
  if (pr && mod) {
    Xref * Xr = pr->get_xref();
    if (Xr && !Xr->is_unreal_file())
      return pr;
  } 
  if (mod){
    // find pmod project
    pr = mod->get_project();
    while(pr->get_xref(1) == NULL){
      projNode * par = pr->find_parent();
      if(!par) 
	break;
      pr = par;
    }
    appHeader_put_projectNode(ah, pr);
    return pr;
  }

  if(pr == 0)
    return NULL;

  if (pr->is_dummy_project()) {
      //.... is special project for pmod_diff logic...
      return pr;
  }
  if (is_smtHeader(ah) && (smtHeaderPtr(ah)->parsed == 0)) return pr;
  Xref * Xr = pr->get_xref();
  if (Xr && !Xr->is_unreal_file()) return pr;
  genString projname;
  pr->fn_to_ln_imp ((char const *)ah->get_phys_name(), projname);
  if (projname.str() == 0) {
    msg("Error: cannot convert $1 from $2") << ah->get_phys_name() << eoarg << pr->get_name() << eom;
    genString nln;
    projNode *npr = 0;
    projHeader::fn_to_ln(ah->get_phys_name(), nln, &npr);
    if (npr == 0) return 0;
    npr->fn_to_ln_imp ((char const *)ah->get_phys_name(), projname);
    if (projname.str() == 0) return npr;
    pr = npr;
  }
  // find particular subproject for this project name
  projNode* subproj=pr->get_pmod_project_from_ln(ah->get_filename());
  if (subproj == 0) return pr;
  if (subproj != pr) {
    appHeader_put_projectNode(ah, subproj);
    Xref* xr = subproj->get_xref();
    IF (xr == 0) {
        msg("Error: cannot get current project!") << eom;
	return subproj;
    }
  }
  return subproj;
}

static int should_insert_smtHeader(smtHeader *sh, Xref* xr)
{
  return (sh->srcbuf &&
	  ( (sh->is_src_modified() || sh->arn || sh->drn) ||
	    xr->need_to_update(sh->get_filename(), sh->get_phys_name())));
}

int projModule_set_version (app *);
void XREF_insert_module(app* ah) // for building xref
{
  Initialize(XREF_insert_module );
  if (ah == 0) return;
  projNode* pr = app_get_proj(ah);
  IF (pr == 0) {
    return;
  }
  Xref* xr = pr->get_xref();
  if (xr == 0 || xr->get_perm_flag() == READ_ONLY)
    return;

  int vres = projModule_set_version (ah);

  XrefTable* xref = xr->get_which_xref(ah);
  if (is_smtHeader(ah)){
    smtHeader *sh = checked_cast(smtHeader, ah);
    if (should_insert_smtHeader(sh, xr))
      xref->insert_module(ah);
  }
  else 
    xref->insert_module(ah);
  
  if ((num_files_inserted_in_xref >= SAVING_INTERVAL) && is_model_build()) {
    copy_home_pmod_to_perm();
    num_files_inserted_in_xref = 0;
  }

}

app * get_app_for_file_sym(symbolPtr fs)
{
  Initialize(get_app_for_file_sym);
  if (fs.isnull() || fs.get_kind() != DD_MODULE)
      return 0;

  char const *name;
  smtHeader* nh;
  name = fs.get_name ();
  nh = smtHeader::find (name);
  if (nh)
    return nh;
   
  projNode* pr = fs.get_projNode ();
  if (pr == 0)
      pr = lookup_all_proj_for_symbol(fs);
  Assert (pr);
  genString pname;
  pr->ln_to_fn (name, pname);
  if (pname == (char *) 0) {
      symbolArr file_ar;
      int num = lookup_all_file_symbol(file_ar, name);
      for (int iii = 0; iii < num; ++iii) {
	  symbolPtr ffs =  file_ar[iii];
	  if (ffs.get_has_def_file() &&
	      (pr = lookup_all_proj_for_symbol(ffs))) {
	      pr->ln_to_fn(name, pname);
	      if (pname.length())
		  break;
	  }
      }
      if (pname.length() == 0)
	  return 0;
  }
  smtLanguage lang = smtLanguage (fs.get_language ());
  return db_new (smtHeader, (name, pname, lang, &pr));
}

// the return app is not in ready form for normal usage
// only intended for smt_ast
// harazdous to your health
// potentially hazardous
// problematic if some .h file is not included by any one

app* XREF_get_files_including(char const *fn)
{
  Initialize(XREF_get_files_including);
  
  symbolArr bs;
  symbolArr as;
  xrefSymbol* fs;
  int bs_len = lookup_all_file_symbol(as, fn);
  int i;
  for (i = 0; i < bs_len; ++i){
    fs = (xrefSymbol*) as[i];
    if (fs->get_link(included_by, bs))
      break;
  }

  symbolArr processed;
  symbolArr leafs;
  
  bs_len = bs.size();

  for (i = 0; i < bs_len; ++i) {
    fs = (xrefSymbol*) bs[i];
    if (processed.includes(fs))
      continue;
    else
      processed.insert_last(fs);
    if (bs_len == fs->get_link(included_by, bs)){
      app *nh = get_app_for_file_sym(fs);
      if (nh) {
	if (is_forgiving())
	  return nh;
	
	if (!db_get_bit_forgive(nh->get_phys_name()))
	  return nh;
      }
    }
    else
      bs_len = bs.size();
  }

  return 0;
}


int XREF_check_files_including(char const *ln)
{
    Initialize(XREF_check_files_including);
    if (ln == 0)
        return 0;

    symbolPtr sym = xref_header_file_included_from(ln);
    return ! sym.isnull();
}


