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
//------------------------------------------
// synopsis:
//    define the member functions for the class projModule
//
//------------------------------------------

// INCLUDE FILES

#include <cLibraryFunctions.h>
#include <msg.h>
#include <prompt.h>
#include <machdep.h>

#include <saveXref.h>
//#include <top_widgets.h>     // for push_busy_cursor, pop_cursor
#include <gtDisplay.h>
#include <app.h>
#include <db_intern.h>
#include <cmd.h>
#include <cmd_enums.h>
//#include <viewUIexterns.h>

#ifndef _question_h
#include <Question.h>
#endif
#include <ddict.h>
#ifndef _ddL_boil_h
#include <dd_boil.h>
#endif

#ifndef _gtRTL_boil_h
#include <gtRTL_boil.h>
#endif

#include <XrefTable.h>
#include <transaction.h>
#include <fileEntry.h>
#include <feedback.h>
#include <proj.h>
#include <timer.h>
#include <driver_mode.h>
#include <disbuild_analysis.h>

#undef MAX

void update_path_hash_tables(char *, projMap *, projNode *);
void backup_the_files(char const *, char const *, char);
int outdated_pset(char const *, char const *, char const *);
int char_based_merge (app *app_h, char const *srcf, char const *psetf, int unload_flag, int& do_not_save_flag);
bool is_forgiving();

#include <errorBrowser.h>
#include <proj_save.h>

#define NULL_NAME "NULL"

#include <vpopen.h>
#include <customize.h>
#include <messages.h>

#include <objOper.h>
#include <objRawApp.h>
#include <proj.h>
#include <tpopen.h>
#include <ddKind.h>
#include <path.h>
#include <pdf_tree.h>

#include <targetProject.h>

#include <xref.h>
#include <db.h>
#include <save.h>

#include <projBackEnd.h>
#include <fileCache.h>

#include <db_intern.h>
#include <RTL_externs.h>
#include <merge_dialog.h>

#include <NewPrompt.h>
#include <genTmpfile.h>

#include <scopeMgr.h>
#include <shell_calls.h>
#include <disbuild_analysis.h>

#define MRG_NOREVISION "no revision"

// this variable is used to synchronize crash rec of files when
// exiting paraset
// assuming projModule::put_module is always called by child process
static int calling_put = 0;
 
int call_from_put()
{
  return calling_put;
}
 
void call_from_put(int n)
{
  calling_put = n;
}

extern void fine_grained_dependency_analysis(projNode*);
extern bool browser_delete_module(gtBase* parent, projModule *module, bool yes_to_all, char const *msg);
//extern "C" int browser_import(...);
void get_pset_fn (char const*, genString&);

static int check_out_pset_special(projModule*,char *,char *,char *,char const*,char const*,char const *,int);

void proj_path_report_app(app *, projNode *prj = NULL);
bool els_parse_edg(projModule *);
int els_parse_file(const Relational*);

void proj_create_ctl_pm();
int get_file_names (char const *ln, projNode* src_proj,  projNode* dst_proj,
    genString& src_file, genString& dst_file,
    genString& src_paraset_file, genString& dst_paraset_file);

bool do_delete_pset();
void ste_smart_view_update (smtHeaderPtr, int with_text = 1);
int ste_temp_view_suspend (smtHeaderPtr);

int ignore_prop;

void obj_delete_or_unload(Obj *root, int delete_flag);
static int delete_module_internal(projModule* This, int new_flag, int type);
projModule * symbol_get_module( symbolPtr& sym);

//boris: remove references to ddElement
extern int module_get_dds (projModule *mod, objSet& dd_set);
extern int dd_set_aname (app *app_head, genString& aname);
extern int dd_get_aname (app *ah, genString& aname);
extern int dd_contain_ddKind (app *app_head, ddKind ki);
extern int dd_dependency_analysis (app *head, char& save_flag);
extern "C" void push_busy_cursor();
extern "C" void pop_cursor();
// pn == 0: pm is for this module
// pn != 0: pm is for pn not for this module
void projModule::update_path_hash_tables(projMap *pm, projNode *pn)
{
    Initialize(projModule::update_path_hash_tables);

    char const *name = get_name(); // logical name should be identical
    genString fn;
    genString proj_n;
    if (pn)
    {
        pm->ln_to_fn(name, pn->get_ln(), FILE_TYPE, fn, 0, 1, pn);
        pm->fn_to_ln_imp(fn, pn->get_ln(), FILE_TYPE, proj_n, 0, 1);
    }
    else
    {
        projNode *proj = get_project_internal ();
        if (pm->is_script())
        {
            pm->ln_to_fn(name, proj->get_ln(), FILE_TYPE, fn, 0, 1, proj);
            pm->fn_to_ln_imp(fn, proj->get_ln(), FILE_TYPE, proj_n, 0, 1);
        }
        else
        {
            proj->ln_to_fn(name, fn, FILE_TYPE);
            proj->fn_to_ln_imp(fn, proj_n, FILE_TYPE);
        }
    }
    pm->insert_hash(fn,     proj_n, name,   FILE_TYPE, LOGIC_NAME_RULE,   FNTOLN);
    pm->insert_hash(fn,     proj_n, proj_n, FILE_TYPE, PROJECT_NAME_RULE, FNTOLN);
    pm->insert_hash(name,   proj_n, fn,     FILE_TYPE, LOGIC_NAME_RULE,   LNTOFN);
    pm->insert_hash(proj_n, proj_n, fn,     FILE_TYPE, PROJECT_NAME_RULE, LNTOFN);
}

void update_path_hash_tables(char *name, projMap *pm, projNode *pn)
{
    Initialize(update_path_hash_tables);

    genString fn;
    genString proj_n;
    if (pn) {
        pm->ln_to_fn(name, pn->get_ln(), FILE_TYPE, fn, 0, 1, pn);
        pm->fn_to_ln_imp(fn, pn->get_ln(), FILE_TYPE, proj_n, 0, 1);

	pm->insert_hash(fn, proj_n, name, FILE_TYPE, LOGIC_NAME_RULE, FNTOLN);
	pm->insert_hash(fn, proj_n, proj_n, FILE_TYPE, PROJECT_NAME_RULE, FNTOLN);
	pm->insert_hash(name, proj_n, fn, FILE_TYPE, LOGIC_NAME_RULE, LNTOFN);
	pm->insert_hash(proj_n, proj_n, fn, FILE_TYPE, PROJECT_NAME_RULE, LNTOFN);
    }
}

int projModule::check_write_priv(int the_type)
// check the write privilege of file
//
// the_type == 0: check both file and pset
//         1: check only file
//         2: check only pset
{
    Initialize(projModule::check_write_priv);

    genString fn;
    get_phys_filename(fn);
    struct OStype_stat buf0;
    int status0;
    int own_flag;
    if (the_type == 0 || the_type == 1)
    {
        status0 = global_cache.stat(fn.str(),&buf0);
        if (status0 == 0)
        {
            own_flag = OSapi_chmod(fn.str(),buf0.st_mode) == 0;
#ifndef _WIN32
            if (!(own_flag || !own_flag && (S_IWGRP&buf0.st_mode)))
#else
            if (!own_flag)
#endif /*_WIN32*/
                return 0; // no write priviledge for the source file
        }
        else
            return 0;
	if (the_type == 1)
	    return 1;
    }

    if (the_type == 0 || the_type == 2)
    {
        char const *paraset_file = paraset_file_name();
        status0 = global_cache.stat(paraset_file,&buf0);
        if (status0 != 0) // the paraset file not exist
            return 1;
	own_flag = OSapi_chmod(paraset_file,buf0.st_mode | S_IWRITE) == 0;
#ifndef _WIN32
        if (!(own_flag || !own_flag && (S_IWGRP&(buf0.st_mode))))
#else
        if (!own_flag)
#endif /*_WIN32*/
            return 0; // no write priviledge for the paraset file
        return 1;
    }
    return 0; // bad the_type
}

void projModule::invalidate_cache()
// clean the stat data in the cache table for the source file and the pset file
// such that the next it will call OSapi_lstat() for these two files
{
    projNode* project = get_project_internal();

    // After delete the whole directory tree
    // project might be deleted
    // projModeule destructor is calle at end_transaction()
    // lets check if project is NULL
    if (project)
    {
        genString fn;

	if (projfile_name)
	    projfile_name->reset();

	if (!psetfile_name)
		paraset_file_name();

	if (psetfile_name)
	    psetfile_name->reset();
    }
    cache_valid = 0;
}

bool has_paraset_file(char const * pset_file)
{
    struct OStype_stat stat_buf;

    bool hpf = (global_cache.stat(pset_file, &stat_buf) == 0);

    return hpf;
}

bool projModule::is_paraset_file ()
{
    Initialize (projModule::is_paraset_file);

    if (!cache_valid) {
	cache_paraset_file = 0;
	struct OStype_stat stat_buf;
 	char const *pset_fname = projModulePtr(this)->paraset_file_name();
	if (pset_fname && pset_fname[0]) {
	    if ( (global_cache.stat (pset_fname, &stat_buf) == 0) )
		cache_paraset_file = 1;
	    cache_valid = 1;
	}
    }
 
    return cache_paraset_file;
}


void projModule::set_target_project(projNode *pn)
{
    Initialize(projModule::set_target_project);

    target_proj = pn;
}

appPtr projModule::get_app()
// Get the module's corresponding app header 
{
    appPtr ah = projectModule_get_appHeader (this);

    if (ah)
	return ah;

    char const *ln = get_name();
    genString pn;

    projNode *proj = get_project();

    get_phys_filename(pn);
    if (get_app_ln())
        ln = get_app_ln();

    ah = get_app(ln, pn);

    if (ah) {
	projectNode_put_appHeader (proj, ah);
	projectModule_put_appHeader (this, ah);
    }

    return ah;
}

appPtr projModule::get_app(char const *ln, char const *pn)
// return app header for this module
//
// ln: is the logic name to be compared
// pn: is the physical name to be compared
{
    Initialize(projModule::get_app);

    if (ln == 0)
        return 0;

    objArr* list = app::get_app_list();

    Obj *el;
    ForEach (el, *list)
    {
        appPtr ah = checked_cast(app,el);

        char const* log_name = ah->get_filename();
 
        if(log_name  &&  strcmp(log_name, ln) == 0) {

	    if (!pn) return ah;

	    // match physical name too before making decision
	    char const *phy_name = ah->get_phys_name();
	    if (phy_name && !strcmp (phy_name, pn))
		return(ah);
	}
    }
    return(0);
}

appPtr projModule::get_app(char const* ln)
{
    return get_app (ln, 0);
}

bool projModule::is_good_paraset_file()
{
    Initialize(projModule::is_good_paraset_file);

    if (is_paraset_file() && !outdated_pset())
        return 1;
    else
        return 0;
}

int outdated_pset_time(char const * src_file, char const *pset_file)
//  nonzero means the pset is outdated, or that it got a status error
{
    Initialize(outdated_pset_time);

    struct OStype_stat pset_stat_buf, stat_buf;

    if (OSapi_stat((char *)pset_file, &pset_stat_buf) != 0) return 1;
    if (OSapi_stat((char *)src_file, &stat_buf) != 0) return 0;

    return pset_stat_buf.st_mtime < stat_buf.st_mtime;
}

// pset_file = fn + ".pset"
int outdated_pset(char const * src_file, char const *pset_file, char const *fn)
{
    if (!has_paraset_file (pset_file))
        return 0;

    if (!db_get_db_format((char *)fn) || global_cache.is_frame(fn))
        return ::outdated_pset_time(src_file, pset_file);

    genTmpfile tnm("pset");

    int status;
    // -1 : fail
    // -2 : pset does not have source
    status = db_read_src_from_pset ((char *)pset_file, (char *)tnm.name());
    if (status == -2)
      return FALSE;
    else if (status < 0)
        return status;

    status = paraset_cmp((char *)src_file, (char *)tnm.name());
    tnm.unlink();

    if (status == 0) // identical
        return FALSE;

    return TRUE;
}

int projModule::outdated_pset()
// to compare the source in the .pset file and the source file
//
// If they are identical, return FALSE; otherwise return TRUE.
// If it is loaded, it is identical.
{
    Initialize (projModule::outdated_pset);

#ifndef _WIN32
    if (get_app())
        return FALSE;
#endif

    char const* pset_file = paraset_file_name();
    char const* src_file = get_phys_filename();

    return ::outdated_pset(src_file, pset_file, src_file);
}

void projModule::touch_module(int delete_pset)
{
  Initialize(projModule::touch_module);

  if (!is_project_writable())
    return;
  
  char *pset_file = (char *)paraset_file_name();
  
  if (!has_paraset_file(pset_file))
    return;

  if(OSapi_access(pset_file, W_OK) < 0) {
    struct OStype_stat buf0;
    OSapi_stat(pset_file, &buf0);
    if(OSapi_chmod(pset_file, 0666) != 0){
      if (!db_change_to_writable(pset_file)){
	msg("Cannot change_to_writable $1", error_sev) << pset_file << eom;
	return;
      }
    } else {
      OSapi_chmod(pset_file, buf0.st_mode);
    }
  }
   
  if(delete_pset)
    OSapi_unlink(pset_file);
  else
    db_set_bit_need_reparse(pset_file, 1);
}


static int insert_pset_in_xref(projModule*module, projNode* proj, int is_header, int do_insert)
{
  int status = 0;

  app *head = module->restore_module();

  if (head) {
    if(is_header)
      module->dependency_analysis(head, proj, 1);

    if(do_insert){
      Xref * xr = proj->get_xref();
      if (xr){
	xr->insert_module_in_lxref(head);
	xr->save_lxref_by_module(head->get_filename());
      }
    }
    obj_unload(head);
    module->invalidate_cache();
    status = 1;
  }
  return status;
}

static void rm_pset_and_mod(projModule *pm, char const*pset_file, projNode *proj)
{
  Initialize(rm_pset_and_mod);
  IF (pm == NULL) return;
  if (proj == NULL)
    proj = pm->get_project();

  if(!pset_file)
     pset_file = pm->paraset_file_name();

  Assert(pset_file);

//  OSapi_unlink(pset_file);
//  boris: it is possible that the pset_file is not writable
//         MG's touch_module() will check out and delete the file

  char const * fn = pm->get_phys_filename();
  if ( !fn || !fn[0] )
      fn = pm->get_name();

  msg(" touch(undo_forgive) $1 $2\n") << fn << eoarg << pm->get_name() << eom;

  int delete_pset = 1;
  pm->touch_module(delete_pset);

  Xref *xref = proj->get_xref();
  if (xref)
      xref->remove_module (pm->get_name());
}

int module_needs_reparse(projModule *module)
{
    Initialize(module_needs_reparse);

    char *pset_file = (char *)module->paraset_file_name();
    IF(!pset_file)
      return 0;

    int need_reparse = 1;
    int db_format = 0;
    bool has_pset = has_paraset_file(pset_file);
    if (has_pset){
        need_reparse = db_get_pset_need_reparse_no_forgive(pset_file);
    }
    int outdated_pset_flag = module->outdated_pset();

    if(need_reparse || outdated_pset_flag){
	return 1;
    }

    return 0;
}

void add_outdated_module(projModule*);

void projModule::update_xref()
// pass 1 for -batch
//
// Dependency analysis from all header files to all source files.
// It requires the restore and merge of the header files.
//
// 0. If it satisfies the following four conditions
//
//    . a source file (*.[cC])
//    . is updated (the need_reparse bit in the header of the .pset file is 0)
//    . is not outofdated (the source in the .pset file and the source are identical)
//    . its pset is newer than the pmod
//
//    do the restore to update the pmod and go to pass 2.
//
// 1. Check out the .pset file for the head or source file (*.[cChH])
//    which requires reparsed if
//    . the .pset file dose not exist OR
//    . the .pset exists but
//                          . no write permission AND
//                          . need_parse_bit is set OR the source is outofdated
//
// 2. Do the restore and/or merge for this .pset file.
// 3. Check out the .pset files for the touched source files.
// 4. Delete the .pset file for the head file.
{
    Initialize (projModule::update_xref);

    projNode *proj = get_project();

    char *path =  (char *)get_phys_filename();
    IF (!path)
      return;

    char *pset_file = (char *)paraset_file_name();
    IF(!pset_file)
      return;

    timer::init (1, "update_xref", path);

    int need_reparse = 1;
    int db_format = 0;
    bool has_pset = has_paraset_file(pset_file);
    if (has_pset){
        need_reparse = db_get_pset_need_reparse_no_forgive(pset_file);
        db_format = db_get_pset_format(pset_file);
    }
    int outdated_pset_flag = outdated_pset();

    if (!is_forgiving() && do_delete_pset() && db_get_pset_forgive(pset_file)){
	rm_pset_and_mod(this, pset_file, proj);
	timer::showtime (1, "update_xref", path);
	return;
    } 

    if(need_reparse || outdated_pset_flag)
	add_outdated_module(this);

    int is_header = ! xref_not_header_file(path);

    if (!need_reparse && !outdated_pset_flag){
	// boris 090298; disbuils analysis pass skips everything but outdated header files
	if (!disbuild_analysis_mode()) {
	    symbolPtr tsp = Xref_file_is_newer((char *)ln_name, (char *)path);
	    bool do_the_update = tsp.xrisnull();
	    if (do_the_update)
		insert_pset_in_xref(this, proj, is_header, 1);
	}
    } else if (is_header) {
      if(outdated_pset_flag){
	int late_save  = 1;

	// boris 970514
	char file_type = -1;  // Ignore read/write permitions. We merge in memory only.
	char do_backup = 0;   // Do not do backup, because we do not save the file.
	char do_unload = 0;   // Do not unload after merge

	projModule::merge_module(this, path, pset_file, file_type, do_backup, do_unload, late_save);

        // boris 090298; in disbuild analysis pass just do dependency
	if (disbuild_analysis_mode()) {
	    app *head = restore_module();
	    if (head) dependency_analysis(head, proj, 1);
	}
      }

      // boris 090298; disbuild analysis pass does not change pmod
      if (!disbuild_analysis_mode())
	  insert_pset_in_xref(this, proj, is_header, 0);

      unload_module();
    }

    timer::showtime (1, "update_xref", path);
    show_arena(path);
}

static app * last_inserted_app;
static Xref * last_inserted_Xref;


void proj_set_app_insert_Xref (app* h, Xref* xr)
{
    last_inserted_app = h;
    last_inserted_Xref = xr;
}

extern "C" void cmd_journal_dump_pset(app *);

bool projModule_dump_pset(projModule *mod)
{
  app *head = NULL;
  bool val = false;

  if (has_paraset_file(mod->paraset_file_name())) {
    head = mod->restore_module();
    if (head) {
      cmd_journal_dump_pset(head);
      val = true;
    }
  }
  return val;
}

app* projModule_try_to_restore(projModule* mod)
{
  app* head = NULL;
  char *path = (char *)mod->get_phys_filename();
  char *pset_file = (char *)mod->paraset_file_name();
  char const * ln_name = mod->get_name();
  projNode* proj_node = mod->get_project();

  if(has_paraset_file(pset_file) && !mod->outdated_pset()){
    if(!db_get_pset_need_reparse_no_forgive(pset_file)){
      symbolPtr tsp  = Xref_file_is_newer(ln_name, path);
      bool do_the_update = tsp.xrisnull();
      if(do_the_update){
	proj_set_app_insert_Xref (NULL, NULL);
	head = mod->restore_module();
	if (head){
	  Xref * xr = proj_node->get_xref();
	  if (xr)
	    if (xr != last_inserted_Xref || head != last_inserted_app)
	      xr->insert_module_in_lxref(head);
	  save_cur_xref (head);
	}
      }
    }    
  }
  return head;
}

// projModule::load_module
//
// Load the module into memory - restore if needed otherwise import it

appPtr projModule::load_module()
{
  return load_module_internal (1);
}

// projModule::load_module_internal
//
// Load the module into memory - restore if needed otherwise import it

appPtr projModule::load_module_internal(int do_import)
{
    Initialize (projModule::load_module_internal);
    appPtr ah = get_app();
    if (ah)
    {
        if (is_smtHeader(ah))
        {
            smtHeaderPtr smt = smtHeaderPtr(ah);
            if (smt->foreign && smt->parsed == 0) {
                obj_unload (smt);
		smt = NULL;
                ah  = NULL;
	    } else
		return ah;
        }
        else
            return ah;
    }
    ah = restore_module();
    if (!ah  && do_import)
      ah = import_module();

    if (ah) {
      projNode* proj_node = get_project();
      if(proj_node)
	{
	  projHeaderPtr proj_hdr = checked_cast(projHeader, proj_node->get_header());
	  if (proj_hdr)
            obj_insert(proj_hdr, REPLACE,proj_node,proj_node,NULL);
	}
      xref_notifier_report(0, this);
    }
    return ah;
}

appPtr projModule::import_module()
// import module
{
    Initialize(projModule::import_module);

    appPtr ah = get_app();
    if (ah)
    {
	if (is_smtHeader(ah))
        {
            smtHeaderPtr smt = (smtHeader*)ah;
	    if (smt->foreign && smt->parsed == 0) {
                obj_unload (ah);
                smt = NULL;
                ah  = NULL;
            }
            else
               return ah;
        }
        else
	    return ah;
    }

    projNode* proj_node = get_project_internal();

    char const * filename = get_phys_filename();
    
    if (OSapi_access(filename, R_OK)) {
      msg("ERROR: Cannot open \"$1\":$2") << filename << eoarg << OSapi_strerror(errno) << eom;
      return NULL;
    }
      
    if (!global_cache.is_ascii(filename))
	return NULL;

    char const *ftype;
    switch (language()) {
	case FILE_LANGUAGE_C:
	    ftype = "c";
	    break;
	case FILE_LANGUAGE_CPP:
	    ftype = "cplusplus";
	    break;
	case FILE_LANGUAGE_ESQL_C:
	    ftype = "esql-c";
	    break;
	case FILE_LANGUAGE_ESQL_CPP:
	    ftype = "esql-cplusplus";
	    break;

	case FILE_LANGUAGE_RAW:
	    ftype = "raw";
	    break;
	case FILE_LANGUAGE_UNKNOWN:
	    ftype = "unknown";
	    break;
	case FILE_LANGUAGE_MAKEFILE:
	    ftype = "makefile";
	    break;

	case FILE_LANGUAGE_ELS:
	    ftype = "els";
	    break;
	default:
	    ftype = "unknown";
	    break;
    }
    
    int write_on = (proj_node->get_xref()) ? proj_node->get_xref()->is_writable(): 0;
    projNode* old_proj =  projNode::get_current_proj ();
    
    if (write_on)
        projNode::set_current_proj (proj_node);
    else
        projNode::set_current_proj (projNode::get_control_project());

    if (language() == FILE_LANGUAGE_ELS)
      {
	int els_parse_file(const Relational*);

	int res = els_parse_file(this);
      }
#ifdef XXX_non_ELS_lang //    else
      browser_import (NULL, filename, ftype);
#endif // XXX: should throw 
    projNode::set_current_proj (old_proj);

    if ( (ah = get_app()) ) {
	if (write_on)
	    ah->set_read_only(0);
	else
	    ah->set_read_only(1);
	ah->set_phys_name ((char *)filename);

        ah->set_imported();
    }

    return ah;
}

void obj_unload(Obj*);

int projModule::unload_module()
// Unload the module from memory
{
    Initialize(projModule::unload_module);

// Unload raw version

//    projNode* project = get_project_internal();
//    genString fn; 
//    project->ln_to_fn (ln_name, fn);

    genString fn = get_phys_filename();
  
    if (fn.length()) {
        genString path;
        project_convert_filename(fn, path);
	appPtr raw_app;
	while (raw_app = app::get_header (App_RAW, path))
	    obj_unload (raw_app);
    }


// Unload paraset model

    appPtr ah = get_app();
    if (!ah)
    {
	update_module();
	return 0;
    }

    obj_unload (ah);

    invalidate_cache();

    if (ah){
        projNode* proj_node = get_project();
        projHeaderPtr proj_hdr = checked_cast(projHeader, proj_node->get_header());
        if (proj_hdr)
            obj_insert(proj_hdr, REPLACE,proj_node,proj_node,NULL);
    }
    if (!get_delete_flag()) {
      symbolPtr xsym = this;
      xsym = xsym.get_xrefSymbol();
      xref_notifier_report(0, xsym);
    }
    update_module();

    return 1;
}

projModule * app_get_mod(appPtr ah)
  /* does not handle raw header */
{
  if (ah == 0) return 0;
  projModule *mod = appHeader_get_projectModule(ah);
  if (mod == 0) {
    projNode *pr = app_get_proj(ah);
    if (pr) {
      char const *nm = (ah->get_type() != App_RAW) ? ah->get_filename() :0;
      if (nm)
	mod = pr->find_module(nm);
    }	
  }
  return mod;
}

//--------------------------------------------------------

int get_file_names (char const *ln, projNode* src_proj, projNode* dst_proj, 
    genString& src_file, genString& dst_file, 
    genString& src_paraset_file, genString& dst_paraset_file)
// get_file_names by logic name ln, the local project src_proj and the system project dst_proj
//
// returns the source file name: src_file
//         the system file name: dst_file
//         the source pset name: src_paraset_file
//         the system pset name: dst_paraset_file
{
    src_proj->ln_to_fn (ln, src_file);
    if (!src_file.length() && src_proj->is_script())
        src_proj->ln_to_fn(ln, src_file, FILE_TYPE, 0, 1);
    if (!src_file.length()) {
	msg("ERROR: No file mapping for logical name $1 in project $2") << ln << eoarg << src_proj->get_ln() << eom;
        return 0;
    }

    src_paraset_file = get_paraset_file_name (src_file, src_proj);

    if (!src_paraset_file.length()) {
        msg("ERROR: No DISCOVER file mapping for logical name $1") << ln << eom;
        return 0;
    }

    dst_proj->ln_to_fn (ln, dst_file);
    if (!dst_file.length())
        dst_proj->ln_to_fn(ln, dst_file, FILE_TYPE, 1, 1);
    if (!dst_file.length()) {
	msg("ERROR: No file mapping for logical name $1 in project $2") << ln << eoarg << dst_proj->get_ln() << eom;

        return 0;
    }
    dst_paraset_file = get_paraset_file_name (dst_file, dst_proj);
    if (!dst_paraset_file.length()) {
        msg("ERROR: No DISCOVER file mapping for logical name $1") << ln << eom;
        return 0;
    }

    return 1;
}

static void report_merge_error(char const *file, char const * /* paraset_file */ )
// print the warning for the merge error
//
// file: the source file name
// paraset_file: the pset file name
{
    Initialize(report_merge_error);

    msg("ERROR: Merge of file '$1' failed. Its pset file might be out of sync.") << file << eom;
}

static void backup_one_file(char const *file)
// create backup file for "file"
{
    Initialize(backup_one_file);

    genString s = OSPATH(file);
    genString cmd;
    char *q = (char *)strrchr(s.str(),'/');

    genString cp_from;
    genString cp_to;

    if (q) {
        *q = '\0';
	cp_to.printf ("%s/##%s#~", s.str(),q+1);
	cp_from.printf ("%s/%s", s.str(),q+1);
    }
    else {
	cp_to.printf ("##%s#~",s.str());
	cp_from = s.str();
    }

    shell_cp (cp_from, cp_to);
}

void backup_the_files(char const *file, char const *paraset_file, char func)
// create backup file for merge
//
// file: the source file name
// paraset_file: the pset file name
// func = 0: for merge
// func = 1: for get, copy, get old module
// func = 2: for 3 file merge
{
    Initialize(backup_the_files);

    if (!customize::configurator_do_backups() && func != 1)
        return;

    if (file && func != 0) // for 2-file merge no backup for source file
        backup_one_file(file);
    if (paraset_file)
        backup_one_file(paraset_file);
}

extern bool is_els_non_ascii_suffix(char const *);

int merge_file_type(char const *file)
// check if the file is source file that includes head files
{
    Initialize(merge_file_type);
 
    int res = 0;
    if (!file || !file[0]) return res;
 
    fileLanguage lang = guess_file_language (file);
 
    if (lang == FILE_LANGUAGE_UNKNOWN ||
	lang == FILE_LANGUAGE_EXT     ||
	lang == FILE_LANGUAGE_EXT_I   ||
	lang == FILE_LANGUAGE_EXT_X )
	res = 0;
    else if (lang == FILE_LANGUAGE_ELS) {
	res = (is_els_non_ascii_suffix(file)) ? 0 : 1;
    } else
	res = 1;
 
    return res;
}
 
extern  appPtr db_restore_update_xref(projModule *pm);
int projModule::merge_module(projModule *pm, char const *file, char const *paraset_file, char type,
                             char backup_flag, char unload_flag, int &do_not_save_flag)
// set up to merge 2 files: file and paraset_file (when C & pset files are out of sync)
//
// pm: is the projmodule for this file
//
// file: is the source file name
//
// paraset_file: is the source pset file name
//
// type = 0 from paraset interactive
//      = 1 from -batch mode
//
// backup flag: create backup copy of both
//
// unload_flag: unload pm after merge.
//
// do_not_save_flag: if 0, and it needs save, do the save at the end of merge
//                 : if 1, and it needs save, set do_not_save_flag to 2 and don\'t do the save
{
    Initialize(projModule::merge_module);

    if (!merge_file_type(file))
        return 0;

    genString fn;
    pm->get_phys_filename(fn);

    // == 0 identical
    // == -2 no source file
    // == -1 fail to read source file
    if ((::outdated_pset(file, paraset_file, fn)) <= 0)
        return 0;

    // check if empty token table
    app* app_h = pm->get_app();
    if (!app_h) {
	if (disbuild_analysis_mode()) {
	    genString pn;
	    pm->get_phys_filename(pn);
	    if (pn.str())
		app_h = ::db_restore(pn, pm->get_name());
        } else {
	    app_h = db_restore_update_xref(pm); // ::db_restore(fn);
        }
    }

    if (!app_h)
        return 0;

    if (is_smtHeader(app_h))
    {
        smtHeader *sh = checked_cast(smtHeader, app_h);
        sh->get_root();
        IF (sh->ttablth == 0 && sh->src_asize != 0)
            return 0;
    }
    // check if empty token table

    struct OStype_stat buf0;
    int status0 = global_cache.stat(file,&buf0);
    int chmod_status0=0;
    int chmod_status1=0;

    if (status0 == 0)
    {
      chmod_status0 = OSapi_chmod(fn.str(),buf0.st_mode|S_IWRITE);
    }

    struct OStype_stat buf1;
    int status1 = global_cache.stat(paraset_file,&buf1);
    if (status1 == 0)
    {
        if (chmod_status1 = OSapi_chmod(paraset_file,buf1.st_mode|S_IWRITE) < 0)
        {
            if (chmod_status0 == 0)
                OSapi_chmod(fn.str(),buf0.st_mode);
        }
    }
 
    push_busy_cursor();
    // copy the backup file
    if (backup_flag)
        backup_the_files(file,paraset_file,0);

    smtHeader *sh = checked_cast(smtHeader,app_h);
    int merge_status = char_based_merge (app_h, file, paraset_file, unload_flag, do_not_save_flag);

    if (status0 == 0 && chmod_status0 == 0)
        OSapi_chmod(fn.str(),buf0.st_mode);

    if (!merge_status) // fail
    {
        // remove the pset
        if (!backup_flag)
            backup_the_files(0,paraset_file,0);
        OSapi_unlink(paraset_file);
        // give an warning
        report_merge_error(file, paraset_file);
    }
    else // succeed
    {
        if (status1 == 0 && chmod_status1 == 0)
            OSapi_chmod(paraset_file,buf1.st_mode);
    }

    pop_cursor();
    if (pm)
        pm->invalidate_cache();
    return 1;
}


char const *projModule::get_app_ln()
{
    Initialize(projModule::get_app_ln);

    return app_ln;
}

projModule *projModule::create_script_module(projNode *pn, char const *ln)
{
    Initialize(projModule::create_script_module);

    projNode *cur = pn;

    while(cur && !cur->get_map())
        cur = cur->find_parent();

    if (!cur)
        return 0;

    projMap *pm = cur->get_map();
    update_path_hash_tables(pm, pn);
    genString fn;
    pn->ln_to_fn(ln, fn);
    projModule *dst_mod = proj_make_module(fn, pn, ln);
    pn->insert_module_hash(dst_mod);
    return dst_mod;
}

void projModule::setup_SWT_Entity_Status(app *ah)
{
  Initialize(projModule::setup_SWT_Entity_Status);
  if (ah == 0)
    ah = get_app();

  if (ah == 0) 
      return;

  projNode *proj = get_project();
  ddRoot * dr = checked_cast(ddRoot, get_relation(ddRoot_of_smtHeader, app_head));
  if (dr == 0) return;
  dr->setup_SWT_Entity_Status(1);
  Xref * Xr = proj->get_xref();
  if (Xr) 
    Xr->insert_SWT_Entity_Status(ah);
}

void projModule::get_destination_candidates(objArr& dst_projs)
{
    Initialize(projModule::get_destination_candidates);

    projNode* this_proj = this->get_project();

    projNode* proj;
    for (int i=1; proj = projList::search_list->get_proj(i); i++) {
	
	projNode *parent;
	for  (parent = this_proj; parent; parent = parent->find_parent()) {
	    if (proj == parent) break;
	}
	if (parent) continue;

	genString fn;
	proj->ln_to_fn (ln_name, fn);
	if (fn.length())
	    dst_projs.insert_last (proj);
    }
    if (dst_projs.size() == 0)
	dst_projs.insert_last (this_proj);
}

void projModule::dependency_analysis(app* head, projNode *dst_proj, int not_save_flag)
// collect the changed ddElements
//
// head: the app header for this module
// dst_proj: the projnode this module belongs to
// not_save_flag: if set, don"t save the changess because the .pset file will be
//		deleted.
{
    Initialize(projModule::dependency_analysis);

    if (head && XREF_check_files_including(ln_name))
    {
	char save_flag = 0;
	int res = dd_dependency_analysis (head, save_flag);
	if (res == -1) {
	    msg("Cannot do dependency analysis for file $1") << (char *)ln_name << eom; 
	    msg("   Affected files should be examined manually") << eom;
	} else {
	    if (save_flag && !not_save_flag)
		::db_save(head, NULL); // update the pset file.
	    fine_grained_dependency_analysis(dst_proj);
	}
    }
}


// projModule::save_module
//
// Saves the module to disk - translates to writable project
int projModule::save_module()
{
    appPtr ah = get_app();
    bool flag = false;

    if (ah) {

	if (ah->is_modified())
	    flag = true;

	int status = ::db_save (ah, NULL); // second parameter can be 
	                                   // physical name for saved file
	invalidate_cache();

        return status;
    } else
	return 0;
}

int projModule::delete_module()
// delete a module
{
    Initialize(projModule::delete_module);

    projNode *par = get_project();
    genString fn;
    get_phys_filename(fn);

    int status = delete_module(0, 0);

    if (par && par->is_script())
    {
        struct OStype_stat stat_buf;
        if (global_cache.stat(fn.str(),&stat_buf) != 0)
        {
            // file not exist. It is not an element yet before it is deleted.
            projNode *proj;

            for (int i=1; proj = projList::search_list->get_proj(i); i++)
            {
                if (proj == par)
                    continue;
                projModule *mod = proj->find_module(ln_name);
                if (mod)
                    mod->delete_module(1, 0);
            }
        }
    }

    return status;
}

int projModule::delete_module (int new_flag)
// delete a module
//
// new_flag = 0: deletes project module, app, and disk file
// new_flag = 1: force_refresh_flag (file is deleted before come here)
// new_flag = 2: 3 file merge for put and for 2 file merge
{
    Initialize(projModule::delete_module);
    return delete_module (new_flag, 0);
}

int projModule::delete_module (int new_flag, int tp)
{
    return delete_module_internal(this, new_flag, tp);
}

bool delete_module_internal_call_cm (projModule *, int, genString, genString,
				     genString, genString);
void delete_module_internal_call_triggers (projModule *, int, int);
void delete_module_internal_finish (char const *, projNode *);

static int delete_module_internal(projModule* This, int new_flag, int type)
// delete a module
//
// new_flag = 0: deletes project module, app, and disk file
// new_flag = 1: force_refresh_flag (file is deleted before come here)
// new_flag = 2: 3 file merge for put and for 2 file merge
// new_flag = 3: same as 1 and don"t remove from pmod which is for reloading pdf
//
// type = 0: delete_module
// type = 1: unget_module
{
    Initialize(delete_module_internal);

    char const *name = This->get_name();

    projNode* dest_proj = This->get_project();
    projNode* non_home_proj = projHeader::non_current_project(name);

    if (!dest_proj) return 0;

    if (!non_home_proj)
	non_home_proj = dest_proj;

    genString src_file;
    genString src_paraset_file;
    genString dst_file;
    genString dst_paraset_file;

    if (!get_file_names (name, non_home_proj, dest_proj	, src_file, dst_file,
	src_paraset_file, dst_paraset_file))
        return 0;

    if (new_flag == 2 || (new_flag == 0 && (customize::no_cm() || !is_aset())))
    {
	OSapi_unlink(dst_file);
	OSapi_unlink(dst_paraset_file);
    }
    else if (new_flag == 0)
    {
	if (new_flag == 2 || customize::no_cm())
	{
	    OSapi_unlink(dst_file);
	    OSapi_unlink(dst_paraset_file);
	}
	else
	{
	    if (delete_module_internal_call_cm (This, type, src_file, dst_file,
						src_paraset_file,
						dst_paraset_file))
	    {
		return 1;
	    }
	}
    }

    // remove paraset file

    start_transaction() {
	This->set_delete_flag(1);
	This->unload_module(); // -1 for delete, remove from view
	This->set_delete_flag(0);	
	This->invalidate_cache();
	obj_delete(This);
        if (new_flag != 3)
	{
	    // must remove from xrefs after deleting app and module
	    Xref *xref = dest_proj->get_xref();
	    if (xref)
	    {
		delete_module_internal_call_triggers (This, new_flag, type);
		xref->remove_module (name);
		if (new_flag != 2)
		    xref->remove_module_from_xref (name);
	    }
	}
    } end_transaction();

    delete_module_internal_finish (name, dest_proj);
    return 1;
}

// projModule::language
//
// Sets the module's language type

void projModule::language (fileLanguage new_lang)
{
    Initialize(projModule::language__fileLanguage);
    if (new_lang == commonTree::language)
	return;
    
    if (disbuild_analysis_mode()) {
	commonTree::language = new_lang;
        return;
    }
	
    projNode *pr = this->get_project();
    if (!pr || pr->root_project() != projNode::get_home_proj()->root_project()) {

	msg("You can only change modules belonging to your home project.", error_sev) << eom;
	return;
    } 
 
    symbolPtr syminxref = NULL_symbolPtr;

    app *ah = this->get_app();

    int symbol_found = 0;
    Xref* xr = pr->get_xref(1);
    if (xr) {
	symbolArr res;
	syminxref = lookup_file_symbol_in_xref(this->get_name(), xr);
	if (syminxref.xrisnotnull())
 	   symbol_found = 1;
    }
    
    symbolPtr xref_symb = pr->lookup_symbol (DD_MODULE, (char *)ln_name);
    symbolPtr xref_syminxrefb = syminxref.get_xrefSymbol();

    switch (commonTree::language) {
      case FILE_LANGUAGE_C:
      case FILE_LANGUAGE_CPP:
      case FILE_LANGUAGE_FORTRAN:
      case FILE_LANGUAGE_COBOL:
      case FILE_LANGUAGE_ESQL_C:
      case FILE_LANGUAGE_ESQL_CPP:
      case FILE_LANGUAGE_UNKNOWN:
	switch (new_lang) {
	  case FILE_LANGUAGE_C:
	  case FILE_LANGUAGE_CPP:
	  case FILE_LANGUAGE_FORTRAN:
	  case FILE_LANGUAGE_COBOL:
	  case FILE_LANGUAGE_ESQL_C:
	  case FILE_LANGUAGE_ESQL_CPP:
	    break;
	    
	  default: {

	      if (this->get_app() || this->is_paraset_file()) {
		  int unload_flag = dis_question (T_CHANGELANG,
						  B_CHANGEANYWAY, B_CANCEL,
						  Q_LOSEASSOCCHANGE, get_name());
		  if (unload_flag == 1) {
		      unload_module();
		      OSapi_unlink (paraset_file_name());
		      invalidate_cache();
		      xr->remove_module (ln_name);
		      xr->remove_module_from_xref (ln_name);
		  }
	      }
          }
	  break;
	}
	break;
	
      case FILE_LANGUAGE_RAW: {
	  if (this->get_app() || this->is_paraset_file()) {
	      int unload_flag = dis_question (T_CHANGELANG,
					  B_CHANGEANYWAY, B_CANCEL,
					  Q_LOSEASSOCCHANGE, get_name());
	      if (unload_flag == 1) {
		  unload_module();
		  OSapi_unlink (paraset_file_name());
		  invalidate_cache();
		  xr->remove_module (ln_name);
		  xr->remove_module_from_xref (ln_name);
	      }
	  }
        } 
	break;
    }
    
    if (symbol_found)          xref_syminxrefb->set_language (new_lang, xr->get_lxref());
    if (xref_symb.xrisnotnull()) xref_symb->set_language (new_lang, xr->get_lxref());
    if (ah)              ah->language = new_lang;

    commonTree::language = new_lang;

    projHeader* h = this->get_header();
    obj_insert (h, REPLACE, pr, pr, NULL);
}

// projModule::is_home_project()
//
// Test to see if module is the home project

bool projModule::is_home_project() const
{
  projNode* proj = get_project();
  projNode* proj_root = proj->root_project();

  projNode* home = projNode::get_home_proj();
  projNode* home_root = home->root_project();

  return (proj_root==home_root);
}

 
bool projModule::is_project_writable() const
// check if this module belongs to a writable projnode
{
    projNode *pn = get_project();

    if (pn)
    {
        Xref *xr = pn->get_xref();
        if (xr && xr->is_project_writable())
            return TRUE;
    }
    return FALSE;
}


// projModule::is_loaded
//
// Test to see if the module is loaded into memory

bool projModule::is_loaded()
{
    if (get_app())
	return 1;
    else
	return 0;
}

// projModule::is_modified
//
// Test to see if the memory copy of the app has been modified

bool projModule::is_modified()
{
    appPtr ah = get_app();
    if (ah)
	return ah->is_modified();
    else
        return 0;
}


// projModule::is_parsed
//
// ???


appPtr projModule::find_app ()
// return app header for this module
{
    appPtr app_ptr =  get_app();
    if (app_ptr) return app_ptr;
    
    genString fn;
    get_phys_filename(fn) ;
    app_ptr =  app::find_from_phys_name(fn);
    
    return app_ptr;
}


app *projModule::get_symbolPtr_def_app ()
{
    Initialize(projModule::get_symbolPtr_def_app);
    app *prh = get_app();
    return prh;
}


void propagate_modified_headers() {
   Initialize(propagate_modified_headers);

   symbolArr hdrs = Hierarchical::prop_rtl_contents();
   Hierarchical::clear_prop_rtl();
   symbolPtr hdr;
   ForEachS(hdr, hdrs) {
      if (hdr.relationalp()) {
	 RelationalPtr objp = hdr;
	 if (is_app(objp)) {
	   projModulePtr mod = appHeader_get_projectModule( (app *)objp );
	   /*   projModulePtr mod = appPtr(objp)->get_module(); */
	    if (mod) {
	       obj_insert(checked_cast(app, mod->get_header()), REPLACE,
			  (appTree*) mod, (appTree*) mod, (void*) NULL);
	    }
	 }
      }
   }
}

//-----------------------------------------------------------------------------
//
// Set of functions  for processing CM-events receiving from NT
//
// All functions have 1 parameter: 
//               char const* fname_list
// null terminate string contaned list of full phisical file names
// separated by `\n`
//
// All functions must return 0 in case of success
//

typedef projModule* (*projModuleptr_func) (symbolPtr&);
 
int cli_question_save_all(symbolArr&);

int dis_confirm_and_save_for_reparse(symbolArr& apps)
{
  int answer = 1;
  
  if (apps.size() == 0)
    return answer;
  
  answer = cli_question_save_all(apps);

  if (answer != 1)
    return answer;

  symbolPtr sym;
  ForEachS(sym,apps) {
    app* ah = (app*)sym;
    ::db_save(ah,NULL);
  }

  return answer;
}

void ste_interface_modified( HierarchicalPtr, boolean );

int els_reparse_modules(symbolArr& modules, symbolArr& rest)
{
  Initialize(els_reparse_modules(symbolArr&,symbolArr&));

  int retval = 0;

  objArr* apps = app::get_modified_headers();

  symbolArr modified;

  { 
    Obj* el;
    ForEach (el,*apps) 
      {
	appPtr appptr = checked_cast(app,el);
	if (is_objRawApp(appptr) && appptr->is_modified() ||
	    is_smtHeader(appptr) && appptr->is_src_modified())
	  modified.insert_last (appptr);
      }
  }

  if (dis_confirm_and_save_for_reparse(modified) == 1)
    {
      start_transaction()
	{
	  errorBrowserClear();
	  symbolPtr sym;
	  int flag_suspend = 0;
	  ForEachS(sym,modules)
	    {
	      projModule *pm = symbol_get_module(sym);
	      fileLanguage type = pm->language();
	      app* ah = pm->get_app();
	      if (ah && is_smtHeader(ah)) {
		ste_temp_view_suspend ((smtHeader*)ah);
		flag_suspend = 1;
	      }
	      bool code = true;
	      if (type == FILE_LANGUAGE_ELS)
	      {
		els_parse_file(pm);
		code = false;
	      }
	      else if (type == FILE_LANGUAGE_C ||
		       type == FILE_LANGUAGE_CPP ||
		       type == FILE_LANGUAGE_ESQL_C ||
		       type == FILE_LANGUAGE_ESQL_CPP) 
		code =  els_parse_edg(pm);
	      
	      if (code)
		rest.insert_last(sym);
	      else
		{
		  ah = pm->get_app();
		  if (ah && is_smtHeader(ah) && flag_suspend) {
		    ste_smart_view_update ((smtHeader*)ah );
		     ste_interface_modified (ah, 0);
		  }
		}
	    }
	  retval = rest.size();
	}
//      errorBrowser_show_errors();
      end_transaction();
    }

  delete apps;
  return retval;
}


//===========================================================
#ifdef NEW_UI
#include <Application.h>
#include <DIS_ui/interface.h>
#include "tcl.h"
#include "Interpreter.h"
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#include <fstream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#include <fstream>
#endif /* ISO_CPP_HEADERS */
//--------------------
//
int cli_process_input(char *data, ostream&, Interpreter *, int do_prompt_and_print);

static char szClientFileName[257] = "";
static ofstream null_stream("nul", ios::out);
static Interpreter* interpClient;

//$ideHandle $back $wv $cmd $arg

int dis_ide_command_event(int argc, char *argv[] )
{
  Initialize(dis_ide_proj_event);
	genString res;

	if ( argc >= 6 )
	{
		if ( OSapi_strcmp( argv[4], "init") == 0 )
		{
			char const *pszTemp = OSapi_getenv("TEMP");
			if ( pszTemp && *pszTemp )
			{
				interpClient = Interpreter::Create();
				res = OSPATH(pszTemp); 
			}
		}
		else
		{
			genString command = argv[4];
			for (int i=5 ; i< argc; i++ )
			{
				command += " ";
				command += argv[i];
			}
			command += "\n";
			cli_process_input(command, null_stream, interpClient, 1);
			if( interpClient->command.length()==0)
				interpClient->GetResult(res);
		}

		if ( res.length() && strcmp( argv[2], "no_cmd") !=0 )
		{
			genString str;
			str.printf ("dis_ide_send_command {dis_ide_command_answer %s %s %s \"%s\"}", 
									argv[1], argv[2], argv[3], res.str() );
			rcall_dis_DISui_eval_async (Application::findApplication("DISui"), (vchar *)(char *)str);
		}
	}
  return 0;
}

#endif

//boris created hash for realOSPATH
//-----------------------------------------------------------------

#ifdef _WIN32
#include <Hash.h>

class realosItem : public namedObject {
    genString realosFName;
public:

    realosItem(char const *phy, char const *rn) : namedObject(phy), realosFName(rn) { }
    char const *get_realosFName() { return (char const *)realosFName; }
};

class realosHash : public nameHash {
public:
    virtual char const* name(const Object*)const;
    virtual bool isEqualObjects(const Object&, const Object&) const;
};


bool realosHash::isEqualObjects(const Object& o1, const Object& o2) const
{
    realosItem *f1 = (realosItem *)&o1;
    realosItem *f2 = (realosItem *)&o2;

    char const *n1 = f1->get_name();
    char const *n2 = f2->get_name();

    bool res = (strcmp(n1,n2) == 0) ? 1 : 0;
    return res;
}

char const *realosHash::name (const Object*o)const
{
    realosItem *f = (realosItem *)o;
    return (char const *)f->get_name();
}


static realosHash realosNames;

static char const *get_realos_name (char const *phy) 
{
    char const *line  = 0;
    Object     *found = 0;
    int x             = 0;
    
    realosNames.find (phy, x, found);
    if (found) {
	    realosItem *f = (realosItem *)found;
	    line = f->get_realosFName ();
    }
    return line;
}

char *DisFN_realOSPATH(char *path);

char const *realOSPATH_hashed(char const *path)
{
  if ( !path || !*path || *path == '*' || (*path == '('&& *(path+1) == '*') )
    return path;

  char const * realos = get_realos_name (path);
  if (realos && realos[0]) 
    return (char *) realos;    // returning permanent char const * from hash table

  char *ret_path = DisFN_realOSPATH( (char *)path );

  realosItem *ri = new realosItem(path, ret_path);
  realosNames.add(*ri);
  return (char *)ri->get_realosFName();
}

#endif

