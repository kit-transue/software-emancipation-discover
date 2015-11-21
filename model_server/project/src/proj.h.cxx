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
//  file proj.h.C
//  implementation for projNode

#undef MAX

#ifdef _WIN32
#include <limits.h>
#ifndef ISO_CPP_HEADERS
#endif /* ISO_CPP_HEADERS */
#include <fcntl.h>
#else
#include <sys/param.h>
#endif

#ifndef ISO_CPP_HEADERS
#include <errno.h>
#else /* ISO_CPP_HEADERS */
#include <cerrno>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */

#ifdef _WIN32
#include <winsock.h>
#include <io.h>
#endif

#include <fcntl.h>

#include <msg.h>
#include <messages.h>
#include <genTmpfile.h>
#include <customize.h>
#include <fileCache.h>

#include <objRelation.h>
#include <proj.h>
#include <xref.h>
#include <path.h>

#ifndef _pathdef_h
#include <pathdef.h>
#endif

#include <RTL.h>
#include <RTL_externs.h>
#include <loadedFiles.h>
#include <ddKind.h>

#include <shell_calls.h>

#include <machdep.h>
#include <db_intern.h>
#include <pdf_tree.h>

#include <XrefTable.h>
#include <transaction.h>
#include <psetmem.h>
#include <fileEntry.h>
#include <pathcheck.h>

#include <scopeMgr.h>
#include <driver_mode.h>

static int PDF_DebugLevel = customize::getIntPref("DIS_internal.PDF_DebugLevel");

static char const *NULL_NAME = "NULL";

init_rel_or_ptr(appTree,proj,0,appTree,module,relationMode::C);
init_rel_or_ptr(appTree,proj,0,appTree,sub,relationMode::C);

loadedFiles * create_loadedFiles(char*);

#ifndef F_OK
#define F_OK            0       /* does file exist */
#define X_OK            1       /* is it executable by caller */
#define W_OK            2       /* is it writable by caller */
#define R_OK            4       /* is it readable by caller */
#endif

// EXTERNAL DATA
extern bool suppress_Xref;    // flag set by proj_make_project() in main.C
// EXTERNAL FUNCTIONS
void get_pmod_projs_from(objArr &, projNode *);

class smtHeader;

extern int smt_reset_strings();

void gen_print_indent(ostream&, int);
//char const * customize_make ();
const int  customize_get_write_flag();
char const * customize_install_root();
// VARIABLE DEFINITIONS

static char const LN_META[] = "/";

init_rel_or_ptr(projNode,parentProject,0,projNode,childProject,0);

init_relational(projHeader,RTL);
init_relational(projNode,RTLNode);
init_relational(projContents,appTree);

projNode* projNode::control_proj;
projNode* projNode::curr_root_proj;
projNode* projNode::curr_home_proj;
RTL*      projNode::projects;
loadedFiles* projNode::loaded_files = 0;
loadedFiles* projNode::unparsed_files;

// phy_path: physical path
// lang is used only when create_new_file is true
// create_new_file = 1: if it creates new file; otherwise, it is false;

/* static */
int projNode::create_global_script_modules (char const *phy_path, 
                                            fileLanguage lang, 
                                            int create_new_file) {

    Initialize(projNode::create_global_script_modules);
    projNode *proj,*top_proj;
    projNode *home_proj = projNode::get_home_proj();	// same as projList::search_list->get_proj(0)

    genString ln;

    for (int i=1; top_proj = projList::search_list->get_proj(i); i++)
    {
	// if home_proj is a child of top_proj, proj = home_proj, else proj = top_proj
	for (proj = home_proj; proj; proj = proj->find_parent()){
	    if (proj == top_proj){
		proj = home_proj;
		break;
	    }
	}
	if (proj == NULL) proj = top_proj;
   
	proj->fn_to_ln(phy_path,ln,FILE_TYPE,0,1);
        if (ln.length())
        {
            projModule* module;
            if (proj->is_script())
            {
                module = proj->create_script_module(ln);
                // Set the language
                if (create_new_file && module)
                    module->language(lang);
            }
            else
            {
                genString proj_name;
                proj->fn_to_ln_imp(phy_path, proj_name);
                projNode *par = proj->find_project(proj_name);
                if (!par)
                {
                     msg("No mapping for $1 in project $2", error_sev) << proj_name.str() << eoarg << proj->get_name() << eom;
                     return 0;
                }
                module = par->put_module(phy_path, ln);
            }
            // Update the RTL
            if (module)
                module->update_module();
        }       
    }

    if (create_new_file)
	scope_mgr.report_cm_command (CM_NEW_FILE, proj, (projNode *)NULL, (char const *)ln);

    return 1;
}           
   
int projNode::construct_symbolPtr(symbolPtr& sym) const
{
    int retval = 0;

// boris: The project xrefSymbol exists in the control project only (so far)
    projNode *cont = projNode::get_control_project ();
    if (cont)
        sym = cont->lookup_symbol (DD_PROJECT, get_name());
    if (sym.xrisnotnull())
	retval = 1;
    
     return retval;
}

app *projNode::get_symbolPtr_def_app ()
{
     Initialize(*projNode::get_symbolPtr_def_app);
     projHeader *prh = checked_cast(projHeader,get_header());
     return prh;
}

void projHeader::remove_obj(objRemover *, objRemover *)
{}
  
void projContents::print(ostream& os, int level) const
{
 gen_print_indent(os, level);

 Obj * mods = proj_get_modules((appTree *)this); 
 Obj * subs = proj_get_subs((appTree *)this);

 int no_mod = mods ? mods->size() : 0;
 int no_sub = subs ? subs->size() : 0;

 os << " contents: " << 
        no_mod << " moduli " << no_sub << " subprojects" << endl;
}

void projHeader::print(ostream& os, int level) const
{
    gen_print_indent(os, level);
    os << get_name() << endl;
}

bool projNode::is_paraset_project()
{
    Initialize(projNode::is_paraset_project);

    if (strcmp(get_ln(), "/ParaDOCS") == 0 ||
        strcmp(get_ln(), "/__ParaSPECIAL") == 0 ||
        strcmp(get_ln(), "/tmp") == 0)
        return TRUE;
    return FALSE;
}

void projNode::insert_dir_hash(char const *dir_path)
{
    Initialize(projNode::insert_dir_hash);

    if (dir_path == 0)
        return;

    int idx = path_hash(dir_path, MAX_MODULE_HASH);

    DBG
    {
        msg("INSERT_DIR_PATH>>>>>") << eom;
        msg("idx = $1 path = ($2)") << idx << eoarg << dir_path << eom;
        ;
    }

    module_hash_entry *cur = module_hash_h[idx];
    module_hash_entry *last = cur;
    while(cur)
    {
        if (cur->get_dir_path() && strcmp(dir_path,cur->get_dir_path()) == 0)
            return;
        last = cur;
        cur = cur->get_next();
    }

    module_hash_entry *nn = db_new(module_hash_entry, (dir_path));
    if (last)
        last->set_next(nn);
    else
        module_hash_h[idx] = nn;
}

void projNode::insert_module_hash(projModule *mod)
{
    Initialize(projNode::insert_module_hash);

    if (module_hash_h == 0)
        module_hash_h = (module_hash_entry **)calloc(sizeof(module_hash_entry *), MAX_MODULE_HASH);

    genString nn = mod->get_name();
    int idx = path_hash(nn, MAX_MODULE_HASH);

    DBG
    {
        msg("INSERT_MODULE>>>>>") << eom;
        msg("idx = $1 m = $2 n = ($3)") << idx << eoarg << (int)mod << eoarg << nn.str() << eom;
        ;
    }

    module_hash_entry *cur = module_hash_h[idx];
    module_hash_entry *last = cur;
    while(cur)
    {
        if (mod == cur->get_module())
            return;
        last = cur;
        cur = cur->get_next();
    }

    module_hash_entry *n0 = db_new(module_hash_entry, (mod));
    if (last)
        last->set_next(n0);
    else
        module_hash_h[idx] = n0;

    char *n = strrchr((char *)nn,'/');
    if (n)
    {
        *n = '\0';
        insert_dir_hash(nn);
    }
}

void projNode::remove_module_hash(projModule *m)
{
    Initialize(projNode::remove_module_hash);

    Assert(module_hash_h);

    char const *n = m->get_name();
    int idx = path_hash(n, MAX_MODULE_HASH);

    DBG
    {
        msg("REMOVE_MODULE<<<<<") << eom;
        msg("idx = $1 m = $2 n = ($3)") << idx << eoarg << (int)m << eoarg << n << eom;
        ;
    }


    module_hash_entry *cur = module_hash_h[idx];
    module_hash_entry *last = cur;

    Assert(cur);

    while(cur)
    {
        if (m == cur->get_module())
        {
            if (last == cur)
                module_hash_h[idx] = cur->get_next();
            else
                last->set_next(cur->get_next());
            cur->set_next(0);
            delete(cur);
            return;
        }
        last = cur;
        cur = cur->get_next();
    }

    Assert(0);
}

projModule *projNode::search_module_hash(char const *nn)
{
    Initialize(projNode::search_module_hash);

    if (nn == 0)
	return 0;

    DBG
    {
        msg("SEARCH_MODULE>>>>>") << eom;
        msg("n = ($1)") << nn << eom;
        ;
    }

    if (module_hash_h == 0)
        return 0;

    int idx = path_hash(nn, MAX_MODULE_HASH);
    module_hash_entry *cur = module_hash_h[idx];
    if (cur == 0)
        return 0;

    DBG
    {
        msg("idx = $1") << idx << eom;
        ;
    }

    while(cur)
    {
        projModule *mod = cur->get_module();

        if (mod) {
	    char const *nm = mod->get_name();

	    if (nm && strcmp(nn, nm) == 0) {
		DBG
		{
		    msg("m = $1") << (int)mod << eom;
		    ;
		}

		return mod;
	    }
        }
        cur = cur->get_next();
    }
    return 0;
}

void projNode::pdf_pr_one_name(char const *node_name, int deep_count, char file_type)
// node_name is project name if file_type is DIR_TYPE
//              module  name if file_type is FILE_TYPE
//
// deep_count is used to line up: One level of project will shift the output two
//      spaces to right
//
// file_type: DIR_TYPE
//            FILE_TYPE
{
    Initialize(pdf_pr_one_name);

    for(int i = 0;i < deep_count; i++)
        msg("  ") << eom;
    genString proj_n;
    genString phy_n;
    genString logic_n;

    if (file_type == FILE_TYPE)
    {
        logic_n = node_name;
	projModule *mod = 0;
        projNode *prnt = this;
	while ( prnt && !prnt->get_map() )
	    prnt= prnt->find_parent();
	if ( prnt )
	    mod= prnt->search_module_hash(logic_n);
	if ( mod )
	{
	    phy_n= mod->get_phys_filename();
	    proj_n= mod->get_implicit_name();
	}
	else
	{
	    ln_to_fn(logic_n,phy_n,file_type);
	    fn_to_ln_imp(phy_n,proj_n,file_type);
	}
    }
    else
    {
        proj_n = node_name;
        ln_to_fn_imp(proj_n,phy_n,file_type);
        fn_to_ln(phy_n,logic_n,file_type);
    }

    genString src_n;
    genString pname;
    if (file_type == FILE_TYPE)
    {
        src_n.printf("%s/%%/.pset",phy_n.str());
        projHeader::fn_to_ln_special_rule(src_n,pname,this);
        if (pname.length() && *pname.str() != '/')
        {
            genString phy;
            ln_to_fn_imp(get_ln(), phy, DIR_TYPE, 0, 1);
            if (phy.length())
            {
                genString s;
                s.printf("%s/%s",phy.str(),pname.str());
                pname = s.str();
            }
        }
    }
    else
        xref_file_name(pname);

    if (proj_n.length() == 0)
        proj_n = NULL_NAME;
    if (phy_n.length() == 0)
        phy_n = NULL_NAME;
    if (logic_n.length() == 0)
        logic_n = NULL_NAME;
    if (pname.length() == 0)
        pname.printf("%s.pset",phy_n.str());

    if (file_type == FILE_TYPE)
        msg("MODULE:  ($1)($2)($3)($4)") << proj_n.str() << eoarg << phy_n.str() << eoarg << logic_n.str() << eoarg << pname.str() << eom;
    else
        msg("PROJECT: ($1)($2)($3)($4)") << proj_n.str() << eoarg << phy_n.str() << eoarg << logic_n.str() << eoarg << pname.str() << eom;
}

int projNode::pdf_prt(char const *proj_name, int deep_count)
// print the whole tree from proj_name down to the leafs (modules)
// deep_count is zero. it will be increased by one when this routine is recursively
//     called for its child projects. see the above routine: pdf_pr_one_name
{
    Initialize(projNode::pdf_prt);

    refresh();
    int found = 0;

    int pr_flag = 0;
    if (strstr(get_ln(),proj_name))
        pr_flag = 1;

    if (pr_flag)
    {
        pdf_pr_one_name(get_ln(), deep_count, DIR_TYPE);
        found = 1;
    }

    symbolArr& proj_contents = rtl_contents();
    symbolPtr sym;
    ForEachS(sym, proj_contents)
    {
        if (sym.get_kind() == DD_MODULE)
        {
            if (pr_flag)
            {
                pdf_pr_one_name(sym.get_name(), deep_count+1, FILE_TYPE);
                found = 1;
            }
        }
        else if (sym.get_kind() == DD_PROJECT)
        {
            appPtr app_head = sym.get_def_app();
            if(app_head && is_projHeader(app_head))
            {
                projNode *child_proj = projNodePtr(app_head->get_root());
                int new_deep_count = deep_count;
                if (pr_flag)
                    new_deep_count = deep_count + 1;

                if (child_proj->pdf_prt(proj_name, new_deep_count))
                    found = 1;
            }
        }
    }
    return found;
}

int projNode::search_dir_hash(char const *dir_path)
{
    Initialize(projNode::search_dir_hash);

    DBG
    {
        msg("SEARCH_DIR>>>>>") << eom;
        msg("n = ($1)") << dir_path << eom;
        ;
    }

    if (module_hash_h == 0)
        return 0;

    if (dir_path == 0)
        return 0;

    char const *p = strrchr(dir_path,'/');
    if (!p)
        return 0;

    int idx = path_hash(dir_path, MAX_MODULE_HASH);

    DBG
    {
        msg("idx = $1") << idx << eom;
        ;
    }

    module_hash_entry *cur = module_hash_h[idx];
    if (cur == 0)
        return 0;

    while(cur)
    {
        if (cur->get_dir_path() && strcmp(dir_path,cur->get_dir_path()) == 0)
            return 1;

        cur = cur->get_next();
    }
    return 0;
}

void projNode::print(ostream& os, int level) const
{
 gen_print_indent(os, level);

 os << get_name() ;

 RTLNode::print(os, level);
}

projHeader::projHeader(char const *name, char const * /*dir*/) : RTL(name)
{
    Initialize(projHeader::projHeader);

    projNode *cont = projNode::get_control_project ();
    if (cont) {
	Xref *Xr         = cont->get_xref();
	XrefTable *Xrt   = (Xr) ? Xr->get_lxref() : NULL;
	xrefSymbol *xsym = (Xrt) ? Xrt->insert_symbol (this) : NULL;
    }
}


void projNode::rtl_insert(symbolPtr symbol, int check_include)
{
    Initialize (projNode::rtl_insert);

    if ( symbol.isnotnull() ){
        // check for the match
        if(!check_include || !rtl_includes(symbol)) {
	    contents.insert_last (symbol);

            if(!symbol.is_xrefSymbol ())
	        put_relation(app_of_rtl, get_header(), RelationalPtr(symbol));
        }
    }
}

void projNode::update_projnode_for_delete()
{
    Initialize(projNode::update_projnode_for_delete);
}

void projNode::update_projnode()
{
    Initialize(projNode::update_projnode);

    if (find_parent())
	obj_insert (find_parent()->get_header(), REPLACE, this, this, NULL);
}

// Remove an object from the RTL and regenerate views

void projNode::rtl_remove (symbolPtr symbol)
{
    Initialize (projNode::rtl_remove);

    contents.remove (symbol);
    if (symbol.relationalp())
	obj_delete(ObjPtr(symbol));
    if ( is_projNode(this) )
         obj_insert( get_header(), REPLACE, this, this, NULL);
    
    selection_remove (symbol);
}


projNode::projNode(char const *ln, projHeader *header, char const * /*dir*/ , projMap* map,
    int open_xref, int pmod_tp)
 : RTLNode(ln), name(ln), pm(map), xref(0), has_xref(open_xref), 
   has_progs_flag(1), subproj_flag(3), subproj_pmod_flag(3),
   module_hash_h(0), read_only_flag(0), write_only_flag(0),
   dummyproj(0), pr_name(0), sym_name(0), link_name(0), ind_name(0), prj_name(0), pmod_type(pmod_tp)
{
    Initialize(projNode::projNode);
    if (map) pmod_type = map->get_pmod_type();

    appTreePtr old_root = checked_cast(appTree,header->get_root());
    header->put_root(this);
    obj_delete (old_root);
    no_refresh_files = no_refresh_dirs = 0;

    if (projNode::control_proj) {
	Xref *Xr         = projNode::control_proj->get_xref();
	XrefTable *Xrt   = (Xr) ? Xr->get_lxref() : NULL;
	xrefSymbol *xsym = (Xrt) ? Xrt->insert_symbol (this) : NULL;
    }

    projContents * con = db_new(projContents,());
    this->put_first(con);
}

projNode::~projNode()
{
    Initialize(projNode::~projNode);
    if (pr_name)
	remove_projNode_from_hashes (this);
}

void projNode::delete_projnode()
// It will recursively delete a tree of projNode and the leaves (projModules)
{
    Initialize(projNode::delete_projNode);

    if (get_map())
    {
        start_transaction()
        {
            delete_xref();
            // next statement is copied from Xref::xref_notifier_report_proj()
            xref_notifier_apply(); // fake an "end transaction" to flush more often
        }  end_transaction();
    }

    start_transaction()
    {
        symbolArr proj_contents = rtl_contents();
        symbolPtr sym;
        ForEachS(sym, proj_contents)
        {
            if (sym.get_kind() == DD_PROJECT)
            {
                appPtr app_head = sym.get_def_app();
                projNode *p = projNodePtr(app_head->get_root());
                p->delete_projnode();
            }
        }

        projNode *par = find_parent();
        if (par)
        {
            symbolArr rtl_cts = par->rtl_contents();
            symbolPtr sym;
            ForEachS(sym, rtl_cts)
            {
                if (sym.get_kind() == DD_PROJECT)
                {
                    appPtr app_head = sym.get_def_app();
                    projNode *p = projNodePtr(app_head->get_root());
                    if (p == this)
                    {
                        par->rtl_remove(sym);
                        break;
                    }
                }
            }
        }
        update_projnode_for_delete();
        obj_delete(get_header());

        if (module_hash_h)
            delete module_hash_h;
    }  end_transaction();
}

void projNode::set_writable_bit(int flag)
{
    Initialize(projNode::set_writable_bit);

    if (flag == R_FLAG)
        read_only_flag = 1;
    else
        write_only_flag = 1;
}

void projNode::update_path_hash_tables(projMap *pmap)
{
    Initialize(projNode::update_path_hash_tables);

    set_visited();
    char const *proj_n = get_ln();
    genString fn;
    genString ln;
    ln_to_fn_imp(proj_n, fn, DIR_TYPE);
    fn_to_ln(fn, ln, DIR_TYPE);
    pmap->insert_hash(fn,     proj_n, ln,     DIR_TYPE, LOGIC_NAME_RULE,   FNTOLN);
    pmap->insert_hash(fn,     proj_n, proj_n, DIR_TYPE, PROJECT_NAME_RULE, FNTOLN);
    pmap->insert_hash(ln,     proj_n, fn,     DIR_TYPE, LOGIC_NAME_RULE,   LNTOFN);
    pmap->insert_hash(proj_n, proj_n, fn,     DIR_TYPE, PROJECT_NAME_RULE, LNTOFN);
}

void projNode::update_all_hash_tables(projMap *pmap)
{
    Initialize(projNode::update_all_hash_tables);

    update_path_hash_tables(pmap);
    symbolArr& proj_contents = rtl_contents();
    symbolPtr sym;
    ForEachS(sym, proj_contents)
    {
        if (sym.get_kind() == DD_MODULE)
        {
            projModule *mod = checked_cast(projModule,sym);
            mod->update_path_hash_tables(pmap, 0);
        }
        else if (sym.get_kind() == DD_PROJECT)
        {
            appPtr app_head = sym.get_def_app();
            if(app_head && is_projHeader(app_head))
            {
                projNode *child_proj = projNodePtr(app_head->get_root());
                child_proj->update_all_hash_tables(pmap);
            }
        }
    }
}

bool projNode::is_writable()
{
    Initialize(projNode::is_writable);

    if (read_only_flag)
        return FALSE;
    if (write_only_flag)
        return TRUE;

    if (is_model_server() && this != projNode::control_proj) 
      return FALSE;

    projMap *pmap = get_projmap_for_projnode(this);
    while(pmap)
    {
        if (pmap->is_read_only())
        {
            set_writable_bit(R_FLAG);
            return FALSE;
        }
        if (pmap->is_writable())
        {
            set_writable_bit(W_FLAG);
            return TRUE;
        }
        pmap = pmap->get_parent_project();
    }
    set_writable_bit(W_FLAG);
    return TRUE;
}     

// this function removes any leading and trailing slash ('/') from a
// string and translates any embedded slashes to dots ('.')
// the result is appended to the supplied genstring. This is useful for
// forming the name of an xref file from a project path.

static void projname_to_xref_basename(char *proj_name, genString &translated_proj_name)
{
    Initialize(projname_to_xref_filename);

    if (proj_name) {
        char *duplicate = new char[strlen(proj_name)+1];
        if (duplicate) {
            strcpy(duplicate, proj_name); // do translation in a copy

            char *p = duplicate;

            if (*p == '/') p++;           // skip over initial slash

            char *pp = p;                 // remember start of interesting part


            // translate / to . and remove a terminal .

            while (p && *p) {
                if (*p == '/') *p = '.';
                p++;
            }
            if (*(p-1) == '.') *(p-1) = '\0';

            translated_proj_name += pp;   // translated part of duplicate

            delete[] duplicate;
        }
    }

    return;
}

void projNode::xref_file_name (genString &xref_name)
{
    Initialize (projNode::xref_file_name);

    genString filename;
		char*			ptmpName;

    ln_to_fn_imp (name, filename, DIR_TYPE);
    int len = filename.length();
    if (len) {
	if (((char *)filename)[len-1] != '/')
	    filename += '/';
        if (name.not_null()) {
            projname_to_xref_basename(name, filename);
            filename += ".pmod";
        } else {
            filename += "PARA.pmod"; // default (was "global.xref"
        }
    }
		else
		{
			create_path_2DIS( OSapi_getenv("TMPDIR"), &ptmpName );
			filename = ptmpName;
			OSapi_free( ptmpName );
		}

    genString pmod_name;
    genString mapped_pmod;

    pmod_name.printf ("%s/%%/.pmoddir", (char*)filename);

    projNode *tmpNode = this; // copy this to tmpNode ,because next call might change it
    projHeader::fn_to_ln_special_rule (pmod_name, mapped_pmod, PMODDIR_RULE, tmpNode);
                      
    if (mapped_pmod.length() > 0) {
	if (is_writable()) {
	    int err=projHeader::make_path (mapped_pmod);
	    if (err) {
		msg("Cannot make directory '$1' for pmod.", error_sev) << (char*)mapped_pmod << eom;
		xref_name = "";
	    } else {
		xref_name = mapped_pmod;
	    }
	} else  // if not_writable...
	    xref_name = mapped_pmod;
    } else
        xref_name = filename;
}

void projNode::ln_to_project_name(char const *ln, genString &proj_name)
{
  ln_to_project_name(ln, proj_name, 1 );
}
void projNode::ln_to_project_name(char const *ln, genString &proj_name, int ln_tp )
{
    Initialize(projNode::ln_to_project_name);

    projNode *cur = this;

    while (cur && !cur->pm)
        cur = cur->find_parent();

    if (cur)
        cur->pm->ln_to_proj_name(ln, get_ln(), proj_name, ln_tp);
    else 
        proj_name = 0;
}

/* static */
projNode* projNode::find_project (const Xref* the_xref)
{
    projNode* proj;

    for (int i=1; proj = projList::search_list->get_proj(i); i++)
	if (proj->get_xref() == the_xref)
	    return proj;

    return NULL;
}

/* static */
projNode* projNode::find_project (char const *proj_name )
{
    return find_project(proj_name, 0 );
}

/* static */
projNode* projNode::find_project (char const *proj_name, char const *logic_name, int ln_tp )
{
// ln_tp=0 - do not cut off the last portion of proj_name if the rule contains "**"
// ln_tp=1 - cut of this portion if the rule contains "**" and try find a project

    Initialize (projNode::find_project);

    projNode *projnode_root = projHeader::ln_to_projNode(proj_name);

    if (projnode_root == 0)
        return 0;

    genString new_proj_name;

    projnode_root->ln_to_project_name((char*)proj_name, new_proj_name, ln_tp );

    if (new_proj_name.length() == 0)
        return 0;

    if (new_proj_name == projnode_root->get_ln())
        return projnode_root;

    return projnode_root->search_project(new_proj_name, (char *)logic_name);
}

projNode* projNode::find_sub_project(char const* proj_name)
{
  Initialize(projNode::find_sub_project);
  genString new_proj_name;

  ln_to_project_name((char*)proj_name, new_proj_name);

  if (new_proj_name.length() == 0)
    return 0;

  if (new_proj_name == get_ln())
    return this;

  return search_project(new_proj_name, (char *)0);
  
}

projNode* projNode::get_pmod_project_from_ln(char const* ln)
/*
  fn: physical file name
  this: root project of fn
  will expand up to project that contain pmod for this fn
*/

{
  Initialize(projNode::get_pmod_project_from_ln);

  genString fn_proj_name;
  genString fn;
  ln_to_fn(ln, fn);
  if (fn.str() == 0) return 0;
  fn_to_ln_imp(fn.str(), fn_proj_name); // return project name for fn;

  if (fn_proj_name.length() == 0)
    return 0;

  if (fn_proj_name == get_ln())
    return this;

  return get_pmod_project_from_proj_name(fn_proj_name);
  
}


projNode *projNode::get_pmod_project_from_proj_name(char const *proj_name)
{
    Initialize (projNode::get_pmod_project_from_proj_name);

    Xref * Xr = get_xref();
    int go_down = (Xr == 0 || Xr->is_unreal_file());
    if (!go_down) {
      projNode * pmod_proj = this;
      while (pmod_proj && !pmod_proj->get_xref(1))
	pmod_proj = pmod_proj->find_parent();
      return pmod_proj;
    }

    if (is_projNode(this)) // should we stop after a fixed depth
        refresh_projects();

    symbolArr& proj_contents = rtl_contents();
    symbolPtr sym;
    ForEachS(sym, proj_contents)
    {
        char const *qq = sym.get_name();
        int len = strlen(qq);
        if (sym.get_kind() == DD_PROJECT)
        {
            if (strncmp(proj_name, qq, len) == 0)
            {
                appPtr app_head = sym.get_def_app();
                if(app_head && is_projHeader(app_head))
                {
                    projNode *child_proj = projNodePtr(app_head->get_root());
                    if (strlen(proj_name) == len)
                        return(child_proj);
		    if (*(proj_name+len) == '/')
		    {
                        child_proj = child_proj->get_pmod_project_from_proj_name(proj_name);
                        if (child_proj)
                            return child_proj;
		    }
                }
            }
        }
    }

    // The following testing could be put at the beginning of this routines
    // but it will slow down for the successful search which alwys happens

    if (is_implicit_proj())
    {
        char const *nm = get_ln();
        int len = strlen(nm);
        if (strncmp(proj_name,nm,len) == 0 && (*(proj_name+len) == '/' || *(proj_name+len) == '\0'))
            return this;
    }
    return NULL;
}


projNode *projNode::search_project(char const *proj_name, char const *ln)
// search the closet projNode for the proj_name
// if it is a module, ln might be its logic name or 0.
// if ln is 0, try to get its logic name from proj_name first
{
    Initialize (projNode::search_project);

    if (is_projNode(this))
        refresh_projects();

    genString sln;
    if (!ln)
    {
        // If proj_name is a module, try to get its logic name
        genString fn;
        ln_to_fn_imp(proj_name,fn);
        if (fn.length())
        {
            fn_to_ln(fn,sln);
            ln = sln;
        }
    }
    symbolArr& proj_contents = rtl_contents();
    symbolPtr sym;
    ForEachS(sym, proj_contents)
    {
        char const *qq = sym.get_name();
        int len = strlen(qq);
        if (sym.get_kind() == DD_MODULE)
        {
            if (ln)
            {
                if (strcmp(ln, qq) == 0)
                    return this;
            }
        }
        else if (sym.get_kind() == DD_PROJECT)
        {
            if (strncmp(proj_name, qq, len) == 0)
            {
                appPtr app_head = sym.get_def_app();
                if(app_head && is_projHeader(app_head))
                {
                    projNode *child_proj = projNodePtr(app_head->get_root());
                    if (strlen(proj_name) == len)
                        return(child_proj);
		    if (*(proj_name+len) == '/')
		    {
                        child_proj = child_proj->search_project(proj_name, ln);
                        if (child_proj)
                            return child_proj;
                    }
                }
            }
        }
    }

    // The following testing could be put at the beginning of this routines
    // but it will slow down for the successful search which alwys happens

    if (is_implicit_proj())
    {
        char const *nm = get_ln();
        int len = strlen(nm);
        if (strncmp(proj_name,nm,len) == 0 && (*(proj_name+len) == '/' || *(proj_name+len) == '\0'))
            return this;
    }
    return NULL;
}

projModule* projNode::find_module(char const* nm)
{
  if (nm==NULL || strlen(nm)==0) return NULL;
  Initialize(projNode::find_module(char const*));
  genString fn;
  return find_module(nm, fn);
}
int projNode::is_expand_all_level()
{
  Initialize(projNode::is_expand_all_level);
  if (!no_refresh_files || !no_refresh_dirs)
    return 0;
  Obj *children = parentProject_get_childProjects(this);
  Obj *el;
  ForEach (el, *children) {
    projNode *child = checked_cast (projNode, el);
    int ret_val = child->is_expand_all_level();
    if (ret_val == 0) return ret_val;
  }
  return 1;

}

projModule* projNode::find_module (char const* nm, genString &fn)
{
    Initialize (projNode::find_module(char const*, genString&));
    int file_exists;

    if (this == projNode::control_proj) return NULL;

    projModule *m = search_module_hash(nm);
    if (m) {
      ln_to_fn (nm, fn);      
      return m;
    }
    else
    {
        if (search_dir_hash(nm))
            return 0;
    }

    // get physical name
    ln_to_fn (nm, fn);
    if (!fn.length())	return NULL;

    // check if file exists
    if (fileCache_access ((char const *)fn, R_OK))
	file_exists = 0;
    else
	file_exists = 1;
    
    genString proj_name;
    fn_to_ln_imp (fn, proj_name);

    if (proj_name.length() == 0) return NULL;

    projNode* proj = find_project(proj_name, nm);
    if (!proj) return NULL;

    if (file_exists)
        proj->refresh();

    genString pathname = nm;

    symbolArr& proj_contents = proj->rtl_contents();
    symbolPtr sym;
    ForEachS(sym, proj_contents)
    {
	if(pathname == sym.get_name())
	    return checked_cast(projModule,sym);
    }

    return NULL;
}

projContents * projNode::get_contents_node() const
{
    Initialize(projNode::get_contents_node);

    projContents * con = checked_cast(projContents, get_first());
    return con;
}

char const * projNode::get_ln() const
{
    Initialize(projNode::get_ln);

    return name;
}

void projNode::put_subproject(projNode * sub)
{
    Initialize(projNode::put_subproject);
    if (sub) {
	char const *nm = sub->get_ln();
        projContents *cont = get_contents_node();
	sub_put_proj (sub, cont);

	childProject_put_parentProject (sub, this);
	rtl_insert(sub, 0);
    }
}

bool projNode::is_implicit_proj() // with =
{
    Initialize(projNode::is_implicit_proj);

    projMap *pmap = get_projmap_for_projnode(this);

    if (pmap && pmap->is_implicit_sub_dir_proj())
        return TRUE;

    return FALSE;
}

//  determine whether the current project contains any sub projects
//  The constructor initializes the flag to 3, and the first time we are
//  called, we will set it to either 0 or 1.  Subsequent calls are fast.
bool projNode::contain_sub_proj()
{
    Initialize(projNode::contain_sub_proj);

    if (subproj_flag==3) {
	if (this==get_control_project()) {
	    subproj_flag = 0;  // no subproj in control project
	} else {
	    projMap *pmap = get_projmap_for_projnode(this);
	    subproj_flag = pmap && pmap->get_view_flag() && pmap->contain_sub_proj();
	}
	if (!subproj_flag) subproj_pmod_flag = 0;  // if no subprojects, no pmods there either
    }
    return subproj_flag;
}

bool projNode::is_visible_proj()
{
    Initialize(projNode::is_visible_proj);

    projNode * cur=(projNode*)this;
    while( cur && !cur->pm)
    {
	char const *nm = (cur->name).str();
	if ( strncmp( nm, "__", 2 ) == 0 || strstr(nm, "/__") )
	    return FALSE;
        cur=cur->find_parent();
      }

    if (!cur)
        return FALSE;

    projMap *pmap = cur->get_map();
    if (pmap && pmap->get_view_flag())
        return TRUE;

    return FALSE;
}

// Returns: 1 on success
//
int projNode_set_pmod_names (projNode *proj)
{
    Initialize(projNode_set_pmod_names);
    if (proj->sym_name && proj->link_name && proj->ind_name && proj->prj_name)
	return 1;

    int res = 0;
    char const *any_name = NULL;
    if (proj->sym_name)
	any_name = proj->sym_name->get_name();
    else if (proj->link_name)
        any_name = proj->link_name->get_name();
    else if (proj->ind_name)
	any_name = proj->ind_name->get_name();
    else if (proj->prj_name)
	any_name = proj->prj_name->get_name();

    genString tmp;
    if (any_name)
	tmp = any_name;
    else {
	proj->xref_file_name (tmp);
	if (tmp.length()) tmp += ".XXX";
    }

    int len  = tmp.length();
    if (len) {
        char *nm = (char *)tmp;
	res = 1;
	if (!proj->sym_name) {
	    nm[len - 3] = 's'; nm[len - 2] = 'y'; nm[len -1] = 'm';
	    proj->sym_name = global_cache.add_entry (nm);
        } 
	if (!proj->link_name) {
	    nm[len - 3] = 'l'; nm[len - 2] = 'i'; nm[len -1] = 'n';
	    proj->link_name = global_cache.add_entry (nm);
	}
	if (!proj->ind_name) {
	    nm[len - 3] = 'i'; nm[len - 2] = 'n'; nm[len -1] = 'd';
	    proj->ind_name = global_cache.add_entry (nm);
        } 
	if (!proj->prj_name) {
	    nm[len - 3] = 'p'; nm[len - 2] = 'r'; nm[len -1] = 'j';
	    proj->prj_name = global_cache.add_entry (nm);
	}
    }
    return res;
}

static Xref *projNode_attach_xref (projNode *proj)
{
    Initialize(projNode::attach_xref);
    Xref *xr = NULL;
    int res = projNode_set_pmod_names(proj);
    if (res && proj->sym_name && proj->ind_name && proj->link_name) {
	proj->sym_name->reset(); proj->ind_name->reset();
	proj->link_name->reset();
	int sym_rok = proj->sym_name->is_rok();
	if (sym_rok == 1) {
	    int ind_rok = proj->ind_name->is_rok();
	    if (ind_rok == 1) {
		int ind_wok = proj->ind_name->is_wok();
		char const *nm = proj->sym_name->get_name ();
		genString xref_file;
		xref_file.put_value(nm, (int)(OSapi_strlen(nm) - 4));
		if (ind_wok == 1 || (proj->ind_name->is_fok() == 0))
		    xr = new Xref (xref_file, 0, proj);
		else
		    xr = new Xref (xref_file, 1, proj);
	    }
	}
    }

    if (xr) proj->set_xref (xr);
    return xr;
}

Xref *proj_attach_existing_xref (projNode *pn)
{
    Initialize(*proj_attach_existing_xref);
    return  projNode_attach_xref (pn);
}
//  This function is called both by the following one (proj_make_project),
//     and by load_project_rtl, in main.C.  It does the latter only for those
//     projects previously suppressed by the suppress_Xref flag.
void  attach_Xref(projNode* proj, projNode* parent, char const *name)
{
    Initialize(attach_Xref);
#ifdef _PSET_DEBUG
    if (suppress_Xref && proj->get_xref(0))
	cerr << "  We're suppressing a project that already has an Xref!! " << proj->get_name() << endl;
#endif
    if (suppress_Xref) return;      // postpone creating an Xref during scan of pdf
    if (proj->get_xref(0)) return;   // do not bother if we have already got an Xref

    Xref * xr = NULL;
    if ((xr = proj->get_xref()) && !xr->is_unreal_file())
	return;   // do not bother if we have already got an Xref
    xr = NULL;
    if(parent) {
	xr = projNode_attach_xref (proj);
    } else { // (this is not a subproject)
	Xref *xr = projNode_attach_xref (proj);
	if (xr == NULL) {   //  sym file was not accessible
	    if (errno == ENOENT) {	// if it's not there at all,
		genString basename;
		if (!name) {
		    basename = proj->get_name();
		    char const * tail = strrchr((char const *)basename,'/');
		    if (tail) basename = tail+1;
		    name = (char*)basename;
		}		    genString filename = "U-";
		if (name[0]!='_' || name[1]!='_') filename += name;
		else filename += name+2;
		genTmpfile tmpfile(filename);	// make a temp RO file
		filename = tmpfile.name();
		filename += ".pmod";
		proj->set_xref (new Xref((char*)filename, 2, proj));
	    }
	}
    }
}

projNode * projNode_make_project (char const *name, projNode* parent, char const *dir,
	      projMap* map, char const *ln, char const *sym_nm, char const *phy_name, int pmod_type)
{
  Initialize(projNode_make_project);
  // check if parent already has this project
  if (parent) {
      Obj *children = parentProject_get_childProjects(parent);
      Obj *el;
      ForEach (el, *children) {
	  projNode *child = checked_cast (projNode, el);
	  if (strcmp (child->get_name(), ln) == 0)
	      return child;
      }
  }

  // Get the project physical name
  int idx = -1;
  projNode *found_parent = NULL;
  projNode *exist_pn = find_projNode_by_impname (ln, idx, found_parent);
  if (exist_pn) {
      if (PDF_DebugLevel > 0) {
	  msg("PDF DEBUG:\tProject name \"$1\" already exists... Ignored.") << ln << eom;
	  ;
      }
      return NULL;
  }
  
  int null_flag = (sym_nm && (OSapi_strcmp((char const *)sym_nm, NULL_NAME) == 0)) ? 1 : 0;
  projHeader * header = db_new(projHeader, (ln, dir));
  projNode *proj = db_new(projNode, (ln, header, dir, map, 0, pmod_type));
  if (sym_nm && sym_nm[0] && !null_flag)
      proj->sym_name = global_cache.add_entry (sym_nm);

  if (phy_name && phy_name[0])
      proj->pr_name = global_cache.add_entry(phy_name);
  else if (map) {
      genString phname;
      char const *sub_proj_name = (parent) ? parent->get_name() : ln;
      map->ln_to_fn_imp (ln, sub_proj_name, DIR_TYPE, phname);
      if (phname.length())
	  proj->pr_name = global_cache.add_entry(phname);
  }

  if(parent) {
      parent->put_subproject(proj);
      if (!null_flag) attach_Xref(proj, parent, name);
  } else {
      if (!null_flag) attach_Xref(proj, parent, name);
      projList::search_list->add_proj (proj);

    projMap *pmap = proj->get_map();
    if (!pmap || pmap->get_view_flag())
        rtl_add_obj ((RTLNodePtr)projNode::get_project_rtl()->get_root(), proj);
  }

  return proj;
}

projNode * proj_make_project (char const *nm, projNode* prnt, char const *dir, projMap* map, char const *ln, int pmod_type)
{
    return projNode_make_project (nm, prnt, dir, map, ln, (char const *)0, (char const *)0, pmod_type);
}
void projNode::update_path_hash_tables(projMap *pmap, char const *nm)
{
    Initialize(projNode::update_path_hash_tables);

    genString fn;
    genString proj_n;
    pmap->ln_to_fn(nm, get_ln(), FILE_TYPE, fn, 0, 1);
    pmap->fn_to_ln_imp(fn, get_ln(), FILE_TYPE, proj_n, 0, 1);

    pmap->insert_hash(fn,     proj_n, nm,   FILE_TYPE, LOGIC_NAME_RULE,   FNTOLN);
    pmap->insert_hash(fn,     proj_n, proj_n, FILE_TYPE, PROJECT_NAME_RULE, FNTOLN);
    pmap->insert_hash(nm,   proj_n, fn,     FILE_TYPE, LOGIC_NAME_RULE,   LNTOFN);
    pmap->insert_hash(proj_n, proj_n, fn,     FILE_TYPE, PROJECT_NAME_RULE, LNTOFN);
}

projModule *projNode::create_script_module(char const *ln)
{
    Initialize(projNode::create_script_module);

    projNode *cur = this;

    while(cur && !cur->get_map())
        cur = cur->find_parent();

    if (!cur)
        return 0;

    projMap *pmap = cur->get_map();
    update_path_hash_tables(pmap, ln);
    genString fn;
    ln_to_fn(ln, fn);
    projModule *dst_mod = proj_make_module(fn, this, ln);
    insert_module_hash(dst_mod);
    return dst_mod;
}


// type = 0 for update (refresh) projects
// type = 1 for search modules
void projNode::force_refresh(int tp)
{
    Initialize (projNode::force_refresh);

    if ( !no_refresh_files || tp == 1)
    {
        no_refresh_files = no_refresh_dirs = 1;
        proj_generate_subs_one_level_force_refresh(this, MODULE_SUB | DIR_SUB);
    } else {
	no_refresh_dirs = 1;
        proj_generate_subs_one_level_force_refresh(this, DIR_SUB);
    }
}

void projNode::refresh_all()
{
  Initialize (projNode::refresh_all);
  refresh();

  // boris, 102697. Refresh projects in the PDF order
  symbolArr& proj_contents = rtl_contents();
  symbolPtr sym;
  ForEachS(sym, proj_contents) {
      if (sym.get_kind() != DD_PROJECT)
	  continue;
      
      appPtr app_head = sym.get_def_app();
      if ( !(app_head && is_projHeader(app_head)) )
	  continue;
      
      projNode *child = projNodePtr(app_head->get_root());
      child->refresh_all();
  }
}

void projNode::refresh()
{
    Initialize (projNode::refresh);

    // ".exec" contents may be changed by make, so ... 
    if (no_refresh_files) {
	char const *nm = get_name();
	char const *last_slash = strrchr(nm, '/');
	if (last_slash) {
	    char const *own_name = last_slash + 1;
	    if (!strcmp(own_name, ".exec"))
		no_refresh_files = 0;
	}
    }
	
    if (!no_refresh_files && !no_refresh_dirs) {
	no_refresh_files = no_refresh_dirs = 1;
        proj_generate_subs_one_level(this,  MODULE_SUB | DIR_SUB);
    }  else if(!no_refresh_files) {
	no_refresh_files = 1;
        proj_generate_subs_one_level(this,  MODULE_SUB);
    } else if(!no_refresh_dirs) {
	no_refresh_dirs = 1;
        proj_generate_subs_one_level(this,  DIR_SUB);
    }
}

void projNode::refresh_projects()
{
    if (!contain_sub_proj()) return;
    if (!no_refresh_dirs) {
	no_refresh_dirs = 1;
	proj_generate_subs_one_level(this,  DIR_SUB);
    }
}

bool projNode::visited()
{
    Initialize(projNode::visited);

    if (no_refresh_files || no_refresh_dirs)
        return TRUE;

    return FALSE;
}

void projNode::set_visited()
{
    Initialize(projNode::set_visited);

    no_refresh_files = 1;
    no_refresh_dirs  = 1;
}

bool projNode::is_script()
// return TRUE, if it is a script project
// return FALSE, otherwise.
{
    Initialize(projNode:is_script);

    projNode *cur = (projNode *)this;
    while(cur && !cur->pm)
        cur=cur->find_parent();

    if (!cur)
        return FALSE;

    projMap *pmap = cur->get_map();
    return pmap->is_script();
}


// level = 0: from refresh all visited project
// level = 1: from refresh one level
// level = 2: from script for the whole tree
void projNode::update_projects(int level)
{
    Initialize(projNode::update_projects);

    force_refresh(0);
    if (level == 1)
        return;
    
    symbolArr& proj_contents = rtl_contents();
    symbolPtr sym;
    ForEachS(sym, proj_contents)
    {
        if (sym.get_kind() == DD_PROJECT)
        {
            appPtr app_head = sym.get_def_app();
            if(app_head && is_projHeader(app_head))
            {
                projNode *child_proj = projNodePtr(app_head->get_root());
                if (child_proj && (child_proj->visited() && level == 0 || level == 2))
                    child_proj->update_projects(level);
            }
        }
    }
}


void projNode::fn_to_ln_imp(char const *fn, genString &s, char file_type)
{
 Initialize(projNode::fn_to_ln_imp(char*,genString&,char));
  
  projNode * cur=(projNode*)this;
  while( cur && !cur->pm)
              cur=cur->find_parent();
  s = 0;
  if (cur)
    cur->pm->fn_to_ln_imp(fn, get_ln(), file_type, s);
}

void projNode::ln_to_fn_imp(char const *ln, genString &s, char file_type)
{
 Initialize(projNode::ln_to_fn_imp);
  
  projNode * cur=(projNode*)this;
  while( cur && !cur->pm)
              cur=cur->find_parent();
  s = 0;
  if (cur)
    cur->pm->ln_to_fn_imp( ln, get_ln(), file_type, s, 0, 0, this);
}

void projNode::ln_to_fn_imp(char const *ln, genString &s, char file_type, int xx_f, int script_force_search)
{
 Initialize(projNode::ln_to_fn_imp);
  
  projNode * cur=(projNode*)this;
  while( cur && !cur->pm)
              cur=cur->find_parent();
  s = 0;
  if (cur)
    cur->pm->ln_to_fn_imp( ln, get_ln(), file_type, s, xx_f, script_force_search, this);
}

void projNode::fn_to_ln_imp(char const *fn, genString &s)
{
 fn_to_ln_imp(fn, s, FILE_TYPE);
}

void projNode::ln_to_fn_imp(char const *ln, genString &s)
{
 ln_to_fn_imp(ln, s, FILE_TYPE);
}

void projNode::ln_to_fn_no_link_check(char const *ln, genString &s)
{
    Initialize(projNode::ln_to_fn_no_link_check);

    projNode * cur=(projNode*)this;
    while( cur && !cur->pm)
        cur=cur->find_parent();
    s = 0;
    if (cur)
        cur->pm->ln_to_fn( ln, get_ln(), FILE_TYPE, s, 1, 0, this);
}

void projNode::ln_to_fn(char const * ln, genString & s, char file_type)
{
 Initialize(projNode::ln_to_fn(char*,genString&,char));
  projNode * cur=(projNode*)this;
  while( cur && !cur->pm)
              cur=cur->find_parent();
  s = 0;
  if (cur)
      cur->pm->ln_to_fn( ln, get_ln(), file_type, s, 0, 0, this);
}

void projNode::ln_to_fn(char const * ln, genString & s, char file_type, int xx_f, int script_force_search)
{
 Initialize(projNode::ln_to_fn(char const*,genString&,char,int,int));
  projNode * cur=(projNode*)this;
  while( cur && !cur->pm)
              cur=cur->find_parent();
  s = 0;
  if (cur)
      cur->pm->ln_to_fn( ln, get_ln(), file_type, s, xx_f, script_force_search, this);
}

void projNode::fn_to_ln(char const * fn, genString & s, char file_type, int xx_f, int script_force_search)
{
    Initialize(projNode::fn_to_ln(char const*,genString&,char,int,int));

    projNode *pr = this;
    fn_to_ln_internal(fn, s, file_type, xx_f, script_force_search, &pr);
}

void projNode::fn_to_ln(char const * fn, genString & s, char file_type)
{
    Initialize(projNode::fn_to_ln(char*,genString&,char));

    projNode *pr = this;
    fn_to_ln_internal(fn, s, file_type, &pr);
}


void projNode::ln_to_fn(char const * ln, genString & s)
{
    ln_to_fn(ln, s, FILE_TYPE);
}

void projNode::fn_to_ln(char const * fn, genString & s)
{
    projNode *pr = this;
    fn_to_ln_internal(fn, s, &pr);
}

void projNode::fn_to_ln_internal(char const *fn, genString &s, projNode **pr)
{
    Initialize(projNode::fn_to_ln_internal);

    fn_to_ln_internal(fn, s, FILE_TYPE, pr);
}

void projNode::fn_to_ln_internal(char const *fn, genString &s, char file_type, projNode **pr)
{
    Initialize(projNode::fn_to_ln_internal);

    projNode * cur=(projNode*)this;
    while( cur && !cur->pm)
        cur=cur->find_parent();
    s = 0;
    if (cur)
        cur->pm->fn_to_ln_internal((char *)fn, get_ln(), file_type, s, pr);
}

void projNode::fn_to_ln_internal(char const *fn, genString &s, char file_type, int xx_f,
                                 int script_force_search, projNode **pr)
{
    Initialize(projNode::fn_to_ln_internal);

    projNode * cur=(projNode*)this;
    while( cur && !cur->pm)
        cur=cur->find_parent();
    s = 0;
    if (cur)
        cur->pm->fn_to_ln_internal(fn, get_ln(), file_type, s, xx_f, script_force_search, pr);
}

projModule * projNode::make_module(char const * nm)
{
    Initialize(projNode::make_module);

    genString fn;
    ln_to_fn (nm, fn);
    return make_new_module(fn);
}


projNode * projNode::find_parent() const
{
   Initialize(projNode::find_parent);

   return (childProject_get_parentProject((projNode *)this));
}

projNode *projNode::root_project()
{
    Initialize (projNode::root_project);

    projNode *parent;
    projNode *proj;
    for (proj = this; parent = proj->find_parent(); proj = parent);

    return proj;
}


/* static */
projNode* projNode::get_home_proj ()
{
    return projNode::curr_home_proj;
}

/* static */
projNode* projNode::get_home_proj_top_node ()
{
    projNode *pr = projNode::curr_home_proj;
    while(pr && pr->get_map() == NULL){
	pr = pr->find_parent();
    }
    if(pr)
	return pr;
    else
	return projNode::curr_home_proj;
}

/* static */
projNode* projNode::set_home_proj_temp (projNode* proj)
{
    Initialize (projNode::set_home_proj_temp);
    projNode* old_proj = projNode::curr_home_proj;
    projNode::curr_home_proj = proj;
    projNode::curr_root_proj = proj;
    return old_proj;
}

// Given a project, call obj_insert for that project and all its 
//   children, recursively.  This is called from set_home_proj to
//   update all the possible icons.
static void insert_obj_recursive(projNode* proj)
{
    Initialize (insert_obj_recursive);
    if (!proj) return;
    obj_insert (proj->get_header(), REPLACE, proj, proj, NULL);
    Obj* os = parentProject_get_childProjects(proj);
    ObjPtr ob;
    ForEach(ob, *os) {
	projNode* pr = checked_cast(projNode, ob);
	if (pr) insert_obj_recursive(pr);
    }
}

/* static */
projNode* projNode::set_home_proj (projNode* proj)
{
    Initialize (projNode::set_home_proj);

    projNode* old_proj = projNode::curr_home_proj;

    update_xref_home_flag(old_proj, proj);

    if (old_proj != proj)
	smt_reset_strings();

    // next few lines is kind of a hack to suppress pmod crash rec in set_current_proj
    Xref* xr = proj->get_xref(1);
    int no_rec = -1;
    if (xr) {
      no_rec = xr->get_crash_recovery();
      xr->set_crash_recovery(1); // supress crash recovery
    }
   
    projNode::curr_home_proj = proj;
    projNode::set_current_proj (proj);

    // next few lines unsuppress pmod crash rec
    if (xr && (no_rec >= 0))
      xr->set_crash_recovery(no_rec);

// update view
    projNode* temp_proj;
    // for some reason, we have to start one level up from the old home proj,
    if (old_proj) {
	temp_proj = checked_cast(projNode,
		get_relation(parentProject_of_childProject,old_proj));
	if (!temp_proj) temp_proj = old_proj;
	insert_obj_recursive(temp_proj);
    }
    // and the same exactly for the new home project
    if (proj) {
	temp_proj = checked_cast(projNode,
		get_relation(parentProject_of_childProject,proj));
	if (!temp_proj) temp_proj = proj;
	insert_obj_recursive(temp_proj);
	// if we are running model build to change the pmod, then
	// expand the pmod by 3mb, right away
	if (is_model_build() && customize_get_write_flag()) {
	    Xref *xref = proj->get_xref();
	    if (xref)
		xref->make_room(3000000);
	}
    }

    re_generate_pdf_tree_for_home(proj);
    if (proj->get_xref(1)) // only do pmod crash recovery after pdf_tree is built
      proj->get_xref(1)->crash_recovery_pmod_from_pset(); // perform crash recovery if needed
    return old_proj;    
}

/* static */
int projNode::home_is_writeable()
{
    Initialize(projNode::home_is_writeable);
    
    projNode* proj = get_home_proj();
    
    RTLNode* list_of_projects = checked_cast(RTLNode,
					     proj->root_project()->
					     get_project_rtl()->get_root());

    int res = 0;
    if (list_of_projects->rtl_contents().includes(proj)) {
	Xref *xref = proj->get_xref();
	if (xref && xref->is_writable())
	    res = 1;
    }
    return res;
}

/* static */
void projNode::set_current_proj (projNode* proj)
{
    Initialize (projNode::set_current_proj);
    Xref* old_xref = NULL;
    Xref* new_xref = NULL;
    IF(!proj)
	return;                 // proj must not be zero
    // if Xref was unreal before, and this is our home project, create it now
    Xref* test_xref = proj->get_xref();
    if (test_xref && test_xref->is_unreal_file() && proj==curr_home_proj) {
	genString xref_file;
	projNode *xref_proj = test_xref->get_projNode();
	if (xref_proj) {
	    xref_proj->xref_file_name (xref_file);
	    if (xref_file.str()) {
		test_xref->make_real(xref_file);
		scope_mgr.invalidate_unit(xref_proj);
	    }
	}
    }

    projNode* wrproj = projList::search_list->writable();

    if (projNode::curr_root_proj) 
        old_xref = projNode::curr_root_proj->get_xref();

    projNode::curr_root_proj = proj;
    if (projNode::curr_root_proj) {
	//  if we're running buildxref, and no pmods for home project,
	//       create it here
	if (is_model_build() && proj==projNode::curr_home_proj && customize_get_write_flag()) {
	    // create xref if it doesn't exist
	    if (!proj->has_xref) {
		genString xref_file;
		proj->xref_file_name (xref_file);
		if (xref_file.length())
		    proj->set_xref (new_xref = new Xref (xref_file, 1, proj));
		else ;
	    } else ; // new_xref = proj->get_xref();
	} else ;     // new_xref = proj->get_xref();
	if (!new_xref) new_xref = proj->get_xref();
    }

    if (new_xref != old_xref) {
//     Normally, current project must be writable.  But write_flag is cleared if all the user
//        is trying to do is to examine the pmod with -checkpmod, -p, ...
//        In that case, we only make it writable if it is the control project
	projNode* cont_proj = projNode::get_control_project();
	bool writeable_needed=customize_get_write_flag();
        if (customize_get_write_flag() || proj == cont_proj) {
	    if (new_xref) new_xref->to_write();
        }
    }
    projList::search_list->writable (proj);
}
 
/* static */
projNode* projNode::get_current_proj ()
{
    return projNode::curr_root_proj;
}

/* static */
RTL* projNode::get_project_rtl ()
{
    return projNode::projects;
}


void  projNode::set_xref(Xref *x)
{
    Initialize (projNode::set_xref);

    if (!x || (x == xref)) return;
    if (xref)
	delete xref;	// if there already was one, delete it first
    has_xref = (x)?1:0;  // XXX: deadcode.  Let's see what Prevent thinks....
    xref = x;
}

Xref* projNode::get_xref (int )
{
  if (has_xref)
    return xref;
  return 0;
}

Xref* projNode::get_xref ()
{
    Initialize (projNode::get_xref);

    projNode* cur_proj = this;

    while (cur_proj && !cur_proj->has_xref)
       cur_proj = checked_cast(projNode,cur_proj->find_parent());

    return cur_proj ? cur_proj->xref : 0;
}

/* static */
projNode* projNode::fn_to_pmod_proj(char const* pn, genString &ln)
{
    Initialize (projNode::fn_to_pmod_proj);
    projNode* topproj =0;
    projNode* cur_proj;
    projHeader::fn_to_ln (pn, ln, &topproj, 1);

    if (!ln.length())
        return 0;
   
    genString proj_name;
    topproj->fn_to_ln_imp(pn, proj_name);
    cur_proj = topproj->get_pmod_project_from_ln(ln.str());
    if (cur_proj == 0) // no pmod exist for this path, default is root
      cur_proj = topproj;

// Now we have the actual subproject the file is in, but
//  it may not actually have a pmod
//  If not, search upwards through projects, returning the
//    first one that has a pmod.  If none do, return the
//    top level projet.
    projNode* temp = cur_proj;
    while (temp && !temp->has_xref) {
	temp = checked_cast(projNode,cur_proj->find_parent());
	if (temp) cur_proj = temp; 
   }
    return cur_proj;
}


void projNode::delete_xref()
{
    Initialize(projNode::delete_xref);
    if (xref && has_xref) {
	delete xref;
	xref = NULL;
	has_xref = 0;
    }
// Now do the same for all the subprojects of this one
    Obj* os =  parentProject_get_childProjects(this);
    ObjPtr ob;
    ForEach(ob, *os) {
	projNode* pr = checked_cast(projNode, ob);
	if (pr) pr->delete_xref();
    }
}

// delete all the xref data structures
// we've been using for the various projects

/* static */
void projNode::terminate()
{
    Initialize (projNode::terminate);

    projList* list = projList::search_list;
    if (list)
    {
	projNode* pr = NULL;
	for (int i = 0; pr = list->get_proj(i); ++i)
            pr->delete_xref();
    }

    // Reconstruct the names of the control project Xref files, and delete them.
    // We know the files base name as our static member control_proj_xref_file.
    // (we really shouldn't have any knowledge of the Xref files here).

    genString ind_file;
    genString sym_file;
    genString link_file;

    Xref::ind_file(control_proj_xref_file, ind_file); // construct the file names
    Xref::sym_file(control_proj_xref_file, sym_file);
    Xref::link_file(control_proj_xref_file, link_file);

    OSapi_unlink(ind_file.str()); // do the deletions
    OSapi_unlink(sym_file.str());
    OSapi_unlink(link_file.str());
}

genString projNode::control_proj_xref_file;

/* static */
void projNode::proj_init()
{
    Initialize(projNode::proj_init);          

    if (projNode::projects)
	return;

    genTmpfile tmpfile("CONT-");
    control_proj_xref_file = tmpfile.name();
    control_proj_xref_file += ".pmod";

    Xref* xr = load_XREF (control_proj_xref_file, 0);
    if (xr) xr->make_room(1000000);       // create some breathing room in control project
    projNode::projects = new RTL(NULL);
    projList::search_list = new projList;
    projList::full_list = new projList;

    projNode::loaded_files = create_loadedFiles("Loaded Files");      
    RTL *lf = projNode::loaded_files;   
    RTLNode *nodes = checked_cast(RTLNode,projects->get_root()) ;
    rtl_add_obj(nodes, lf);

    projNode::unparsed_files = create_loadedFiles("Unparsed Files");
    RTL *uf = projNode::unparsed_files;   
    rtl_add_obj(nodes, uf);
    
    
    projHeader* head = new projHeader ("ParaControl", "/tmp");
    projNode::control_proj = new projNode ("/tmp", head, "/tmp", 0);

    projNode::control_proj->has_xref = 1;
    projNode::control_proj->xref = xr; 

    projList::search_list->writable (projNode::control_proj);
    projList::search_list->add_proj (projNode::control_proj);

    genString docs;

    docs += customize_install_root();
    docs += "/lib/ParaDOCS.pdf";

    init_psetProjects (docs);
}

/* static */
projNode* projNode::get_control_project()
{
  return projNode::control_proj;
}

// this is parent
int projNode::is_younger(projNode* child)
{
  while (child) {
    if (child == this)
      return 1;
    else
      child = child->find_parent();
  }
  return 0;
}
projNode* projNode::get_child_project(char const* pn)
{
  Initialize(projNode::get_child_project);
  if (strcmp(pn, name) == 0) return this;
  Obj* os = parentProject_get_childProjects(this);
  ObjPtr ob;
  ForEach(ob, *os) {
    projNode* pr = checked_cast(projNode, ob);
    projNode* cpr = pr->get_child_project(pn);
    if (cpr) return cpr;
  }
  return 0;
}

// one utility
// return 0 if both file are identical.
// return 1 if they are different.
int paraset_cmp(char const *local_fn, char const *global_fn)
{
    Initialize(paraset_cmp);

#define BUF_SIZE 1024

    char *local_buf;
    char *global_buf;

    local_buf  = (char *)psetmalloc(BUF_SIZE);
    global_buf = (char *)psetmalloc(BUF_SIZE);

    int local_f;
    int global_f;

    local_f  = OSapi_open(local_fn, O_RDONLY, 0666);
    if (local_f < 0)
        return 1;

    global_f = OSapi_open(global_fn,O_RDONLY, 0666);
    if (global_f < 0)
    {
        OSapi_close(local_f);
        return 1;
    }

    int status = 0;
    int read_size;
    while((read_size = OSapi_read(local_f,local_buf,BUF_SIZE)) > 0)
    {
        int size0 = OSapi_read(global_f,global_buf,BUF_SIZE);
        if (read_size != size0 || memcmp(local_buf,global_buf,read_size))
        {
            status = 1;
            break;
        }
    }

    if (OSapi_read(global_f,global_buf,BUF_SIZE)) // second file not done
        status = 1;

    OSapi_close(local_f);
    OSapi_close(global_f);

    psetfree(local_buf);
    psetfree(global_buf);

    return status;
}

void projNode::search_modules_by_name(char const *proj_name, symbolArr &s1)
{
    Initialize(search_modules_by_name);

    projMap *pmap = get_map();
    if (pmap)
        pmap->search_modules_by_name(this, proj_name, s1);
}

void projNode::search_nodes_by_name(char const *proj_name, symbolArr &s1)
{
    Initialize(search_nodes_by_name);

    refresh_projects();

    symbolArr& proj_contents = rtl_contents();
    symbolPtr sym;
    ForEachS(sym, proj_contents)
    {
        if (sym.get_kind() == DD_PROJECT)
        {
            appPtr app_head = sym.get_def_app();
            projNode *child_proj = projNodePtr(app_head->get_root());

            char const *ln = child_proj->get_ln();
            long len = strlen(proj_name);
            long len1 = strlen(ln);
            if (len1 >= len)
            {
                char const *p = ln + len1 - len;
                if (len1 > len && *(p-1) == '/' || len1 == len)
                {
                    if (strcmp(p, proj_name) == 0)
                        s1.insert_last(sym);
                }
            }

            child_proj->search_nodes_by_name (proj_name, s1);
        }
    }
}


void projNode::search_all_modules(symbolArr &s1)
{
    Initialize (projNode::search_all_modules);

    if (is_projNode(this)) 
        force_refresh(1);

    symbolArr& proj_contents = rtl_contents();
    symbolPtr sym;
    ForEachS(sym, proj_contents)
    {
        if (sym.get_kind() == DD_MODULE)
            s1.insert_last(sym);
        else if (sym.get_kind() == DD_PROJECT)
        {
            appPtr app_head = sym.get_def_app();
            if(app_head && is_projHeader(app_head))
            {
                projNode *child_proj = projNodePtr(app_head->get_root());
                child_proj->search_all_modules(s1);
            }
        }
    }
}

void proj_search_modules_by_name(char *proj_name, symbolArr &s1)
{
    Initialize(proj_comp_nodes);

    projNode* proj;
    for (int i=1; proj = projList::search_list->get_proj(i); i++)
        if (is_projNode(proj))
	    proj->search_modules_by_name(proj_name, s1);
}

symbolPtr projNode::lookup_symbol (ddKind knd, char const *nm)
{
    Initialize(projNode::lookup_symbol);

    symbolArr sa;
    symbolPtr sym = NULL_symbolPtr;

    Xref* Xr = get_xref();
    if (Xr)
	sym = Xr->lookup_in_lxref(sa, knd, nm); 

    return sym;
}

/* static */
fsymbolPtr projNode::lookup_xrefSymbol_in_cur_proj(ddKind kind, char const *fn)
{
    Initialize(projNode::lookup_xrefSymbol_in_cur_proj);

    projNode *proj = projNode::get_current_proj();
    Xref* xr = proj->get_xref(1);
    if ((xr==0) || (!xr->has_lxref()))
        return NULL_symbolPtr;

    symbolArr res;
    return xr->lookup_in_lxref(res,kind,fn);
}


bool proj_is_writable(projNode *proj, genString& fn)
//
//  check if project is writable (that requires writability of project, pmod
//  files as well as w and x permissions on the directory where pmod files are)
//
//  return the physical filename of the proposed pmod
//
{
    Initialize(proj_is_writable);
    Assert(proj);     // proj must be nonzero

    Xref *xr = proj->get_xref() ;

    genString xref_fn;
    proj->xref_file_name(xref_fn);  
    xref_fn += ".sym";
    fn = xref_fn;     // physical filename for caller to use

    int ret_val = 0;
    if (proj->is_writable())
    {
//
// determine if the directory with *.pmod.* files is writable
//
	char *str = (char *) xref_fn;
	if (str) 
	{
	    char *end = strrchr(str,'/');
	    if (end) 
	    {
		char c = *(end + 1);
		*(end + 1) = '\0';
		int test1 = !OS_dependent::access(str, W_OK);
		int test2 = !OS_dependent::access(str, X_OK);
		*(end + 1) = c;
//
// now check if the pmod files are writable. Note that we require that the pmod
// files as well as the directory where they are located be writable.
//
		struct OStype_stat stat_buf;
//
// the logic to determine writability of the project is the following: first, require
// that the directory has w+x permission. If that is satisfied, consider the following
// two cases:
//
// 1. pmod files do not exist. Then project is writable.
//		
// 2. pmod files do exist. Then project is writable if xr->is_project_writable() (if
// xr is 0, Trung says to consider project writable - this will probably never happen).
//
		if (test1 && test2)
		{
		    ret_val = 1;
		    if (!OSapi_stat(xref_fn, &stat_buf)) 
			if (xr) ret_val = xr->is_project_writable();
		}
	    }
	}
    }
    return ret_val;
}

/* static */
void projNode::update_xref_home_flag(projNode* old_proj, projNode* new_proj)
{
  Initialize (projNode::update_xref_home_flag);
  if (old_proj != new_proj) {
    
    Xref* xr = 0;

    //reset home flag in old home project
    if (old_proj)  {
      xr = old_proj->get_xref(1);
      if (xr) xr->reset_home_flag();
    }

    //set home_flag in current home project
    xr = new_proj->get_xref(1);
    if (xr)
      xr->set_home_flag();
    
  }
}
