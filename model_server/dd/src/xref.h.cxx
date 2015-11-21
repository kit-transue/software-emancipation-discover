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
#undef MAX
#if defined(_WIN32) || defined(hp10)
#include <errno.h>
#endif

#include <msg.h>
#include <xxinterface.h>
#include <fileEntry.h>
#include <dfa_graph.h>
#include "cLibraryFunctions.h"
#include "Question.h"
#include "messages.h"
#include <genTry.h>
#include <genString.h>
#include <genTmpfile.h>
#include <customize.h>
#include <ddict.h>
#include <xref.h>
#include <xref_priv.h>
#ifdef NULL
#undef NULL
#endif
#define NULL 0

#include <db.h>
#include <saveXref.h>
#include <path.h>
//#include <steDocument.h>
#include <db_intern.h>
#include <fileCache.h>
#include <subsys.h>
#include <timer.h>
#include <setprocess.h>
#include <MemSeg.h>
#include <machdep.h>
#include <psetmem.h>
#include <shell_calls.h>
#include <tcl.h>
#include <disbuild_analysis.h>
#include <driver_mode.h>

#define  SEEK_END  2

typedef int (*purify_call)();
purify_call purify_new_inuse_ptr;
purify_call purify_new_leaks_ptr;

void load_pdfs_complete();

extern void inc_num_files_inserted_in_xref();
#ifdef NULL
#undef NULL
#endif
#define NULL 0

void xref_set_doing_second_pass(int);

static bool initDFA = false;

bool
isDFAinit()
{
  return initDFA;
}

static bool dump_psets = false;

void
show_arena(char const *full_path)
// conditionally display time & memory usage to stdout
// For internal use;  setenv PSET_ARENA show  to activate this display
{
#if !defined(sun5) && !defined(irix6) && !defined(_WIN32) && !defined(__MC_MACOSX__)
  genString hack = OSapi_getenv("PSET_ARENA");

  if ((char *) hack && !strncmp(hack, "show", 4)) {
    struct mallinfo info = mallinfo();
    static int filecount = 0;
    OSapi_fprintf(stdout, "...After %d , %s, arena size = %d\n", ++filecount, full_path, info.arena);
    OSapi_fprintf(stdout, "   SharedMem = %d\n", builtin_mmgr->get_seg_used());
    time_t date = OSapi_time((time_t *) 0);
    OSapi_fprintf(stdout, "   time is %s\n", OSapi_ctime(&date));
  }
#endif
}

static int batch_cpp_errors = 0;

void
set_batch_cpp_errors(int i)
{
  batch_cpp_errors = i;
} // set in smt_ast


int cli_eval_string(const char* cmd);

app* projModule_try_to_restore(projModule*);

/* returns false on success, true on failure.  (!?) */
bool
els_parse_edg(projModule *m)
{
  genString cmd;
  app* ah = m->get_app();

  if (!ah && is_model_build()) {
    ah = projModule_try_to_restore(m);
    if (ah) {
      obj_unload(ah);
      return false;
    }
  } 
#ifndef _WIN32
  else if (ah && is_smtHeader(ah) && !((smtHeader*)ah)->arn)
    return false;
#endif

  symbolPtr mod = m;
  cmd.printf("dis_parse_edg_file 0X%X", &mod);
  int code = cli_eval_string(cmd);
  inc_num_files_inserted_in_xref();

  return
    (code == TCL_CONTINUE);
}

int els_parse_file(const Relational*);

int els_parse_or_restore_file(projModule *m)
{
  app* ah = m->get_app();

  if (!ah && is_model_build()) {
    ah = projModule_try_to_restore(m);
    if (ah) {
      obj_unload(ah);
      return TCL_OK;
    }
  }
  return els_parse_file(m);
}

static void
load_one_file(projModule *module,
	      char *full_path,
	      fileLanguage type,
	      int save_files, 
	      int save_includes,
	      int update_xref)
// load one source file in pass 2 for model build
// load one header file in pass 3 for model build
//
// projnode: this file belongs to
// path is the logical name
// full_path is the physical path name
// type: the language type of this file: (*.[cChH])
// save_files: if set, save the source file (the .pset file); otherwise, no save
// save_includes: if set, save the header file (the .pset file); otherwise, no save
// update_xref: if set, update pmod; otherwise, do nothing for pmod
{
  Initialize(load_one_file);

  timer::init(1, "load_one_file", full_path);
  set_batch_cpp_errors(0);

  if (type == FILE_LANGUAGE_UNKNOWN) {
    if (xref_not_allowed_file_type(full_path))
      msg("Warning:The file $1 is not in a recognized language, so we are not parsing it.") << full_path << eom;

    return;
  }

  if (update_xref) // for -C and -c
    module->update_xref();

  if (type == FILE_LANGUAGE_ELS) {
    els_parse_or_restore_file(module);
    return;
  } else if (type == FILE_LANGUAGE_C ||
	     type == FILE_LANGUAGE_CPP ||
	     type == FILE_LANGUAGE_ESQL_C ||
	     type == FILE_LANGUAGE_ESQL_CPP) {
    bool code =  els_parse_edg(module);
  }
  timer::showtime(1, "load_one_file", full_path);
}

static int update_xref_flag = 0;

bool
do_delete_pset()
{
  return 
    (update_xref_flag != 1); // for -u
}

extern char SUBSYSTEM_ROOT[];

static bool is_subsys_logical_path(const char* path) {
   Initialize(is_subsys_logical_path);
   int length = strlen((char *) SUBSYSTEM_ROOT);
 
   return
     (strncmp(path, SUBSYSTEM_ROOT, length) == 0);
}

extern void proj_get_modules(projNode *proj, objArr &modules);
extern bool projModule_dump_pset(projModule *);

static void
load_all_files(projNode *pn,
	       projNode *projnode,
	       int save_files, 
	       int save_includes)
//
// pn: is the root projnode
// projnode: is the sub-projnode
// save_files = 1: save source file
//            = 0: don"t save source file
// save_includes = 1: save header files
//               = 0: don"t save header files
{
  Initialize(load_all_files(projNode *, projNode *, int, int, int));

  if (!projnode || !projnode->is_writable())
    return;

  objArr modules;
  GDSet *hashtable = NULL;
  proj_get_modules(projnode, modules);
  int no_modules = modules.size();
  int sym_res = projNode_set_pmod_names(projnode);
  bool undoDFA = true;

  if (initDFA) {
    if (sym_res != 0 && projnode->sym_name) {
      genString fileName = projnode->sym_name->get_name();
      ((char *) fileName)[fileName.length() - 3] = '\0';
      fileName += "dfa";
      OSapi_unlink((char *) fileName);
    }

    if (_lo(LIC_DFA)) {
      _lm(LIC_DFA);
      return;
    }
  }

  for (int ii = 0; ii < no_modules; ++ii) {
    projModule *module = checked_cast(projModule, modules[ii]);
    char const *path = module->get_name();

    if (dump_psets) {
      if (is_subsys_logical_path(path))
	continue;

      projModule_dump_pset(module);
      continue;
    }

    if (initDFA) {
      if (is_subsys_logical_path(path))
	continue;

      if (!hashtable) {
	hashtable = new GDSet();
	project_put_table(projnode, hashtable);
	GDSet::tableSet_.insert_last(hashtable);
      }

      symbolPtr fileSymbol(module);
      fileSymbol = fileSymbol.get_xrefSymbol();

      if (fileSymbol.isnull())
	continue;

      GDGraph *graph = hashtable->find(fileSymbol, false);

      if (graph)
	continue;

      module->restore_module();
      app *app_head = module->get_app();

      if (!app_head)
	continue;

      if (is_smtHeader(app_head)) {
	smtHeader *smt = smtHeaderPtr(app_head);
	astRoot *root = (astRoot *) get_relation(astRoot_of_smtHeader, smt);

	if (root)
	  hashtable->generateFlow(root, true, false);
      }

      msg("Unloading $1 ... ") << app_head->get_name() << eom;
      cout.flush();
      obj_unload(app_head);
      msg("done\n") << eom;
      cout.flush();
      continue;
    }

    genString phys_name;
    module->get_phys_filename(phys_name);
    int do_it = 1;

    int need_reparse = db_get_bit_need_reparse_no_forgive(phys_name);

    if (!need_reparse && module->outdated_pset())
      need_reparse = 1;

    // checkout not for pass 2 and pass 3
    if (need_reparse && do_delete_pset() && xref_not_txt_file(phys_name)) {
      genString pset_file = get_paraset_file_name(phys_name, module);
      OSapi_unlink(pset_file);
    }

    if (is_subsys_logical_path(path))
      continue;

    if (!need_reparse) {
      symbolPtr check_newer = Xref_file_is_newer(path, phys_name);
      do_it = check_newer.isnull();
    }

    if (do_it && !xref_not_header_file(phys_name)) {
      //  update header files if they are included somewhere
      do_it = XREF_check_files_including(path);
    }

    if (do_it) { // really do it
      load_one_file(module, phys_name, module->language(), 
		    save_files, save_includes, 0);

      if (undoDFA && sym_res != 0 && projnode->sym_name) {
	genString fileName = projnode->sym_name->get_name();
	((char *) fileName)[fileName.length() - 3] = '\0';
	fileName += "dfa";
	OSapi_unlink((char *) fileName);
	undoDFA = false;
      }
    }
  }

  if (initDFA && hashtable)
    GDSet::saveDFA(hashtable, GDT_LOCAL, true);
}


symbolArr dep_mod_name;

void
add_outdated_module(projModule *module)
{
  Initialize(add_outdated_module);

  if (disbuild_analysis_mode())
      disbuild_analysis_outdated_module(module);
  else
      dep_mod_name.insert_last(module);
}

void
add_dependency_module(projNode *proj,
		      symbolPtr &file)
{
  Initialize(add_dependency_module);
  char const *ln = file.get_name();

  projModule *module = proj->find_module(ln);

  if (module) {
      if (disbuild_analysis_mode())
	  disbuild_analysis_outdated_module(module);
      else
	  dep_mod_name.insert_last(module);
  } else
    msg("Cannot find module $1", error_sev) << ln << eom;
}

bool do_delete_pset();
bool module_is_not_viewable(projModule *);

bool app_is_not_viewable(appPtr app_head)
{
    return module_is_not_viewable(app_head->get_module());
}

void
output_dependency_modules(char const *outfile)
{
  Initialize(output_dependency_modules(char const *outfile));

  dep_mod_name.usort();
    int fp = -1;

  if (outfile && *outfile) {
    fp = OSapi_open(outfile, O_RDWR|O_CREAT, 0644);

    if (fp < 0) {
      msg("Cannot open file $1 for reporting dependency analysis", error_sev) << outfile << eom;
    }
  }

  int sz = dep_mod_name.size();

  if ( sz > 0 ) {
    if (OSapi_getenv("DIS_DEBUG_MRG")) {   // debug mode
      msg("DIS_DEBUG_MRG: skip -dependency actions\n") << eom;
    } else {
      int delete_pset = do_delete_pset();

      for (int ii = 0; ii < sz; ii++) {
	projModule *module = checked_cast(projModule, dep_mod_name[ii]);

    	if (module_is_not_viewable(module))
          continue;

	module->touch_module(delete_pset);
	const char *fn = module->get_phys_filename();
	msg(" touch $1 $2\n") << fn << eoarg << module->get_name() << eom;    

	if (fp < 0)
	  continue;
	       
	OSapi_write(fp, (char *) fn, strlen(fn));
	OSapi_write(fp, "\n", 1);
      }
    }

    dep_mod_name.removeAll();

    if (fp >= 0)
      OSapi_close(fp);
  }
}

void pr_bad_proj_name(const char *projname);

void
dd_build_xref(int l_num,
	      char const **l_files,
	      fileLanguage *types,
	      int r_num,
	      char const **r_files,
	      char const *projname,
	      int save_files,
	      int save_includes,
	      int update_xref,
	      char const **visible_projects,
	      char const *report_file)
{
  Initialize(dd_build_xref);
 
  update_xref_flag = update_xref; /* set no_delelete flag */

  genString path;
  int create_smt = save_files || save_includes;

  set_build_xref_flag(3);

  // add all root projects in the scan list
  projList::search_list->scan_projects("/");

  // get to project node from project name
  projNode *projnode = projNode::find_project (projname);

  if (!projnode) {
    pr_bad_proj_name(projname);
    return;
  }

  if (projnode->get_xref(1) && !projnode->get_xref()->is_unreal_file())
    ;
  else {
    Xref *proj_xref = projnode->get_xref();

    /* check for parents pmod */
    if (proj_xref && !proj_xref->is_unreal_file() && !projnode->get_xref(1)) {
      projNode *par = proj_xref->get_projNode();

      if (par)
	msg("Error: parent project $1 of subproject $2 contains a pmod;\n       projects and their subprojects cannot both have pmods.") << par->get_name() << eoarg << projname << eom;

      return;
    } else if (Xref::test_subproj_pmod(projnode) == -1) {
      /* check for sub project pmod     */
      msg("SE:Error: project $1 has one or more subprojects that contain a pmod\n       projects and their subprojects cannot both have pmods.") << projname << eom;
      return;
    } else if (!projnode->get_xref(1)) {
      genString xref_file;
      projnode->xref_file_name(xref_file);

      if (xref_file.length())
	projnode->set_xref(new Xref(xref_file, 1, projnode));
    }
  }

  //boris, 102697: refresh the coresponding top project to fix wrong hidding
  //       It is vital for the -headers path and modelbuild without .prj files
  projNode *root = projnode->root_project();
  root->refresh_all();

  // set the projnode to be current project
  static int complete_pdf;
  if ((update_xref == 6 || update_xref == 7) && projNode::get_home_proj())
    projNode::set_home_proj_temp (projnode);
  else
    projNode::set_home_proj (projnode);

  if (!complete_pdf) {
    load_pdfs_complete();
    complete_pdf = 1;
  }

  if (projnode->get_xref() && customize::get_write_flag())
    if (update_xref != 6 && update_xref != 7)
      projnode->get_xref()->make_room(2000000);

  // set visible projects
  if (visible_projects[0] != NULL) {
    symbolArr all_projs, visible_projs;
    projList::search_list->get_search_list(all_projs);

    for (int ii = 0; visible_projects[ii]; ++ii) {
      symbolPtr s;

      ForEachS(s, all_projs) {
	char const *name = s.get_name ();

	if (name && !strcmp(name, visible_projects[ii])) {
	  visible_projs.insert_last(s);
	  break;
	}
      }
    }

    projList::search_list->set_search_list(visible_projs);
  }

  // get the xref from project node
  Xref *xref = projnode->get_xref();

  if (!xref || (!xref->is_writable() &&
		update_xref != 6 && update_xref != 7)) {
    //  either no xref, or we don't have write-access to it
    // suppress the message if the user is not interested in writing pmods
    if (customize::get_write_flag())
      msg("Error: Could not write to pmod files for project $1") << projname << eom;

    return;
  }

  // Make sure that we scan this project first
  projList::search_list->scan_project_first(projnode);

  // update all .pset files in xref
  if (update_xref) {  // 1-u, 2-dependency, 3-proj, 4-included_files 5-metrics 6-dfa
    if (update_xref <= 2) {
      projnode->refresh_all();
      msg("===== Analysis pass =====") << eom;
      timer::init(0, "Analysis pass", "");
      projnode->update_xref();
      timer::showtime(0, "Analysis pass", "");
      l_num = 0; // skip the next for block
    }

    if (update_xref == 1 || update_xref == 3) {
      msg("===== Update pass =====") << eom;
      timer::init(0, "Update pass", "");
      xref_set_doing_second_pass(1);
      load_all_files(projnode, projnode, save_files, save_includes);
      xref_set_doing_second_pass(0);
      timer::showtime(0, "Update pass", "");
    } 

    if (update_xref == 1 || update_xref == 4) {
      msg("===== Headers pass =====") << eom;
      timer::init(0, "Headers pass", "");
      load_all_files(projnode, projnode, save_files, save_includes);
      timer::showtime(0, "Headers pass", "");
    }

    if (update_xref == 5) { 
      projnode->refresh_all();
      msg("===== Metrics pass =====") << eom;
      timer::init(0, "Metrics pass", "");
      projnode->update_metrics();
      timer::showtime(0, "Metrics pass", "");
      l_num = 0; // skip the next for block
    }

    if (update_xref == 6) {
      msg("===== DFA pass =====") << eom;
      if (_lo(LIC_DFA)) {
	_lm(LIC_DFA);
      } else {
	timer::init(0, "DFA pass", "");
	projnode->updateDFA();
	timer::showtime(0, "DFA pass", "");
	_li(LIC_DFA);
      }

      l_num = 0; // skip the next for block
    }

    if (update_xref == 7) {
      msg("===== DFA init pass =====") << eom;
      timer::init(0, "DFA init pass", "");
      initDFA = true;
      load_all_files(projnode, projnode, save_files, save_includes);
      initDFA = false;
      timer::showtime(0, "DFA init pass", "");
      l_num = 0; // skip the next for block
    }      

    if (update_xref == 8) {
      msg("===== pset dump pass =====") << eom;
      dump_psets = true;
      load_all_files(projnode, projnode, save_files, save_includes);
      dump_psets = false;
      l_num = 0; // skip the next for block
    }      

    if (update_xref == 1 && report_file)
      output_dependency_modules(report_file);
  }

  int i;
  for (i = 0; i < l_num; i++) {
    // map filename to logical name
    genString full_path;
    paraset_get_fullpath(l_files[i], full_path);
    projnode->fn_to_ln(l_files[i], path);

    if (!path.length()) {
      genString new_full_path;
      project_convert_filename((char *) full_path.str(), new_full_path);

      if (new_full_path.length())
	projnode->fn_to_ln (new_full_path, path);
    }

    if (!path.length()) {
      msg("Error: $1 is not defined in your project.") << l_files[i] << eom;
      continue;
    }

    // refresh project to the necessary point
    if (Xref_file_is_newer(path, full_path) == (xrefSymbol *) 0) {
      projModule *module = projnode->find_module (path);

      IF (!module)
	continue;

      load_one_file(module, full_path, types[i], save_files, save_includes, 0);
    } else
      msg("$1: is already updated in project $2") << l_files[i] << eoarg << projname << eom;
  }

  // remove unwanted entries.
  if (r_num > 0) {
    for (i = 0; i < r_num; i++) {
      genString full_path;
      paraset_get_fullpath(r_files[i], full_path);
      projnode->fn_to_ln(full_path, path);

      if (!path.length()) {
	genString new_full_path;
	project_convert_filename(full_path, new_full_path);

	if (new_full_path.length())
	  projnode->fn_to_ln(new_full_path, path);
      }

      if (!path.length()) {
	msg("Error: $1 does not exist.") << r_files[i] << eom;
	continue;
      }

      // refresh project to the necessary point
      //    projModule *module = projnode->find_module (path);

      //if (!module) continue;
      msg(" The file $1 is being removed from model...") << path.str() << eom;
     
      // get pset_file
      genString phys_name;
      projnode->ln_to_fn(path, phys_name);
      genString pset_file;

      if (phys_name.str())
	pset_file = get_paraset_file_name(phys_name, projnode);

      if (pset_file.str())
	OSapi_unlink(pset_file);

      xref->remove_module(path);
    }
     

    xref->save_lxref_by_whole();
  } else {
    // save lxref if files are loaded for xref purposes only
    if (!create_smt)
      xref->save_lxref_by_whole();
  }
}



int
BITTST(char *ba,
       int bn)
{
  return ((ba)[(bn) >> 3] &  (1 << ((bn) & 0x07))) ;
}

void
BITSET(char *ba,
       int bn)
{
  ((ba)[(bn) >> 3] |= (1 << ((bn) & 0x07)));
}

int
user_ddKind(ddKind k)
{
    return (k == DD_SUBSYSTEM /*|| k == DD_ASSOCLINK*/ || k == DD_MODULE ||
	  k == DD_PROJECT || k == DD_IFL_SRC || k == DD_IFL_TRG ||
            /*k == DD_STYLE ||*/ k == DD_CATEGORY);
}


char *strsav(char const *);

void
smt_fill_src_line_num(smtTree *root)
{
//  Initialize(smt_fill_src_line_num);

  smtTree *first;

  if (root == 0)
    return;

  for (smtTree *st = first =  root->get_first();
       st; st = st->get_next()) {
    if (st->get_node_type() != SMT_token)
      smt_fill_src_line_num(st);

    if (first && root->src_line_num == 0)
      root->src_line_num = first->src_line_num;
  }
}
