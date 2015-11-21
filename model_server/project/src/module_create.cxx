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
#include <machdep.h>
#include <proj.h>
#include <path.h>
#include <proj_save.h>
#include <fileCache.h>
#include <fileEntry.h>
#include <transaction.h>
#include <pathcheck.h>
#include <xref.h>
#include <mpb.h>

init_relational(projModule,appTree);
init_rel_or_ptr (projModule,projectModule,0,app,appHeader,relationMode::D);
init_rel_or_ptr (projNode,projectNode,0,app,appHeader,relationMode::D);

int get_PDF_DebugLevel() {
    static int PDF_DebugLevel = -1;
    if (PDF_DebugLevel == -1)
	PDF_DebugLevel = customize::getIntPref("DIS_internal.PDF_DebugLevel");
    return PDF_DebugLevel;
}

projModule::projModule(char const *ln, projContents *cont, char const *phy_path, int i_selector)
   : ln_name(ln), app_head(0), app_ln (0), cache_valid(0), included(i_selector),
     del_flag(0), cur_ver(0), target_proj(0), projfile_name(NULL), psetfile_name(NULL)
{
    Initialize(projModule::projModule);
    module_put_proj (this, cont);
    projNode *pr = get_project ();
    symbolPtr sym;
    if(pr) 
      sym = pr->lookup_symbol (DD_MODULE, ln);
    if (sym.xrisnotnull()) {
	symbolPtr symb = sym;
	commonTree::language = (enum fileLanguage)symb.get_language_local();
    }

    if (commonTree::language == FILE_LANGUAGE_UNKNOWN) {
	commonTree::language = guess_file_language(phy_path);
    }

    if (phy_path) {
	set_sname(phy_path);
	add_module_to_hashes (this);
    }
} 

projModule::projModule (projNode *prnt, projLine& pline)
   : app_head(0), app_ln (0), cache_valid(0), del_flag(0), cur_ver(0),
     target_proj(0), projfile_name(NULL), psetfile_name(NULL),
     included(pline.included)
{
    Initialize(projModule::projModule__proNode*__projLine&);
    projContents *pcont = (prnt) ? prnt->get_contents_node() : NULL;
    if (!pcont) return;
    module_put_proj (this, pcont);
    imp_name             = pline.left_name;
    ln_name              = pline.right_name;
    commonTree::language = pline.lang;

    projfile_name = global_cache.add_entry (pline.phys_name);
    if (projfile_name) {
	projfile_name->restore_flags (pline, PRJ_phys, prnt->is_writable());
	add_module_to_hashes (this);
    }
}

// projModule::~projModule
//
// Destructor


projModule::~projModule()
{
    Initialize(projModule::~projModule);

    start_transaction()
    {
	int is_removed = 0;
	if (get_sname())
	    is_removed = remove_module_from_hashes (this);

        invalidate_cache();
        projNode *pn = get_project();
        while(pn && !pn->get_map())
            pn = pn->find_parent();
        if (pn)
        {
            // clean the module hash table
            pn->remove_module_hash(this);

            // clean the path hash table
            projMap *pm = pn->get_map();
            if (pm)
            {
                genString fn;
                get_phys_filename(fn);
                // clean the hash table for physical name to project name and physical name to logical name
                if (fn.length())
                {
                    pm->remove_hash(fn, FILE_TYPE, PROJECT_NAME_RULE, FNTOLN, pn->get_ln());
                    pm->remove_hash(fn, FILE_TYPE, LOGIC_NAME_RULE,   FNTOLN, pn->get_ln());
                }
                // clean the hash table for project name to physical name and logical name to physical name
                if (ln_name.length())
                {
                    pm->remove_hash((char *)ln_name, FILE_TYPE, PROJECT_NAME_RULE, LNTOFN, pn->get_ln());
                    pm->remove_hash((char *)ln_name, FILE_TYPE, LOGIC_NAME_RULE,   LNTOFN, pn->get_ln());
                }
            }
        }
    } end_transaction();
}

projModule* projNode::put_module(char const *nm, char const *ln)
{
 Initialize(projNode::put_module);

 refresh();

 projModule* mod = this->find_module (ln);

 if (!mod) {
     mod = raw_put_module(nm, ln);
 }
 return mod;
}

projModule* projNode::raw_put_module(char const *nm, char const *ln, int included)
{
    Initialize(projNode::raw_put_module);

    projContents * cont = get_contents_node();
    projModule * mod = db_new(projModule, (ln, cont, nm, included));

    module_put_proj (mod, cont);
    rtl_insert(mod, 0);

    projNode *pn = this;
    while(pn && !pn->get_map())
        pn = pn->find_parent();
    if (pn)
        pn->insert_module_hash(mod);

    return mod;
}


projModule *proj_restore_module (projNode *prnt, projLine& pline)
{
    Initialize(proj_restore_module__projNode*__projLine&);
    projModule *mod = NULL;
    projContents * cont = (prnt) ? prnt->get_contents_node() : NULL;
    if (cont && pline.left_name.length()) {
	mod = new projModule (prnt, pline);
	if (mod->get_pr_name() == NULL) {
	    delete mod;
	    mod = NULL;
	} else {
	    module_put_proj (mod, cont);
	    prnt->rtl_insert(mod, 0);
	    
	    projNode *pn = prnt;
	    while(pn && !pn->get_map())
		pn = pn->find_parent();

	    if (pn)
		pn->insert_module_hash(mod);
	}
    }

    return mod;
}

struct old_dir_entry *found_old_dir(char const *name, int type);

extern int num_of_fail_get_log_name;
int pdf_tree_parse (char const *fn, int start);
void pdf_tree_fn_to_ln_internal (int off, char const *fn, genString& ln, projNode **pn);

void my_make_module(char const *phys_name, projNode *pn, projMap *proj_head, int force_refresh_flag, int list_proj)
// create a new module
//
// phys_name: physical path for this module
// pn: projNode where this module belongs to
// proj_head: projMap where this module belongs to
// force_refresh_flag: if set, this is called by refresh project. this module might already exist.
//                     if not set, this is an original call and make a new module.
// list_proj = 1: it is a list project; proj_head is the parent project
//           = 0: it is a regular project; proj_head is a leaf project
{
    Initialize(my_make_module);

    if (!proj_head->get_view_flag())
	return;

    int idx               = -1;
    projNode   *found_pn  = NULL;
    projModule *found_mod = find_module_by_physname (phys_name, pn, idx, found_pn);
    if (found_mod) {
	if (get_PDF_DebugLevel() > 0) {
	    if (strcmp(pn->get_name(), found_pn->get_name())) {
		msg("PDF DEBUG: Phys name \"$1\"") << phys_name << eom;
		msg("           from project: $1") << pn->get_name() << eom;
		msg("           is hidden by: $1, ln: $2") << found_pn->get_name() << eoarg << found_mod->get_name() << eom;
		;
	    } else {
		msg("PDF DEBUG: Phys name \"$1\"") << phys_name << eom;
		msg("           is multiply specified in project: $1") << pn->get_name() << eom;
		;
	    }
	}
	return;
    }
    
    genString ln;
    int pdf_tree_checked = 0;

    if (!list_proj) {
        proj_head->set_this_is_target(1);
	int start = proj_head->get_root_pdf_tree_arr_index();
	if (!force_refresh_flag && start > -1) {
	    // indexes starts from zero (array index)
	    int end   = proj_head->get_leaf_pdf_tree_arr_index();

	    // offset is the line number, that starts from 1
	    int off   = pdf_tree_parse (phys_name, start);

	    // The concervative algorithm: if the filename could not be found in
	    // the pdf_tree_arr, or found after the curent leaf, then stop searching.
	    // If the mame is on exact cur_leaf line, then we found it and arrow-like
            // fn_to_ln() finds ln. If the phys_name is found before the cur_leaf, then
            // let full-power fn_to_ln to handle the case (leave pdf_tree_checked = 0).
	    if (off == 0 || off > (end + 1))
		pdf_tree_checked = 1;
	    else if (off == (end + 1)) {
		pdf_tree_checked = 1;
		projNode *proj = pn;
		pdf_tree_fn_to_ln_internal (off, phys_name, ln, &proj);
	    }
	}
    }

    genString pdf_fn = phys_name;

    if (pdf_tree_checked == 0) {
	if (force_refresh_flag)
	    pn->fn_to_ln(phys_name, ln, FILE_TYPE, 0, 1); // force mapping for script
	else
	    pn->fn_to_ln(phys_name, ln, FILE_TYPE);

	if (ln.length()) {
	    if (force_refresh_flag)
		pn->ln_to_fn ((char const *)ln, pdf_fn, FILE_TYPE, 0, 1);  // force mapping for script
	    else
		pn->ln_to_fn ((char const *)ln, pdf_fn, FILE_TYPE);

	    if (pdf_fn.length() == 0) {
		msg(" ** ERR: Failed to convert ln back to fn") << eom;
		msg("         Phys name: $1") << phys_name << eom;
		msg("         ln name:   $1") << (char *)ln << eom;
		msg("         project:   $1") << pn->get_name() << eom;
		;

		ln = 0;
	    } else {
		if (strcmp((char *)pdf_fn, phys_name)) {
		    if (get_PDF_DebugLevel() > 0) {
			msg("PDF DEBUG: Phys name \"$1\"") << phys_name << eom;
			msg("           converted to PDF phys name \"$1\"") << (char *)pdf_fn << eom;
			msg("           ln: $1, project: $2") << (char *)ln << eoarg << pn->get_name() << eom;
			;
		    }

		    int idx               = -1;
		    projNode   *found_pn  = NULL;
		    projModule *found_mod = find_module_by_physname (pdf_fn, pn, idx, found_pn);
		    if (found_mod) {
			if (get_PDF_DebugLevel() > 0) {
			    if (strcmp(pn->get_name(), found_pn->get_name())) {
				msg("PDF DEBUG: Converted phys name \"$1\"") << (char *)pdf_fn << eom;
				msg("           from project: $1") << pn->get_name() << eom;
				msg("           is hidden by: $1, ln: $2") << found_pn->get_name() << eoarg << found_mod->get_name() << eom;
				msg("           original phys name: $1") << phys_name << eom;
				;
			    } else {
				msg("PDF DEBUG: Converted phys name \"$1\"") << (char *)pdf_fn << eom;
				msg("           is multiply specified in project: $1") << pn->get_name() << eom;
				msg("           original phys name: $1") << phys_name << eom;
				;
			    }
			}
			return;
		    }
		}
	    }
	}
    }

    if (!list_proj)
        proj_head->set_this_is_target(0);

    if (ln.length()) {
	if (force_refresh_flag) {
	    if (!found_old_dir(ln,DD_MODULE)) {
		projModule *mod = pn->raw_put_module(pdf_fn, ln.str(), proj_head->get_included_type());
		mod->update_module();
	    }
	} else
	    pn->raw_put_module(pdf_fn, ln.str(), proj_head->get_included_type());
    } else
        num_of_fail_get_log_name++;
}

void projModule::update_module()
{
    Initialize (projModule::update_module);
    projHeader *pr = this->get_header();
    Assert(pr);
    obj_insert (pr, REPLACE, this, this, NULL);
}

static char const *null_str = "NULL";

void projModule::put_signature(ostream& os) const
{
    Initialize(projModule::put_signature__ostream&__const);

    char const *nm = (ln_name.length()) ? (char const *)ln_name : null_str;
    os << "MODULE." << nm;
}

#define NULL_NAME "NULL"

void projModule::save_signature (ostream& os)
{
    Initialize(projModule::save_signature__ostream__const);
    // sets the fields' values
    char const *nm = this->paraset_file_name ();

    genString wrk;
    char const *left_name = NULL;
    if (imp_name.length() == 0) {
       projNode *pr = get_project ();
       if (pr && projfile_name)
	   pr->fn_to_ln_imp (projfile_name->get_name(), wrk);

       left_name = (char const *)wrk;
    } else
       left_name = (char const *)imp_name;

    if (!left_name || !*left_name)
	left_name = NULL_NAME;

    int lang = (int) commonTree::language;

    char const *tag = "m";
    if (included) 
	tag = "i";
    else if (lang == FILE_LANGUAGE_ELS    || 
	     lang == FILE_LANGUAGE_ESQL_C ||
	     lang == FILE_LANGUAGE_ESQL_CPP )   // boris 090398, all ELS and esql -> source files
	tag = "s";
    else if (projfile_name) {
	char const *phy_name = projfile_name->get_name();
	if (!xref_not_header_file(phy_name))
	    tag = "h";
	else if (!xref_not_src_file(phy_name))
	    tag = "s";
    }

    char sep = proj_save_get_separator();

    os << tag << sep << lang << sep << (char *)quote_str_if_space(left_name) << sep;

    struct OStype_stat stat_buf;
    if (projfile_name) {
	int was_valid = 0;
	(void) projfile_name->stat(&stat_buf, &was_valid);
	projfile_name->put_signature (os);
    } else
	os << NULL_NAME;

    os << sep << (char *)quote_str_if_space(ln_name) << sep;

    if (psetfile_name) {
	int was_valid = 0;
	(void) psetfile_name->stat(&stat_buf, &was_valid);
	psetfile_name->put_signature (os);
    } else
	os << NULL_NAME;
}


void projModule::print (ostream& st, int level) const
{
    Initialize(projModule::print__ostream&_int__const);
    gen_print_indent(st, level);
    char const *ver = (cur_ver.length()) ? (char const *)cur_ver : null_str;

    char const *log_name = (ln_name.length()) ? (char const *)ln_name : null_str;

    char const *prj_name = get_pr_name();
    if (!prj_name || !prj_name[0])
	prj_name = null_str;

    char const *app_name = (app_ln.length()) ? (char const *)app_ln : null_str;

    st << "app_head= " << (void *)app_head << ", cur_ver= " << ver << ", target_proj= "
	<< (void *)target_proj << "\n";

    gen_print_indent(st, level);
    st << "ln_name= " << log_name << "\n";
    gen_print_indent(st, level);
    st << "pr_name= " << prj_name << "\n";
    gen_print_indent(st, level);
    st << "app_ln=  " << app_name << "\n";
}

char const *projModule::get_name () const
{
    char const *ln = (char const *)ln_name;
    return ln;
}

void projModule::set_sname (char const *pname)
{
    char const *old_name = get_pr_name ();
    set_pr_name (pname);
}


projNode * projModule::get_project () const
{
   projNode * proj = get_project_internal ();
   for (; proj && !proj->get_xref (1); proj = proj->find_parent ());
   return proj;
}

projNode * projModule::get_project_internal () const
{
  Initialize(projModule::get_project_internal);
   appTree* contents = module_get_proj(this);
   if (!contents)
       Assert(contents);
   projNode * proj = checked_cast(projNode, contents->get_parent());
   return proj;
}


Hierarchical *projModule::get_header_virtual () const
{
    Initialize(projModule::get_header_virtual__const);
    projNode *proj_nd = get_project_internal();
    Assert(proj_nd);
    app *pr = proj_nd->get_header();
    return pr;
}

projHeader *projModule::get_header () const
{
    Initialize(projModule::get_header__const);
    projHeader *pr = checked_cast(projHeader,get_header_virtual ());
    return pr;
}


ddKind projModule::get_symbolPtr_kind() const { return DD_MODULE; }
int projModule::is_symbolPtr_def () const { return 1; }
fileLanguage projModule::language () const {return (fileLanguage) commonTree::language;}

void projModule::set_pr_name (char const *new_nm)
{
    if (new_nm && new_nm[0]) {
	char const *old_nm = (projfile_name) ? projfile_name->get_name() : NULL;
	if (!old_nm || strcmp(new_nm,old_nm) != 0)
	    projfile_name = global_cache.add_entry (new_nm);
    } else
        projfile_name = NULL;
}

symbolPtr projModule::get_def_file_from_proj (projNode *pr)
{
    Initialize(projModule::get_def_fiel_from_proj__projNode*);
    if (pr == NULL)
	pr = get_project ();

    symbolPtr sym = NULL_symbolPtr;
    if (pr) {
	sym = pr->lookup_symbol(DD_MODULE, (char const *)ln_name);
    }
    return sym;
}

int projModule::construct_symbolPtr(symbolPtr& sym) const
{
    int retval = 0;
    projNode *project = get_project();
    sym = ((projModule*)this)->get_def_file_from_proj(project);

    if (sym.xrisnotnull())
	retval = 1;

    return retval;
}

char const *projModule::get_pr_name () const
{
    char const *nm = NULL;
    nm = (projfile_name) ? projfile_name->get_name() : NULL;
    return nm;
}

char const * projModule::get_phys_filename()
{
    Initialize(projModule::get_phys_filename);
    char const *nm = get_pr_name();
    if (!nm) {
	projNode* proj_node = get_project_internal();
	if (proj_node) {
	  genString pn;
	  proj_node->ln_to_fn ((char const *)ln_name, pn);
	  Assert(pn.str());
	  projfile_name = global_cache.add_entry((char const *)pn);
	}
    nm = get_pr_name();
    }
    return nm;
}


projNode* projHeader::get_pmod_project_from_ln (char const *ln)
{
    Initialize(projHeader::get_pmod_project_from_ln);

    if (!ln) return NULL;
    appPtr ah = projModule::get_app(ln);
    if (ah) {
      projNode * pr = app_get_proj(ah);
      return (pr)?pr->get_pmod_project_from_ln(ln):0;
    }
    genString real_ln;
    projNode* project = projNode::get_current_proj();
    if (project) {
        project->fn_to_ln (ln, real_ln);
	if (real_ln.length())
	    ln = real_ln;
    }
    projModule *module = NULL;
    projNode * home_project = projNode::get_home_proj();
    if (home_project) {
      module = home_project->find_module(ln);  // want to expand home
      if (module)
	return home_project->get_pmod_project_from_ln(ln);
    }

    // should search thru all atria project first
    projNode *proj;
    int i;
    for (i=0; proj = projList::search_list->get_proj(i); i++) {
      if ((proj != home_project) && (module = proj->search_module_hash(ln))) {
	return proj->get_pmod_project_from_ln(ln);	
      }
    }

    // what happens to RCS (not atria project)
    for (i=0; proj = projList::search_list->get_proj(i); i++) {
      if ((proj != home_project) && !proj->is_expand_all_level()) {
	genString fn;
	proj->ln_to_fn(ln, fn);
	if (fn.str()) { // check for existence of file
	  return proj->get_pmod_project_from_ln(ln);	
	}
      }
    }

    return 0;
}


void proj_get_modules(projNode *proj, objArr& modules, int depth) 
// a depth of 0 will signify infinite depth
{
 Initialize(proj_get_modules);
 
  mpb_mpb_segment(1);         //increment progress bar
 
  proj->refresh();

  symbolArr& proj_contents = proj->rtl_contents();
  symbolPtr sym;
  ForEachS(sym, proj_contents){
    switch (sym.get_kind()) {
      
    case DD_PROJECT:
      {
	if (depth != 1) 
	{
	  appPtr app_head = sym.get_def_app();
	  if(app_head && is_projHeader(app_head)){
	    projNode *child_proj = projNodePtr(app_head->get_root());
	    proj_get_modules(child_proj, modules, depth-1 );
          }
        }
      }
      break;

    case DD_MODULE:
      {
	projModule *module = checked_cast(projModule,sym);
	if (module)
	  modules.insert_last(module);
      }
      break;
    }
  }
}

void proj_get_modules(projNode *proj, objArr& modules)
{
    proj_get_modules(proj, modules, 0);
}

char const *get_paraset_file_name(char const *src_file, projNode *pr);

// rewrites the memory on each next call
// use first the subproject pset rule and if failed the global level pset rule
char const *get_paraset_file_name(char const *src_file, projModule *mod)
{
    Initialize(get_paraset_file_name__const_char *_projModule*_genSring&);
    char const *pname = 0;
    projNode *pr = (mod) ? mod->get_project () : NULL;
    pname = get_paraset_file_name (src_file, pr);
    return pname;
}

char const *projModule::paraset_file_name ()
// return the pset file name. Rewrite the memory on each call
{
    Initialize (projModule::paraset_file_name);
    char const *pset_nm = NULL;
    if (psetfile_name) {
      pset_nm = psetfile_name->get_name();
    } else {
      char const *fn = get_phys_filename();
      pset_nm = OSPATH(get_paraset_file_name(fn, this));
      set_psetfile_name (pset_nm);
    }

    return pset_nm;
}
    
void projModule::set_psetfile_name (char const *new_nm)
{
    if (new_nm && new_nm[0]) {
	char const *old_nm = (psetfile_name) ? psetfile_name->get_name() : NULL;
	if (!old_nm || strcmp(new_nm,old_nm) != 0)
	    psetfile_name = global_cache.add_entry (new_nm);
    } else
        psetfile_name = NULL;
}


char const *projModule::get_implicit_name () const
{
      return (char const *)imp_name;
}

void projModule::get_phys_filename (genString&pn){pn = get_phys_filename();}
