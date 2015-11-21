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
// save_apps.C
//------------------------------------------
// synopsis:
// Create "Save Apps" dialog and fill in the list of modified apps.
// 
//------------------------------------------

// INCLUDE FILES

#include "machdep.h"
#include <msg.h>
#include <objArr.h>
#include <objOper.h>
#include <objRawApp.h>
#include <db.h>
#include <ddict.h>
#include <transaction.h>
#include <messages.h>
#include "fileEntry.h"

#include <genError.h>
#include <Save_Apps.h>
#include <proj.h>
#include <save_app_dialog.h>
#include <top_widgets.h>
#include <XrefTable.h>
#ifndef _gtAppShellXm_h
#include <gtAppShellXm.h>
#endif

#ifndef _groupHdr_h
#include <groupHdr.h>
#endif

// FUNCTION DEFINITIONS

static int select_callback(
    SaveDialog* dlg, void* modified, int* selected, int count)
{
    Initialize(select_callback);

    objArr& array = *(objArr*)modified;

    dlg->clear_to_be_saved();
    for(int i = 0; i < count; ++i)
    {
	appPtr app_header = checked_cast(app,array[selected[i] - 1]);
	//
	// append other dependencies here!
	//
	if(app_header->get_phys_name() != NULL)
	    dlg->add_to_be_saved(app_header->get_phys_name());
	else
	    dlg->add_to_be_saved(app_header->get_filename());
    }

    return(1);
}

int in_selected(int *selected, int count, int ind)
{
  for (int i = 0; i < count; i++) {
    if (selected[i] == ind) return 1;
  }
  return 0;
}

static projModule *app_get_projModule(app* ah)
{
  Initialize(app_get_projModule);
  if (ah == 0) return 0;
  projModule* module = appHeader_get_projectModule (ah);
  if (module) return module;
  projNode* pn = app_get_proj(ah);
  if (pn)
    module = pn->find_module(ah->get_filename());
  return module;
}

void update_mem_pmod_with_pset_on_disk(app *ah, int unload = 0)
{
  Initialize(update_pmod_with_pset_on_disk);
  projModule * module = app_get_projModule(ah);
  if (module) {
    module->unload_module();
    module->invalidate_cache();
    if (module->is_good_paraset_file()) {
      projNode *pr = module->get_project();
      if (pr) {
	Xref *xr = pr->get_xref();
	if (xr) {
	  symbolPtr fsym = xr->lookup_module(module->get_name());
	  if (fsym.isnotnull() && fsym.get_has_def_file())
	    fsym->set_last_mod_date((time_t) 1, xr->get_lxref());
	}
      }
      ah = module->restore_module(1);
      if (ah && unload) {
	module->unload_module();
	module->invalidate_cache();
      }
    }
    else {
      projNode *pr = module->get_project();
      if (pr) {
	Xref *xr = pr->get_xref();
	if (xr)
	  xr->remove_module(module->get_name());
      }
    }
    // add files to recovery list
  }
}
int SaveDialog::prepare_for_update_perm_pmod(int *selected, int count,
					     int restore_not_sel)
{
  Initialize(SaveDialog::prepare_for_update_perm_pmod);
  if (our_flavor == Flavor_Quitting) {
    objArr& array = *(objArr*)modified;
    int array_size = array.size();
    start_transaction() {
      for (int i = 0; i < array_size; ++i) {
	if ((!in_selected(selected, count, i+1) && restore_not_sel) ||
	    (in_selected(selected, count, i+1) && !restore_not_sel)) {
	  appPtr ah = checked_cast(app, array[i]);
	  if (is_objRawApp(ah)) continue;
	  update_mem_pmod_with_pset_on_disk(ah);
	} 
      }
    }end_transaction();
  }
  return 1;
}

static int
save_callback(SaveDialog *dlg,
	      void *modified,
	      int *selected,
	      int count)
{
  Initialize(save_callback(SaveDialog *, void *, int *, int));

  int errors = 0;
  objArr &array = *(objArr *) modified;

  int i;
  for (i = 0; i < count; ++i) {
    appPtr app_head = checked_cast(app, array[selected[i] - 1]);

    if (db_save(app_head, NULL)) {
      projModule *module = appHeader_get_projectModule(app_head);

      if (module)
	module->invalidate_cache();

      app_head->clear_modified();
      app_head->clear_imported();
    } else {
      msg("ERROR: Error while saving file $1") << app_head->get_filename() << eom;
      errors = 1;
    }
  }

  dlg->prepare_for_update_perm_pmod(selected, count, 1);

  if (!errors) {
    for (i = count - 1; i >= 0; --i)
      array.remove(selected[i] - 1);

    dlg->clear_to_be_saved();
  }
    
  return(!errors);
}

static int notsave_callback(
    SaveDialog* dlg, void* modified, int* selected, int count)
{
    Initialize(notsave_callback);

    int errors = 0;

    objArr& array = *(objArr*)modified;
    
    if (dlg->get_flavor() == Flavor_Quitting) {
      int array_size = array.size();
      for (int i = 0; i < array_size; i++) {
	appPtr app_head = checked_cast(app, array[i]);
	if (!in_selected(selected, count, i+1)) {
	  if(db_save(app_head, NULL))
	  {
	    projModule* module = appHeader_get_projectModule (app_head);
	    if (module) module->invalidate_cache();
	    app_head->clear_modified();
	    app_head->clear_imported();
	  }
	  else
	  {
	    msg("ERROR: Error while saving file $1") << app_head->get_filename() << eom;
	  }
	}
      }
    }
    else {
      for(int i = 0; i < count ; i++)
      {
	appPtr app_head = checked_cast(app,array[selected[i] - 1]);
        if (is_objRawApp (app_head))
	  continue;
	update_mem_pmod_with_pset_on_disk(app_head);	
      }
    }
    dlg->prepare_for_update_perm_pmod(selected, count, 0);
    for(int i = count - 1; i >= 0; --i)
      array.remove(selected[i] - 1);
    dlg->clear_to_be_saved();
    return(!errors);
}

int proj_hide_save_callback(SaveDialog* dlg, void* modified, int* selected, int count)
{
  Initialize(proj_hide_save_callback);

  int errors = 0;

  objArr& array = *(objArr*)modified;
  start_transaction() {
  int array_size = array.size();
  int i;
  for (i = 0; i < array_size; i++) {
    appPtr app_head = checked_cast(app, array[i]);
    if (in_selected(selected, count, i+1)) {
      projModule* module = app_get_projModule (app_head);
      if(app_head->needs_to_be_saved() && db_save(app_head, NULL))
      {
	if (module) module->invalidate_cache();
	app_head->clear_modified();
	app_head->clear_imported();
      }
      else
      {
	msg("ERROR: Error while saving file $1") << app_head->get_filename() << eom;
	errors = 1;
      }
      if (module && !is_objRawApp (app_head)) {
	module->unload_module();
	module->invalidate_cache();
      }
    }
    else if (!is_objRawApp (app_head) && app_head->needs_to_be_saved())
      update_mem_pmod_with_pset_on_disk(app_head, 1);
  }
  if (!errors)
  {
    for(i = count - 1; i >= 0; --i)
      array.remove(selected[i] - 1);
    dlg->clear_to_be_saved();
  }
  }end_transaction();
  return(!errors);
}

int proj_hide_notsave_callback (SaveDialog *dlg, void *modified, int *selected, int count)
{
  Initialize(proj_hide_notsave_callback);

  int errors = 0;

  objArr& array = *(objArr*)modified;
   start_transaction() { 
  int array_size = array.size();
  int i;
  for (i = 0; i < array_size; i++) {
    appPtr app_head = checked_cast(app, array[i]);
    if (!in_selected(selected, count, i+1)) {
      projModule* module = app_get_projModule (app_head);
      if(app_head->needs_to_be_saved() && db_save(app_head, NULL))
      {
	if (module) module->invalidate_cache();
	app_head->clear_modified();
	app_head->clear_imported();
      }
      else
      {
	msg("ERROR: Error while saving file $1") << app_head->get_filename() << eom;
      }
      if (module && !is_objRawApp (app_head)) {
	module->unload_module();
	module->invalidate_cache();
      }
    }
    else if (!is_objRawApp (app_head) && app_head->needs_to_be_saved())
      update_mem_pmod_with_pset_on_disk(app_head,1);
  }
  for(i = count - 1; i >= 0; --i)
    array.remove(selected[i] - 1);
  dlg->clear_to_be_saved();
  }end_transaction();
 
  return(!errors);
}

static int cleanup_callback(SaveDialog*, void* modified, int*, int)
{
    Initialize(cleanup_callback);

    delete (objArr*)modified;

    return(1);
}

int offer_to_save_wait(gtBase* parent, dlgFlavor flavor, objArr *list,
			      void (*OK_callback)(void*), void* OK_data, int wait)
// This is the original offer_to_save routine
// It adds a new feature. The calling routine could ask this one to wait
// until some button is pressed
{
    Initialize(offer_to_save_wait);

    int ret_val = 0;
    if(!list->empty())
    {
	SaveDialog* dlg = new SaveDialog(parent, list, flavor,
	    select_callback, save_callback, cleanup_callback,
	    OK_callback, OK_data, notsave_callback);
	
        {
	    ObjPtr el;
	    ForEach(el, *list){
		appPtr app_head = checked_cast(app, el);
		if(app_head->get_phys_name() != NULL)
		    dlg->add_modified(app_head->get_phys_name());
		else
		    dlg->add_modified(app_head->get_filename());
	    }
	}
	ret_val = dlg->popup(wait != 0);
    }
    else if (OK_callback)
	OK_callback(OK_data);
    
    return ret_val;
}

static void offer_to_save(gtBase* parent, dlgFlavor flavor, objArr *list,
			  void (*OK_callback)(void*), void* OK_data)
// This is the old offer_to_save interface routine.
//
// It just to call the new offer_to_save_wait interface routine with wait = 0 which
//  means no wait.
{
    Initialize(offer_to_save);

    offer_to_save_wait(parent, flavor, list, OK_callback, OK_data, 0);
    
    return;
}

void offer_to_save_list(gtBase* parent, dlgFlavor flavor, objArr* list,
			void (*OK_callback)(void*), void* OK_data)
// This is the old offer_to_save_list interface routine.
//
// It just to call the new offer_to_save_list_wait interface routine with wait = 0 which
//  means no wait
{
    Initialize(offer_to_save_list);

    offer_to_save_list_wait(parent, flavor, list, OK_callback, OK_data, 0);

    return;
}

int offer_to_save_list_wait(gtBase* parent, dlgFlavor flavor, objArr* list,
			     void (*OK_callback)(void*), void* OK_data, int wait)
// This is the original offer_to_save_list routine
// It adds a new feature. The calling routine could ask this one to wait
// until some button is pressed
{
    Initialize(offer_to_save_list_wait);

    objArr* modified = new objArr;

    {
	ObjPtr elem;
	ForEach(elem, *list) {
	    appPtr ah = checked_cast(app,elem);
	    if (ah->needs_to_be_saved())
		modified->insert_last(ah);
	}
    }
    
    return offer_to_save_wait(parent, flavor, modified, OK_callback, OK_data, wait);
}

// returns: 1 - OK; -1 canceled
int offer_to_save_apps_wait(gtBase* parent, dlgFlavor flavor,
			void (*OK_callback)(void*), void* OK_data, int wait)
{
    Initialize(offer_to_save_apps_wait);

    objArr* modified = app::get_modified_headers();

    int res = offer_to_save_wait(parent, flavor, modified, OK_callback, OK_data, wait);

    return res;
}

void offer_to_save_apps(gtBase* parent, dlgFlavor flavor,
			void (*OK_callback)(void*), void* OK_data)
{
    Initialize(offer_to_save_apps);
    
    (void) offer_to_save_apps_wait (parent, flavor, OK_callback, OK_data, 0);
}
