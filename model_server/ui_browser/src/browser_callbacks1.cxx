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
#define  SUBPROJ_HOME_ALLOWED
//  Comment out this #define to revert to old home project behavior, of
//    always using the root project for home, regardless of what level
//    was selected.
//  See also xref.h.C and projModule.C and ui/globals/homeProjSelector.C

// browser_callbacks.C
//------------------------------------------
// synopsis:
//
// Pull-down menu callback functions
//
//------------------------------------------

// INCLUDE FILES
#include <cLibraryFunctions.h>
#include <msg.h>

#ifdef hp10
#include <errno.h>
#endif

#ifndef _genError_h
#include <genError.h>
#endif

#ifndef _genString_h
#include <genString.h>
#endif

#include <vpopen.h>

#include <messages.h>
#define _xref_h
#define _dd_forward_h
#include <xrefSymbol.h>

#include <gtShell.h>
#include <gtPushButton.h>

#include <Question.h>
#include <openFromBrowser.h>

#ifndef _objOper_h
#include <objOper.h>
#endif

#include <OperPoint.h>
#include <viewSelection.h>
#include <steSlot.h>
#include <dialog.h>

#include <steStyleHeader.h>
#include <stePoint.h>
#include <ste_interface.h>

#ifndef _proj_h
#include <proj.h>
#endif

#include <gtList.h>
#include <projectBrowser.h>
#include <gtMenuBar.h>
#include <Save_Apps.h>

#ifndef _top_widgets_h
#include <top_widgets.h>
#endif

#ifndef _groupHdr_h
#include <groupHdr.h>
#endif

#include <gtStringEd.h>

#ifndef _subsys_h
#include <subsys.h>
#endif

#ifndef _browserShell_h
#include <browserShell.h>
#endif

#ifndef _gtForm_h
#include <gtForm.h>
#endif

#ifndef _gtTextEd_h
#include <gtTextEd.h>
#endif

#include <merge_dialog.h>
#include <save_app_dialog.h>
#include <putUI.h>

projModulePtr  app_get_mod(appPtr);
typedef int (projModule::*int_method_const_char) (const char*);
typedef int (projModule::*int_method) ();

int configurator_to_icon();

// PRE-PROCESSOR DEFINITIONS

#undef PUSHBUTTONCB
#define PUSHBUTTONCB(name)\
    void browserShell::name(gtPushButton*, gtEvent*, void* bs_data, gtReason)

// FUNCTION DEFINITIONS
inline const projNode* current_project(const char *operation)
{
    const projNode *proj = projNode::get_home_proj();
    if(!proj)
	msg("ERROR: Home Project must be set before performing $1.") << gettext(operation) << eom;
    return proj;
}

inline int valid_selection(const symbolArr &array)
{
    int ret_val = (array.size() > 0);
    if(!ret_val) {
#ifndef NEW_UI
	gtPushButton::next_help_context("Browser.Manage.EmptyError.Help");
#endif
	msg("ERROR: No modules are selected.") << eom;
    }

    return ret_val;
}

bool browser_delete_module (gtBase* parent, projModule *module, bool yes_to_all, const char *mesg)
{
    Initialize (delete_module);

    if (!module) return false;

    if (!(module->is_home_project())) {
	msg("ERROR: To delete, the Home Project must be set to this project") << eom;
	return false;
    }
    
    int ans;
    if (yes_to_all)
        ans = 2;
    else {
        genString txt;
        txt.printf(TXT("%s\nOK to delete module?\n\"%s\""), mesg, module->get_name());
        ans = dis_question3("Browser.File.Delete.Help", TXT("Delete"),
			    TXT("Yes"), TXT("Yes to All"), TXT("Cancel"), txt.str());
    }

    if (ans == 1 || ans == 2)
	module->delete_module();

    if (ans == 2)
        return true;

    return false;
}

int delete_path(gtBase* parent, const char* file_name)
{
    // Return 1 only if deleted succesfully

    Initialize(delete_path);

    struct stat info;
    if(OSapi_stat(file_name, &info) == 0)
    {
	// Ask if it is OK to delete path.
	genString txt;
	txt.printf(
	    gettext((S_IFDIR & info.st_mode) ?
		    TXT("OK to delete entire directory?\n\"%s\"") :
		    TXT("OK to delete file?\n\"%s\"")),
	    file_name);

        int ans = dis_question3("", TXT("Delete"), TXT("Yes"), NULL,
                                TXT("Cancel"), txt.str());
	if(ans > 0)
	{
	    projModule *mod;
	    if (!(S_IFDIR & info.st_mode))     // not a directory
	    {
                genString log_name;
                projHeader::fn_to_ln(file_name, log_name);

                if (log_name.length()) // if not a project file, do nothing
                {
                    projNode* proj_node = projNode::get_home_proj();
                    if (!proj_node) 
                        proj_node = projHeader::non_current_project(log_name);

                    if (!proj_node) 
                        return -1;

                    mod = proj_node->find_module(log_name);
                    if (mod) {
                        // confirm deletion if module loaded

                        appPtr app_head = mod->get_app();
                        if (app_head) {
                            if ( app_head->is_modified() )
				if (dis_question("", B_YES, B_NO,
                                    "Module modified; really want to delete?") <= 0)
				    return 0;
			    
			    mod->unload_module();                          
                        }
			mod->delete_module();
                    }
                }
	    }

	    genString command;
	    command.printf("rm -rf %s", file_name);
	    v_system(command);

	    // Check if deletion failed:
	    // stat() succeeds  OR  failure was not due to missing file
	    if(!OSapi_stat(file_name, &info)  ||  errno != ENOENT)
	    {
		msg("ERROR: Unable to delete file:\n$1") << file_name << eom;
		return -1;
	    }
            return 1;
	}

       return 0 ; /* user does not want to delete after all */ 
    }
    else {   /* reason as to why no delete was initiated */
    switch(errno)
    {
      case EACCES:
	msg("ERROR: No search permission for:\n$1") << file_name << eom;
	break;

      case ENOENT:
        msg("WARN: File $1 does not exist.") << file_name << eom;
	break;

      default:
	msg("ERROR: Error #$1 occured while deleting:\n$2") << errno << eoarg << file_name << eom;
	break;
    }
    return -1;
  }
}

static void operate_on_modules(symbolArr& array, int_method func)
//
// Invoke a projModule member-function on the projModules in array.
// func returns int.
//
{
    Initialize(operate_on_modules);

    if(!valid_selection(array))		// if nothing is selected, let user know
	return;

    symbolPtr sym;
    ForEachS(sym, array)
    {
        projModulePtr module = NULL;

        if(sym.is_xrefSymbol())
	    module = projModule_of_symbol(sym);
	else if(is_projModule(sym))
	    module = projModulePtr(RelationalPtr(sym));

        if(module)
	    (module->*func)();
    }
}

PUSHBUTTONCB(project_lock)
{
    Initialize(browserShell::project_lock);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop
 
    browserShell* bs = (browserShell*)bs_data;
    ste_finalize ((viewPtr)NULL);

    if (bs->right_project_browser(0))
    {
        symbolArr& modules = bs->project_browser->symbol_selection();

	operate_on_modules(modules, &projModule::lock_module);
	configurator_to_icon();
    }
    bs->project_browser->deselect();
#endif
}

static void operate_on_modules(
    symbolArr& array, int_method_const_char func, const char* message)
//
// Invoke a projModule member-function on the projModules in array.
// func takes a string and returns int.
//
{
    Initialize (operate_on_modules);

    if(!valid_selection(array))		// if nothing is selected, let user knwo
	return;

    symbolPtr sym;
    ForEachS(sym, array)
    {
        projModulePtr module = NULL;

        if(sym.is_xrefSymbol())
	    module = projModule_of_symbol(sym);
	else if(is_projModule(sym))
	    module = projModulePtr(RelationalPtr(sym));

        if(module)
	    (module->*func)(message);
    }
}

void unlock_module_arr(symbolArr &modules)
{
    Initialize(unlock_module_arr);

    int msg_for_each = 1;
    genString msg = "";
    if (modules.size() > 1)
    {
        while(1)
        {
            msg_for_each = dis_question3("Browser.Manage.Unlock.EnterMessages",
                                         TXT("MESSAGE FOR EACH UNLOCK MODULE"),
					 TXT("Yes"), TXT("No"), TXT("Cancel"),
                                         TXT("Do you want to enter a separate message for each module?"));
            if (msg_for_each != 1 && msg_for_each != 2)
                break;
            if (msg_for_each == 2)
            {
                int cancel_button = projModule::obtain_unlock_message(msg, 0);
                if (cancel_button == 1)
                {
                    operate_on_modules(modules, &projModule::unlock_module_c,msg.str());
                    break;
                }
            }
            else
            {
                symbolPtr sym;

                ForEachS(sym, modules)
                {
                    projModulePtr mod = NULL;

                    if (sym.is_xrefSymbol())
                        mod = projModule_of_symbol(sym);
                    else if (is_projModule(sym))
                        mod = projModulePtr(RelationalPtr(sym));

                    if (mod)
                    {
                        int cancel_button = projModule::obtain_unlock_message(msg, 0, "Skip");
                        if (cancel_button == 1) // OK
                            mod->unlock_module_c(msg.str());
                        else if (cancel_button == -1) // cancel
                            break;
                        // else if (cancel_button == 2) continue; // skip
                    }
                }

                break;
            }
        }
    }
    else
        operate_on_modules(modules, &projModule::unlock_module);
}

PUSHBUTTONCB(project_unlock)
{
    Initialize(browserShell::project_unlock);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop

    browserShell* bs = (browserShell*)bs_data;
    ste_finalize ((viewPtr)NULL);

    if (bs->right_project_browser(0))
    {
        symbolArr& modules = bs->project_browser->symbol_selection();
        unlock_module_arr(modules);
        configurator_to_icon();
    }
    bs->project_browser->deselect();
#endif
}

static void really_unload(void* data)
{
    Initialize(really_unload);

    symbolArr& array = *(symbolArr*)data;
    if (array.size())
	operate_on_modules(array, &projModule::unload_module);
}

extern "C" void global_really_unload(void* data)
{
    Initialize(really_unload);

    really_unload(data);
}

static symbolArr* modules_to_unload;

void unload_selection(symbolArr &symbols)
{
    Initialize(unload_selection);
    
    objArr app_headers;
    delete modules_to_unload;
    modules_to_unload = NULL;
    symbolPtr el;
    ForEachS(el, symbols)
    {
	// For each element of symbols, attempt to find a projModule and
	// assign it to mod (below).
	
	projModulePtr mod = NULL;
	
	if(el.is_xrefSymbol())
	{
	    if(el.get_kind() == DD_MODULE  &&  el.is_loaded())
		    mod = projModule_of_symbol(el);
	}
	else if(is_projModule(RelationalPtr(el)))
	{
	    mod = projModulePtr(RelationalPtr(el));
	}
	if (!mod)
	    mod = projModule_of_symbol(el);
	
	if(mod)
	{
	    appPtr ah = mod->find_app();
	    if(modules_to_unload == NULL)
		modules_to_unload = new symbolArr;
		
	    modules_to_unload->insert_last(mod);
		
	    if(ah)
		app_headers.insert_last(ah);
	}
    }
    if(modules_to_unload  &&  modules_to_unload->size())
    {
	// offer_to_save_list copies its third argument (app_headers).
	
	offer_to_save_list(
	    NULL, Flavor_Unloading, &app_headers, really_unload,
	    modules_to_unload);
    }
    else
    {
#ifndef NEW_UI
        gtPushButton::next_help_context("Pset.Help.Errors.Unload");
#endif
	msg("ERROR: No appropriate files are selected for unloading.") << eom;
    }
}

static int under_home(projNode *proj)
{
    Initialize(under_home);

    const projNode *h = current_project("Check in modules in a project");
    projNode *cur = proj;
    while(proj)
    {
        if (proj == h)
            return 1;

        if (proj->get_map())
            break;
        proj = proj->find_parent();
    }
#ifndef NEW_UI
    gtPushButton::next_help_context("Browser.Manage.Put.MustBeHome");
#endif
    msg("ERROR: Selected project $1 does not belong to home project $2,\nand its modules can't be checked in.") << cur->get_ln() << eoarg << h->get_ln() << eom;
    return 0;
}

static void really_put(void* data)
{
    Initialize(really_unload);

    symbolArr& array = *(symbolArr*)data;
    putUI* dialog = putUI::load_putUI(array);
}

static void add_proj_sel(symbolArr &symbols, symbolArr& projects)
{
    Initialize(add_proj_sel);

    symbolPtr el;
    ForEachS(el, projects)
    {
        projNodePtr proj = sym_to_proj(el);

        if (!under_home(proj))
            continue;
        proj->search_all_modules(symbols);
    }   
}

PUSHBUTTONCB(project_put)
{
    Initialize(browserShell::project_put);
#ifndef NEW_UI
    browserShell* bs = (browserShell*)bs_data;
    ste_finalize ((viewPtr)NULL);

    // Summary:
    // Build a symbolArr of selected projModules.
    // Also build an arrays with loaded and modified projModules and apps.
    // Pass these two arrays to offer_to_save_list, which will offer to
    // save the appropriate app-headers before passing the projModules to
    // operate_on_modules

    int rejections = 0;  // Some selection rejected

    if (bs->right_project_browser(0))
    {
        objArr app_headers;

	delete modules_to_unload;
	modules_to_unload = NULL;

	symbolArr& symbols = bs->project_browser->symbol_selection();
        add_proj_sel(symbols, bs->project_browser->project_selection());

	symbolPtr el;
	ForEachS(el, symbols)
	{
	    // For each element of symbols, attempt to find a projModule and
	    // assign it to mod (below).

	    projModulePtr mod = NULL;

	    if(el.is_xrefSymbol())
	    {
		if(el.get_kind() == DD_MODULE)
		    if(el.is_loaded())
			mod = projModule_of_symbol(el);
		    else
			mod = projHeader::find_module(el.get_name());
	    }
	    else if(is_projModule(RelationalPtr(el)))
	    {
		mod = projModulePtr(RelationalPtr(el));
	    }

 	    appPtr appptr;
	    if(mod) {

		if (!mod->get_project()->get_xref()->is_writable()) {
		    rejections++;
		    continue;
		}

	        if(modules_to_unload == NULL)
	            modules_to_unload = new symbolArr;

   	        if (appptr = mod->get_app())
	            app_headers.insert_last(appptr);

		modules_to_unload->insert_last(mod);
	    }
	}

        if (modules_to_unload && modules_to_unload->size()) {
    	      // offer_to_save_list copies its third argument (app_headers).

              offer_to_save_list(
		bs->shell, Flavor_Putting, &app_headers, really_put,
                modules_to_unload);
	  }

          if (rejections)
	      msg("ERROR: Some files were selected which were already in system projects") << eom;
	  else if (!modules_to_unload)
	      msg("WARN: Nothing is selected.") << eom;
    }
    bs->project_browser->deselect();
#endif
}

#if 0
/*
   $Log: browser_callbacks1.C  $
   Revision 1.32 2001/11/07 14:07:46EST ktrans 
   Remove dormant project code, -verify_pdf option, and slow put w/ feedback.  See no_slow_put branch for details.
*/
#endif
