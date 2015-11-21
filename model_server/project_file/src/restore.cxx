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

#include <machdep.h>
#include <cLibraryFunctions.h>
#include <msg.h>

#include <fileCache.h>

#include <app.h>
#include <db_intern.h>
#include <RTL_externs.h>
#include <merge_dialog.h>

#include <customize.h>
#include <systemMessages.h>

#include <objOper.h>
#include <objRawApp.h>
#include <proj.h>
#include <feedback.h>
#include <ddKind.h>
#include <path.h>
#include <pdf_tree.h>

#ifndef _WIN32
#ifndef _targetProject_h
#include <targetProject.h>
#endif
#endif

#include <xref.h>
#include <db.h>
#include <save.h>

#include <groupHdr.h>
#include <driver_mode.h>
#include <shell_calls.h>

void proj_path_report_app(app *, projNode *prj = NULL);
int outdated_pset(const char * src_file, const char *pset_file,
        const char *fn);

bool isDFAinit();
void check_auto_save_copy(char *fn);

void proj_set_app_insert_Xref (app* h, Xref* xr);

int proj_same_root(projNode*p1, projNode*p2)
{
  projNode *r1 = p1->root_project();
  projNode *r2 = p2->root_project();

  return r1==r2;
}

int proj_ln_is_visible(projNode * proj, const char * ln, projNode**pmodp=NULL)
{  
    projNode *vis_proj = projHeader::get_pmod_project_from_ln(ln);
    if(pmodp) *pmodp = vis_proj;
    if(!vis_proj)  // this file is not in any pmod (yet)
	return 1;
    if(proj_same_root(proj, vis_proj)){
       return 1;
    } else {
       return 0;
    }
}

static bool proj_under_home (projNode* proj)
{
  projNode* home = projNode::get_home_proj();
  
  for(projNode* cur = proj; cur; cur = cur->find_parent())
    if (cur == home)
      return TRUE;

  return FALSE;
}

app *proj_restore_file(projNode*that, const char *fn, const char *ln, 
      projModule * mod, int(*process_events)(void*), void* data)
// try to restore the designated file, returning 0 if you cannot
//
// fn: is the file name
//
// ln: is the logical name
//
// mod: is the projmodule for this file
{
    Initialize(proj_restore_file);

    check_auto_save_copy((char *)fn);

    projNode *model_prj;
    if(!proj_ln_is_visible(that, ln, &model_prj)){
	if(model_prj){
	   msg("WARN: model for $1 is hidden by $2") << fn << eoarg << model_prj->get_name() << eom;
	}
	return NULL;
    }

    if(process_events && process_events(data)) return NULL;

    genString fn_gs = fn;
    genString pset_file = get_paraset_file_name(fn, that);

    if (!has_paraset_file(pset_file))
        return NULL;

    int outdated = 1;
    if (mod)
	outdated = mod->outdated_pset();
    else
	outdated = ::outdated_pset(fn, pset_file, fn);

    int db_format = db_get_db_format(fn);
    
    if (!merge_file_type(fn) && outdated) {
	return NULL;
    }

    int is_readonly_merged = 0;
    
    if (db_format) {
	if (outdated) {
          if(process_events && process_events(data))return NULL;
	  if (!mod)
		mod = that->find_module(ln);
	  if (!mod )
	    return NULL;
	   
	  int backup_needed = 1;
	  int file_type = 0; // local
	  int unload_flag = 0;
	  int do_not_save = 0;
	  if( !mod->check_write_priv(0)){
	    file_type = -1;   // system
	    backup_needed = 0;
	    do_not_save = 1;
            is_readonly_merged = 1;
	  }
	  
          int merstat =  projModule::merge_module(mod, fn, 
		     pset_file, file_type, backup_needed, unload_flag, do_not_save);
	  if(merstat < 0)
	    return NULL;

	  if(process_events && process_events(data))return NULL;

	}
    } else {  // this code is needed when some pset file does not have the source
	if (outdated)
	    return NULL;	// use the raw view
    }


    app *app_head = 0;
    if (mod)
        app_head = mod->get_app(); // merge_module might already do the restore
    if (!app_head){
        if(process_events && process_events(data))return NULL;
        app_head = ::db_restore (fn, ln);
	if (app_head)
	    proj_path_report_app(app_head, that);
    }
    if (app_head){
// mg: changed for sys file, to avoid root level instead of pmod_level
        if(is_readonly_merged){
          projNode*proj = appHeader_get_projectNode(app_head);
	  if(!proj)
	    projectNode_put_appHeader (that, app_head);
        } else {
	  projectNode_put_appHeader (that, app_head);
	}
	if(!mod)
	  mod= appHeader_get_projectModule(app_head);
//	    mod = app_head->get_module();
        if (mod){
	  mod->update_module ();
	  projectModule_put_appHeader (mod, app_head);
	  (checked_cast(commonTree,mod))->language = app_head->language;
	}

	if (proj_under_home(that)) {
	  symbolPtr newer =  Xref_file_is_newer((char *) ln, (char *) fn);

	  if (newer.xrisnull()) {
	    if (isDFAinit()) {
	      msg("The DFA initialization pass is not supposed to modify non-DFA model files\nLogical filename: $1\nPhysical filename: $2\n") << ln << eoarg << fn << eom;
	      cout.flush();
	    } else {
	      Xref *xr = that->get_xref();

	      if (xr && xr->get_perm_flag() != READ_ONLY) {
		proj_set_app_insert_Xref(app_head, xr);
		xr->insert_module_in_lxref(app_head);
	      }
	    }
	  }
	}

	proj_path_report_app (app_head, that);
    }
    return app_head;
}

app *projNode::restore_file(const char *fn, const char *ln, projModule * mod)
{
    return
       proj_restore_file(this, fn, ln, mod, NULL, NULL);
}

app *proj_restore_file_wo_module(projNode*proj, const char *fn, const char *ln);

app *projNode::restore_file_wo_module(const char *ln)
  /* will try to restore file from this project if possible */
{
  Initialize(projNode::restore_file_wo_module);

  projNode * pmod_proj = get_pmod_project_from_ln(ln);
  if(!proj_same_root(pmod_proj, this)){
      msg("WARN: model for $1 is hidden by $2") << ln << eoarg << pmod_proj->get_name() << eom;
      return NULL;
  }



  genString fn;
  ln_to_fn(ln, fn);
	  
  appPtr ah = (pmod_proj && fn.str())?
    proj_restore_file_wo_module(pmod_proj, fn.str(), ln):0;

  return ah; // flag error messages here ?
}


app *proj_restore_file_wo_module(projNode*proj, const char *fn, const char *ln)
{

    // need to get app before checking for outdated_pset, is there a cheaper way than static
    appPtr app_head = projModule::get_app(ln, fn);
    if (app_head)
      return app_head;
    
    check_auto_save_copy((char *)fn);
    genString fn_gs = fn;
    genString pset_file = get_paraset_file_name((char*)fn, proj);

    if (!has_paraset_file(pset_file))
        return NULL;

    int outdated = ::outdated_pset(fn, (const char *)pset_file, fn);

    int db_format = db_get_db_format((char *)fn);
    
    if (!merge_file_type((char *)fn) && outdated) {
	return NULL;
    }

    if (outdated)
        return NULL;

    if (!app_head)
        app_head = ::db_restore ((char*)fn, (char*)ln);
    if (app_head)
    {
        projectNode_put_appHeader (proj, app_head);
	if (proj_under_home (proj))  {
	  symbolPtr newer =  Xref_file_is_newer((char*)ln, (char*)fn);
	  if (newer.xrisnull()) {
	    Xref* xr = proj->get_xref();
	    if (xr && xr->get_perm_flag()!=READ_ONLY)
	      xr->insert_module_in_lxref(app_head);
	  }
	}
        proj_path_report_app (app_head, proj);
    }
    return app_head;
}

appPtr db_restore_update_xref(projModule *pm)
{
  Initialize(projModule::db_restore_update_xref);
  if (pm == 0) return 0;
  genString pn;
  pm->get_phys_filename(pn);
  if (pn.str() == 0) return 0;
  appPtr app_h = ::db_restore(pn, pm->get_name());

  if (app_h) {
    projNode * pr = pm->get_project();
    if (pr) 
      proj_path_report_app (app_h, pr);
    if (proj_under_home (pr)) {
      symbolPtr newer =  Xref_file_is_newer((char*)pm->get_name(), (char*)pn);
      if (newer.xrisnull())    {
	Xref* xr = (pr) ? pr->get_xref():0;
	if (xr && xr->get_perm_flag()!=READ_ONLY)
	  xr->insert_module_in_lxref(app_h);
      }
    }
  }
  return app_h;
}

appPtr projModule::restore_module()
// Restore the module into memory
{
    appPtr app_head = get_app();
    if (app_head && is_smtHeader(app_head))
    {
        smtHeaderPtr smt = smtHeaderPtr(app_head);
        if (smt->foreign && smt->parsed == 0)
        {
            obj_unload (smt);
            app_head = NULL;
        }
    }

    if (app_head) return app_head;

    projNode* proj_node = get_project_internal();

    genString filename;
    get_phys_filename(filename);
    app_head =  proj_node->restore_file(filename, (char *)ln_name, this);

    return app_head;
}

appPtr projModule::restore_module(int update_xref)
// restore module
//
// if update_xref is set and the pset is newer, update pmod
{
    Initialize(projModule::restore_module(int));
    appPtr app_head = get_app();
    if (app_head)
    {
        if (is_smtHeader(app_head))
        {
            smtHeaderPtr smt = smtHeaderPtr(app_head);
            if (smt->foreign && smt->parsed == 0)
            {
                obj_unload (smt);
                app_head = NULL;
            }
        }
    }
    if (app_head)
        return app_head;
    genString filename;
    projNode* proj_node = get_project_internal();
 
    if(! proj_ln_is_visible(proj_node, (const char *)ln_name))
       return NULL;

    proj_node->ln_to_fn ((char *)ln_name, filename);
    app_head = ::db_restore ((char*)filename, (char*)ln_name);
    if (app_head)
    {
        update_module ();
        projectNode_put_appHeader (proj_node, app_head);
	proj_path_report_app (app_head, proj_node);
	if (proj_under_home (proj_node)) {
	  symbolPtr newer =  Xref_file_is_newer((char*)ln_name, (char*)filename);
	  if (update_xref && newer.xrisnull()) {
	    Xref* xr = proj_node->get_xref();
	    if (xr && xr->is_writable()){ 
	      xr->remove_module(app_head->get_filename()); // mg + wmm
	      xr->insert_module_in_lxref(app_head);
	    }
	  }
	}
    }

    return app_head;
}
