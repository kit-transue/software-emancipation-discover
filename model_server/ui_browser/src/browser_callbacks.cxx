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
#include "cLibraryFunctions.h"
#include "msg.h"
#include "xxinterface.h"
#include "genError.h"
#include "genString.h"
#include "messages.h"

#include "_Xref.h"

#define _xref_h
#define _dd_forward_h

#include "driver.h"
#include "xrefSymbol.h"
#include "ddict.h"
#include "attribute.h"

extern int xref_not_header_file(char *full_path);
extern int xref_not_src_file(char *full_path);
extern int xref_not_txt_file(char *full_path);
extern void delete_scratch_apps();

#include <gtTopShell.h>
#include <gtPushButton.h>
#include <Question.h>
#include <openFromBrowser.h>
#include <aboutDialog.h>
#include <setupProps.h>
#include "../include/statsUI.h"
#include <objOper.h>
#include <steStyleHeader.h>
#include <steTextNode.h>
#include <stePoint.h>
#include <ste_interface.h>
#include <proj.h>
#include <fileLanguageDlg.h>
#include <save_app_dialog.h>
#include <fileCache.h>
#include <cmd.h>
#include <oodt_relation.h>
#include <extract-dialog.h>
#include <JournalDlg.h>
#include <top_widgets.h>
#include <getFileSelector.h>
#include <putUI.h>
#include <feedback.h>

#include <externApp.h>

#include <StatusDialog.h>
#include <getUI.h>
#include <disUIDG_extern.h>
#include <partition.h>
#include <OODT_apl_entries.h>
#include <subsys.h>
#include <browserShell.h>
#include <NewPrompt.h>
#include <path.h>
#include <viewerShell.h>
#include <view_creation.h>
#include <messagePopup.h>

void HelpUI_instance_edit_widget_index();
void HelpUI_instance_dump_widget_index();
boolean HelpUI_get_set_show_pathnames();
boolean HelpUI_get_set_use_helper_shell();

#include <gtForm.h>
#include <gtTextEd.h>
#include <driver_mode.h>
#include <scopeMgr.h>

void view_flush_on_top();
typedef int (projModule::*int_method_projNode) (projNode *);
typedef projModule* (projModule::*projModuleptr_method) ();
typedef appPtr (projModule::*appPtr_method) ();
typedef projModule* (*projModuleptr_func) (symbolPtr&);
typedef projModule* (*projModuleptr_with_data_func) (symbolPtr&, void *);

extern "C"
{
    int home_proj_selector();
    void cmd_journal_start(char *, char *, char *, char *, char *);
    void cmd_journal_save(char *);
    void cmd_journal_exec_within(char *);
    void cmd_journal_insert_line(char *);
    void apl_PROJ_Debug_Break();
    void apl_PROJ_Debug_Print_Selected_Node();
    void apl_PROJ_Debug_Follow(int);
    void apl_PROJ_Debug_Print_Selected_Tree();
    void apl_PROJ_start_metering();
    void apl_PROJ_stop_metering(const char*);
    void apl_PROJ_suspend_metering();
    void apl_PROJ_resume_metering();
    void dr_clear_selection();
    void doFindDeadCode(void*);
    void browser_import(Widget, const char*, const char*);
}

extern void reset_xref_notifier();

int offer_to_save_apps_wait(gtBase* parent, dlgFlavor flavor,
			    void (*OK_callback)(void*), void* OK_data, int wait);

int configurator_to_icon();

// PRE-PROCESSOR DEFINITIONS

#undef PUSHBUTTONCB
#define PUSHBUTTONCB(name)\
    void browserShell::name(gtPushButton* btn, gtEvent* evt, void* bs_data, gtReason reas)

// VARIABLE DEFINITIONS

static genString put_message; // Used by "Put"


// FUNCTION DEFINITIONS

//
// returns: <0: error, >=0: the number of selected symbols
//
int
get_all_browser_selected_symbols(symbolArr & selection)
{
    Initialize(get_all_browser_selected_symbols);

    int ret_val = -1;

    int old_sel = selection.size();
    browserShell::get_all_browser_selections(selection);
    ret_val = selection.size() - old_sel;

    return ret_val;
}

inline const projNode* current_project(const char *operation)
{
    const projNode *proj = projNode::get_home_proj();
    if(!proj)
	msg("Home Project must be set before performing $1.", error_sev) << gettext((char *)operation) << eom;
    return proj;
}

inline int valid_selection(const symbolArr &array)
{
    int ret_val = (array.size() > 0);
    if(!ret_val) {
#ifndef NEW_UI
	gtPushButton::next_help_context("Browser.Manage.EmptyError.Help");
#endif
	msg("no_modules_selected:browser_callbacks.C", error_sev) << eom;
    }

    return ret_val;
}

bool browserShell::right_project_browser(int file_browser_flag)
{
    Initialize(browserShell::right_project_browser);

    if (selected_browser == project_browser)
        return TRUE;

    if (selected_browser == 0)
    {
#ifndef NEW_UI
        gtPushButton::next_help_context("Pset.Help.Errors.BrowserSelection");
#endif
	msg("Nothing is selected.", warning_sev) << eom;
        return FALSE;
    }

    if (!file_browser_flag)
    {
#ifndef NEW_UI
        gtPushButton::next_help_context("Pset.Help.Errors.BrowserSelection");
#endif
	msg("Your selection is in the File Browser.\nPlease make your selection in the Project Browser.", warning_sev) << eom;
    }

    return FALSE;
}

#ifndef NEW_UI
static void set_message_options(systemMessageOptions* options)
{
    msg_options(*options, false);
}
#endif

symbolPtr sym_of_subsys(symbolPtr sym)
{
    Initialize(sym_of_subsys);

    symbolPtr xsym = sym.get_xrefSymbol();
    if (xsym.xrisnull())
        return NULL_symbolPtr;

    if (xsym.relationalp()) {
	Relational *rel = (Relational *)xsym;
	if (rel->is_symbolPtr_def())
	    return NULL_symbolPtr;
    }

    if (xsym.get_kind() == DD_SUBSYSTEM)
        return xsym;

    symbolArr arr;
    xsym->get_link(is_defining, arr);
    symbolPtr sym0;
    ForEachS(sym0, arr)
    {
        if (sym0.get_kind() == DD_SUBSYSTEM)
            return sym0;
    }
    return NULL_symbolPtr;
}

static void convert_to_projModules(symbolArr& arr)
//  convert all symbols to projModule, that can be.  Leave the others
//     alone.
{
    Initialize(convert_to_projModules);
    symbolArr local;
    symbolPtr sym;
    ForEachS(sym, arr) {
	local.insert_last(sym);
    }
    arr.removeAll();
    symbolPtr sym2;
    projModulePtr pr;
    ForEachS(sym2, local) {
	if (sym2.relationalp() && !is_projModule(sym2)) {
	    pr = projModule_of_symbol(sym2);
	    if (pr)
		arr.insert_last(pr);
	}else
	    arr.insert_last(sym2);
    }
}

static void operate_on_modules(
    symbolArr& array, int_method_projNode func)
//
// Invoke a projModule member-function on the projModules in array.
// func takes projNode* and returns int.
//
{
    Initialize (operate_on_modules);

    if(!valid_selection(array)) // if nothing is selected, let user know
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
        {
            projNode *pn = module->get_project();
            (module->*func)(pn);
        }
    }
}

static void operate_on_modules(symbolArr& array, projModuleptr_with_data_func func, void* data)
//
// Invoke a projModule member-function on the projModules in array.
// func returns projModule*. Argument void* data is passed on to func as its 
// last argument.
//
{
  Initialize(operate_on_modules);
  
  if(!valid_selection(array))		// if nothing is selected, let user know
    return;
  
  symbolPtr sym;
  
  ForEachS(sym, array) {
    (*func)(sym, data);
  }
  
}

static void operate_on_modules(symbolArr& array, projModuleptr_func func)
//
// Invoke a projModule member-function on the projModules in array.
// func returns projModule*.
//
{
  Initialize(operate_on_modules);

  if(!valid_selection(array))		// if nothing is selected, let user know
    return;
  
  symbolPtr sym;
  
  ForEachS(sym, array) {
    (*func)(sym);
  }
}

static void operate_on_modules(symbolArr& array, projModuleptr_method func)
//
// Invoke a projModule member-function on the projModules in array.
// func returns projModule*.
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

static void operate_on_modules(symbolArr& array, appPtr_method func)
//
// Invoke a projModule member-function on the projModules in array.
// func returns appPtr.
//
{
    Initialize(operate_on_modules);

    if(!valid_selection(array))
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

static void finalize_all_ste_views()
//
// Flush all pending changes in STE views.
//
{
    Initialize(finalize_all_ste_views);
    ste_finalize ((viewPtr)NULL);
}

extern "C" void viewerShell_unmap (viewerShell *);
extern void groupManager_unmap ();

static int do_you_really_want_to_quit()
{
    Initialize(do_you_really_want_to_quit);

    if ( setupProps::instance()->is_modified())
    {
        int answer = dis_question3("", TXT("Quit"),
				   TXT("Save then Quit"),
				   TXT("Quit without Save"), TXT("Cancel"), TXT("Your preferences have been modifed"));
	
        switch (answer)
        {
        case 1:
            customize::save_preferences(NULL);
            break;

        case 2:
            break;

        default:
            return 0;
        }
    }
    return 1;
}

static int do_not_ask_twice = 0;

static int pset_unmap_all_windows ()
{
  do_not_ask_twice = 0;
  if (!driver_instance || !is_gui())
      return 1;

  if (!do_you_really_want_to_quit())
      return 0;

// This unmap all windows breaks emacs19...  I tried
// making all pertinent emacs windows invisible (unmapped)
// but to no avail.  The event loop may need to be synced
// when this is done...  For now this extraneous.
  do_not_ask_twice = 1;
  return 1;
}


#include "CMIntegratorCtrlr.h"
static void really_quit(void*)
//
// Make ParaSET go away.
//
{
    Initialize(really_quit);

    if (!do_not_ask_twice)
    {
        if (!do_you_really_want_to_quit())
            return;
    }

    printf("Deleting the controller... %x\n",browserShell::m_pcmController);
    delete browserShell::m_pcmController;
    browserShell::m_pcmController = NULL;
    printf("The controller has been deleted...\n");

    delete_scratch_apps();
    projHeader::save_all_projects();

    driver_instance->stop_running();
}

void browserShell::quit()		// class static
//
// Polite quit function that can be called from anywhere, e.g. Epoch.
//
{
    Initialize(browserShell::quit);

    browserShellPtr* browser_ptr = instance_array[0];
    gtBase* parent = (browser_ptr ? (*browser_ptr)->shell : NULL);

    //boris: do not wait if it is script execution
    int wait_flag = (cmd_execute_journal_name) ? 0 : 1;

    driver_instance->unselect((viewPtr)NULL);
    // res = 1 - OK, -1 cancelled
    int res = offer_to_save_apps_wait(parent, Flavor_Quitting, NULL, NULL, wait_flag);
    if (res == -1)
	return;

    VALIDATE {
	really_quit ((void *)NULL);
    } else {
        if (pset_unmap_all_windows())
            pset_send_own_callback (really_quit, (void *)NULL);
    }
}


int browserShell::wm_close(void* shell_data)
//
// Perform an orderly and friendly closing of a browserShell window.
//
{
    Initialize(browserShell::wm_close);

    gtTopLevelShell* shell = (gtTopLevelShell*)shell_data;

    // If this is the last browserShell,
    // then turn this into a request to quit.

    if(instance_array.size() == 1)
    {
	objArr* modified = app::get_modified_headers();
	const int no_save_needed = modified->empty();
	delete modified;

	if (no_save_needed) {
	    if (dis_question3(T_NULL, TXT("Confirm DIScover exit?"), B_YES, NULL, B_NO,
                              Q_WANT_TO_QUIT) > 0) {
		really_quit(NULL);
	    }
	} else {
	    //boris: do not wait if it is script execution
	    int wait_flag = (cmd_execute_journal_name) ? 0 : 1;
    
	    int res = offer_to_save_apps_wait(shell, Flavor_Quitting, NULL, NULL, wait_flag);
	    if (res == -1)
		return 0;

	    really_quit ((void *)NULL);
	}
    }

    // Otherwise, find the browserShell that contains the topLevelShell
    // and delete it (which closes the window).

    else
    {
	for(int i = 0; i < instance_array.size(); ++i)
	{
	    browserShell* bs = *instance_array[i];
	    if(bs->shell == shell)
	    {
		delete bs;
		break;
	    }
	}
    }

    return 0;
}

int is_external(projModulePtr);


static int smt_model_is_good(projModule*mod)
{
   app* ah = projectModule_get_appHeader(mod);
   int is_good = 0;
   if(ah && is_smtHeader(ah)){
     smtHeader*smth = smtHeaderPtr(ah);
     is_good = smth->get_actual_mapping();
   } else {
     is_good = mod->is_good_paraset_file();
   }
   return is_good;
}

void msg_model_is_not_good(repType rep_type, char*filename)
{
  char* rep_name=0;
  if (rep_type == Rep_VertHierarchy)
    rep_name = "outline";
  else 	if (rep_type == Rep_FlowChart)
    rep_name = "flowchart";

  if(rep_name)
      msg("Cannot open $1 view for file $2.\n ", warning_sev) << rep_name << eoarg << filename << eom;

}


// module_is_not_viewable defined in xref.h.C
extern bool module_is_not_viewable(projModule *module);

void browserShell::open_views(repType rep_type)
//
// Open views of selected symbols.
//
{
    Initialize(browserShell::open_views);

    if(_lo(LIC_DISCOVER)) {
	_lm(LIC_DISCOVER);
	return;
    } 

    if(selected_browser != project_browser) 
         return;

    symbolArr& selected_symbols = project_browser->symbol_selection();
    symbolArr symbols;

    symbolPtr el;
    symbolArr files_to_open;

    int num_of_exts = 0;         // number of extern (Frame) docs

    bool erd_or_chart_or_map = (rep_type == Rep_ERD || rep_type == Rep_DataChart || rep_type == Rep_SubsystemMap);

    if (!selected_symbols.size()) {
	if(rep_type == Rep_Tree){
	    symbolArr& projects = project_browser->project_selection();
	    files_to_open = projects;
	} else if ( !erd_or_chart_or_map ) {
	    msg("Nothing is selected.", warning_sev) << eom;
	    return;
	}
    } else ForEachS(el, selected_symbols) {
	projModulePtr mod = NULL;
	if(el.isnull())
	    continue;
	if(el.is_xrefSymbol()) {
	    ddKind el_kind = el.get_kind();
	    if (el_kind==DD_ASSOC_CONST ||
		el_kind==DD_SOFT_ASSOC || el_kind==DD_SOFT_ASSOC_TYPE) {
		continue;
	    }
	    // omazor: Local vars and strings cannot be viewed because they may be 
	    //   "declared" in more than one place and in multiple files 
	    if (el_kind==DD_LOCAL || el_kind==DD_STRING) {
		continue;
	    }
	    if(el_kind==DD_MODULE  &&  el.is_loaded())
		mod = projModule_of_symbol(el);
	} else if(el.is_instance()||el.is_ast()||el.is_dataCell()) {
	    
	} else if(is_projModule(RelationalPtr(el))) {
	    mod = projModulePtr(RelationalPtr(el));
	}

	if (!mod ) {
	    symbols.insert_last(el);
	    continue;
	}

	if (mod && module_is_not_viewable(mod)) {
#ifndef NEW_UI
	    gtPushButton::next_help_context("Browser.View.Help.Module");
#endif
	    msg("Object code module $1 is not viewable.", error_sev) << mod->get_name() << eom;
	    continue;
	}

	files_to_open.insert_last(mod);
	if (!mod->is_paraset_file())
	    msg_model_is_not_good (rep_type, el.get_name());
	else if (!smt_model_is_good(mod))
      	    msg("Model is outdated for file \"$1\".", warning_sev) << el.get_name() << eom;
    }

    int to_do = 0;
    int sz    = 0;

    if ( (sz = files_to_open.size()) ) {
	to_do = to_do + sz;
	openFromBrowser command(top_level(), files_to_open, rep_type);
	command.start();
    }
    
    if ( (sz = symbols.size()) || erd_or_chart_or_map) {
	to_do = (sz) ? to_do + sz : to_do + 1 ;
	openFromBrowser command(top_level(), symbols, rep_type);
	command.start();
    }
    
    if (to_do && to_do > num_of_exts)
	view_flush_on_top();
    else if (selected_symbols.size() == 0) {
	msg("Nothing is selected.", warning_sev) << eom;
    }
}

extern void eval_test();

PUSHBUTTONCB(debug_eval)
{
    Initialize(browserShell::file_eval);
    
    eval_test();
}

PUSHBUTTONCB(browser_about_paraset)
{
    Initialize(browserShell::browser_about_paraset);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop 
    
    aboutDialog::instance()->popup();
#endif
}


PUSHBUTTONCB(browser_new_browser_window)
{
    Initialize(browserShell::browser_new_browser_window);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop 
    
    finalize_all_ste_views();

    new browserShell();
#endif
}

PUSHBUTTONCB(browser_close_window)
{
    Initialize(browserShell::browser_close_window);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop 
    
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    wm_close(bs->shell);
#endif
}

PUSHBUTTONCB(browser_preferences)
{
    Initialize(browserShell::browser_preferences);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop 
    
    finalize_all_ste_views();

    setupProps::instance()->popup();
#endif
}

#include "CMIntegratorCtrlr.h"
CCMIntegratorCtrlr* browserShell::m_pcmController = NULL;
PUSHBUTTONCB(cm_command) {
  Initialize(browserShell::cm_command);

  browserShell* bs = (browserShell*)bs_data;
  Widget parent = bs->top_level()->rep()->widget();
  symbolArr& prjSelected = bs->project_browser->project_selection();
  symbolArr& selected = bs->project_browser->symbol_selection();
  
  if(!selected.size() && !prjSelected.size()) {
    msg("Nothing is selected.", warning_sev) << eom;
    return;
  } else {
    symbolPtr el;
    string** filenames = (string**)new string[selected.size()+1];
    int nPos = 0;
    ForEachS(el, selected) {
//      if(el.isnull())  continue;
    
//      if(el.is_xrefSymbol()) {
	filenames[nPos]=new string(ATT_physical_filename(el));
	nPos++;
//      }
    }
    filenames[nPos]=NULL;
    
    string** projnames = (string**)new string[prjSelected.size()+1];
    nPos = 0;
    { // HP and Irix fix
    ForEachS(el, prjSelected) {
      if(el.isnull())  continue;
    
      projnames[nPos]=new string(ATT_physical_filename(el));
      nPos++;
    }
    }
    projnames[nPos]=NULL;
    
    m_pcmController->OnCMCommand(parent,btn->title(),projnames,filenames);
    string* name;
    int i=0;
    while((name=filenames[i])!=NULL) {
      delete name;
      i++;
    }
    i=0;
    while((name=projnames[i])!=NULL) {
      delete name;
      i++;
    }
  }
}

#include <gtCascadeB.h>
#include <gtMenuBar.h>
#include <Xm/DialogS.h>
#include <Xm/PanedW.h>
#include <Xm/PushBG.h>
#include <Xm/Form.h>

void browserShell::buildCMMenu() {
#ifndef NEW_UI
  if(m_pcmController==NULL
         || !m_pcmController->m_CMIntegrator->IsConnected()
         || manage_button == NULL) {
      return;
  }
  
  manage_button->set_sensitive(0);
  // remove all commands
  gtPrimitive* pPrim = manage_button->entry(1);
  while(pPrim!=NULL) { 
      manage_button->remove_entry(0);
      gtPrimitive* pTmp=manage_button->entry(1);
      if(pTmp == pPrim) {
          manage_button->remove_entry(0);
          break;
      }
      pPrim = pTmp;
  }

  CCMCommandsList::iterator iPos = m_pcmController->m_pCMCommands->begin();
  CCMCommandsList::iterator iEnd = m_pcmController->m_pCMCommands->end();
  while(iPos!=iEnd) {
    CCMCommand* pCommand = *iPos;
    string& cmdName = pCommand->GetName();
    if(!cmdName.empty()) 
	manage_button->insert_entries(-1, gtMenuStandard, "submenu_item",
			       (char*)cmdName.c_str(), this, cm_command, gtMenuNull);
    iPos++;
  }
  manage_button->set_sensitive(1);
#endif
}

PUSHBUTTONCB(browser_cm)
{
  Initialize(browserShell::browser_cm);
   
  browserShell* bs = (browserShell*)bs_data;
  Widget parent = bs->top_level()->rep()->widget();
  if(m_pcmController==NULL) m_pcmController = CCMIntegratorCtrlr::getInstance();

  if(m_pcmController->SelectCM(parent)==1) {
      bs->buildCMMenu();
  }   
}

PUSHBUTTONCB(browser_messages)
{
    Initialize(browserShell::browser_messages);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop 
    
    finalize_all_ste_views();

    popup_SystemMessages(&msg_options(), set_message_options);
#endif
}


PUSHBUTTONCB(browser_quit)
{
    Initialize(browserShell::browser_quit);
#ifndef NEW_UI
    browserShell* bs = (browserShell*)bs_data;

    finalize_all_ste_views();
    //boris: do not wait if it is script execution
    int wait_flag = (cmd_execute_journal_name) ? 0 : 1;
    
    int res = offer_to_save_apps_wait (bs->shell, Flavor_Quitting, NULL, NULL, wait_flag);

    if (res != -1) {
	VALIDATE {
		really_quit ((void *)NULL);
	} else {
                if (pset_unmap_all_windows())
                    pset_send_own_callback (really_quit, (void *)NULL);
	}
	return;
    }
    
    if (res == -1)
	msg("Quit canceled", warning_sev) << eom;
#endif
}

PUSHBUTTONCB(file_save_all_files)
{
    Initialize(browserShell::file_save_all_files);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop 
    
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    objArr* modified = app::get_modified_headers();
    if (modified->size())
	offer_to_save_apps(bs->shell, Flavor_Saving, NULL, NULL);
    else
	msg("No files need saving.", warning_sev) << eom;

    delete modified;

    projHeader::save_all_projects();
#endif
}

void preload_selection(symbolArr& modules)
{
    Initialize(preload_selection);
    
    convert_to_projModules(modules);

    // Tell browser_import to store all files passed.
    // It shall store them until told to import them all at once.
    const char* file_name = "not_used";
    browser_import(NULL, file_name, (const char*)"begin list");
    
    // Pass all the files to be imported to browser_import.
    // Note that load_module calls browser_import.
    operate_on_modules(modules, &projModule::load_module);
    
    // Import all the files
    browser_import(NULL, file_name, (const char*)"end list");
}

PUSHBUTTONCB(file_preload)
{
    Initialize(browserShell::file_preload);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop 
    
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    if (bs->right_project_browser(0))
    {
	symbolArr& modules = bs->project_browser->symbol_selection();
	preload_selection(modules);
    }
    bs->project_browser->deselect();
#endif
}

int els_reparse_modules(symbolArr& modules, symbolArr& rest);

void update_selection(symbolArr& mods)
{
    Initialize(update_selection);
    
    convert_to_projModules(mods);
    
    symbolArr* modules = &mods, rest;

    if (!els_reparse_modules(mods, rest))
	  return;
    else
#ifdef XXX_update_selection
#error TODO: expect that all parsing is done via els.
#else
	  modules = & rest;
    
    // Tell browser_import to store all files passed.
    // It shall store them until told to import them all at once.
    const char* file_name = "not_used";
    browser_import(NULL, file_name, (const char*)"begin list");
    
    // Pass all the files to be imported to browser_import.
    // Note that load_module calls browser_import.
    operate_on_modules(*modules, &projModule::update_reparse_module);
    
    // Import all the files
    browser_import(NULL, file_name, (const char*)"end list");
#endif
}

PUSHBUTTONCB(file_update)
// This is the menu item that forces a reparse, even if the file is
//  already loaded.  It also loads the ast.
{
    Initialize(browserShell::file_update);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop 
    
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    if (bs->right_project_browser(0))
    {
	symbolArr& modules = bs->project_browser->symbol_selection();
	update_selection(modules);
    }
    bs->project_browser->deselect();
#endif
}

void unload_selection(symbolArr &);

PUSHBUTTONCB(file_unload)
{
    Initialize(browserShell::file_unload);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop 
    
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    // Summary:
    // Build a symbolArr of selected projModules, and build an objArr
    // containing the app-headers corresponding to the selected projModules
    // Pass these two arrays to offer_to_save_list, which will offer to
    // save the appropriate app-headers before passing the projModules to
    // really_unload (above).

    if (bs->right_project_browser(0)) {
	symbolArr& symbols = bs->project_browser->symbol_selection();
	unload_selection(symbols);
    }
    
    bs->project_browser->deselect();
#endif
}

void language_selection(symbolArr &modules)
{
    Initialize(language_selection);
    
    convert_to_projModules(modules);

    // if nothing selected let user know
    if (!valid_selection(modules))
	return;

    // get language from user
#ifndef NEW_UI
    gtPushButton::next_help_context("Browser.File.Language.Help");
#endif
    fileLanguage language = fileLanguageDlg::get_language (NULL);
    if (language == FILE_LANGUAGE_UNKNOWN) return;
    symbolPtr sym;
    ForEachS(sym, modules)
    {
        projModule* module;
	
        if(sym.is_xrefSymbol())
            module = projModule_of_symbol(sym);
        else if(is_projModule(sym))
            module = projModulePtr(RelationalPtr(sym));
	
	if (module) module->language (language);
    }
}

PUSHBUTTONCB(file_language)
{
    Initialize(browserShell::file_language);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop 
    
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    // set language on selected stuff
    if(bs->selected_browser != bs->project_browser) return;

    symbolArr& modules = bs->project_browser->module_selection();
    language_selection(modules);
    bs->project_browser->deselect();
#endif
}

static projMap *global_pm = 0;


extern void setup_home_from_selection(symbolArr&);
PUSHBUTTONCB(project_home_project)
{
    Initialize(browserShell::project_home_project);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop
 
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    symbolArr& projects = bs->project_browser->project_selection();
    setup_home_from_selection(projects);
    bs->project_browser->deselect();
    bs->project_browser->refresh_browsed_project();
#endif
}

PUSHBUTTONCB(project_get)
{
    Initialize(browserShell::project_get);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop
 
    if (projNode::get_home_proj()->is_paraset_project()) {
#ifndef NEW_UI
	gtPushButton::next_help_context("Pset.Help.Get.MustSetHome");
#endif
	msg("Cannot do a GET without setting a home project", error_sev) << eom;
	return;
    }
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    if (bs->right_project_browser(0))
    {
        symbolArr& modules = bs->project_browser->symbol_selection();

	if(current_project(TXT("Get"))) {
	    if (customize::configurator_get_comments() && 
		(!strcmp(customize::configurator_system(), "clearcase") || 
		 !strcmp(customize::configurator_system(), "continuus") || 
		 !strcmp(customize::configurator_system(), "custom"))) {
		
	        getUI dlg;
		genString comm;
		
		if (dlg.ask(comm)>0) {
		    genString tnm_for_comment = create_one_tmp_file(comm);
		    operate_on_modules(modules, &projModule::get_module_with_data, (void*)tnm_for_comment.str());
		    OSapi_unlink(tnm_for_comment);
		}
	    } else
	      operate_on_modules(modules, &projModule::get_module);
	    configurator_to_icon();
	}
    }
    bs->project_browser->deselect();
#endif
}

PUSHBUTTONCB(project_get2)
{
    Initialize(browserShell::project_get);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop
 
    if (projNode::get_home_proj()->is_paraset_project()) {
	msg("Cannot do a GET without setting a home project", error_sev) << eom;
	return;
    }
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    if (bs->right_project_browser(0))
    {
        symbolArr & modules = bs->project_browser->symbol_selection();

	if(current_project(TXT("Get"))) {
#ifndef NEW_UI
	    gtPushButton::next_help_context("Browser.Manage.Get.WithOptions.Help");
#endif
            Prompt prompt(bs->top_level(), TXT("Get Options"),
                          TXT("Enter options for Get command:"));
            
            genString name(customize::configurator_get());
            if(prompt.ask(name) > 0 ) {
                if (strcmp((char *)name, customize::configurator_get()) != 0)
                    customize::configurator_get((char *) name);
                customize::save_configurator(NULL);
    
		operate_on_modules(modules, &projModule::get_module);

		configurator_to_icon();
	    }
	}
    }
    bs->project_browser->deselect();
#endif
}

static bool good_date(char *date)
// check if the string "date" contains a good date
// The range for month, hour, minute, second is easy to understand.
// day for February or April is not checked completely: Ex: both 02/31/94 and 04/31/94
//    are bad. But it is treated as good.
// year: only accept to 2099. This bug will appear 100 years later.
//
// The check is not 100% accurate. But it is OK, because this date is used to display
// the versions of a file after this date.
{
    Initialize(good_date);

    int month=0,day=0,year=0,hour=0,minute=0,second=0;
    int num;

    num = OSapi_sscanf (date,"%d/%d/%d %d:%d:%d", &month, &day, &year, &hour, &minute, &second);

    if (num < 3 || num > 6) return 0;

    if (month < 0 || month > 12)   return 0;
    if (day   < 0 || day   > 31)   return 0;
    if (year  < 0 || year  > 2100) return 0;
    if (num > 3)
    {
        if (hour   < 0 || hour   > 24) return 0;
        if (minute < 0 || minute > 60) return 0;
        if (second < 0 || second > 60) return 0;
    }
    return 1;
}

void browserShell::project_get_by_version_date(char date_type)
// date_type = 0: don"t need date: display all version numbers
// date_type = 1; need date: display all version numbers after specified date
//
// customer could select one version form the displayed list and get the file
//  of that version
{
    Initialize(project_get_by_version_date);

    if (projNode::get_home_proj()->is_paraset_project())
    {
	msg("Cannot do a GET without setting a home project", error_sev) << eom;
        return;
    }

    browserShell* bs = this;
    finalize_all_ste_views();

    if (bs->right_project_browser(0))
    {
        symbolArr &modules = bs->project_browser->symbol_selection();

        if(modules.size() != 1)
	    msg("You must select exactly one module.", error_sev) << eom;
        else if (current_project(TXT("Get")))
        {
            projModule *mod = checked_cast(projModule, modules[0]);
            genString fn;
            mod->get_phys_filename(fn);
            if (!xref_not_txt_file(fn))
            {
#ifndef NEW_UI
		gtPushButton::next_help_context("Pset.Help.Get.CantGetDocByVersion");
#endif
		msg("Cannot get a version of a document file.", error_sev) << eom;
                return;
            }
            if (mod->is_raw_modified(0))
                return;

            genString date = 0;

            if (date_type)
            {
#ifndef NEW_UI
		gtPushButton::next_help_context("Browser.Manage.Get.ByDate.Prompt.Help");
#endif
                Prompt prompt(bs->top_level(), TXT("Copy by Date (after)"),
                              TXT("Enter the date of the file you wish to get:\n(Format: [mm/dd/yy [hh:mm:ss]])"));

                if (prompt.ask(date) == 1)
                {
                    if ( date.length()) // Ignore empty strings
                    {
                        if (!good_date(date)) {
#ifndef NEW_UI
			    gtPushButton::next_help_context("Pset.Help.Get.EnterDateFormat");
#endif
			    msg("Enter the date with format 'mm/dd/yy hh:mm:ss'", error_sev) << eom;
			}
                    }
                }
                else
                    return;
            }

            getFileSelector *gfs;
            if (date_type) {
#ifndef NEW_UI
		gtPushButton::next_help_context("Browser.Manage.Get.ByDate.Selector.Help");
#endif
                gfs = new getFileSelector(date, mod, GET_BY_DATE);
	    }
            else {
#ifndef NEW_UI
		gtPushButton::next_help_context("Browser.Manage.Get.ByVersion.Selector.Help");
#endif
                gfs = new getFileSelector(date, mod, GET_BY_VERSION);
	    }
        }
    }
}

PUSHBUTTONCB(project_get_version)
{
    Initialize(browserShell::project_get_version);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop
 
    ((browserShell*)bs_data)->project_get_by_version_date(0);
#endif
}

PUSHBUTTONCB(project_get_date)
{
    Initialize(browserShell::project_get_date);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop
 
    ((browserShell*)bs_data)->project_get_by_version_date(1);
#endif
}

PUSHBUTTONCB(project_copy)
{
    Initialize(browserShell::project_copy);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop
 
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    if (bs->right_project_browser(0))
    {
        symbolArr& modules = bs->project_browser->symbol_selection();

        if(current_project(TXT("Copy"))) {
            operate_on_modules(modules, &projModule::copy_module);
	    configurator_to_icon();
	}
    }
    bs->project_browser->deselect();
#endif
}

PUSHBUTTONCB(project_diff)
// the callback routine for manage->diff button in the project browser
{
    Initialize(browserShell::project_diff);
#ifndef NEW_UI
    tempCursor the_cursor; // busy cursor push/pop

    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    if (bs->right_project_browser(0))
    {
        symbolArr& modules = bs->project_browser->symbol_selection();

        if(current_project(TXT("Diff")))
        {
            operate_on_modules(modules, &projModule::diff_module);
            configurator_to_icon();
        }
    }
    bs->project_browser->deselect();
#endif
}

bool browser_unget_module (gtBase* parent, projModule *module, bool yes_to_all)
// unget one module after ask the confirmation
{
    Initialize(browser_unget_module);

    if (!module) return false;

    if (!(module->is_home_project()))
    {
	msg("To unget, the Home Project must be set to this project", error_sev) << eom;
        return false;
    }

    int ans;
    if (yes_to_all)
        ans = 2;
    else
    {
#ifndef NEW_UI
        gtPushButton::next_help_context("Browser.File.Unget.Help");
#endif
        genString txt;
        txt.printf(TXT("OK to unget module?\n\"%s\""), module->get_name());
        ans = dis_question3("Browser.File.Unget.Help", TXT("Unget"),
                             TXT("Yes"), TXT("Yes to All"), TXT("Cancel"), txt.str());
    }

    if (ans == 1 || ans == 2)
        module->unget_module();

    if (ans == 2)
        return true;

    return false;
}

PUSHBUTTONCB(project_unget)
// the browser callback routine to unget a module
{
    Initialize(browserShell::project_unget);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop

    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    if (bs->right_project_browser(0))
    {
        symbolArr& modules = bs->project_browser->symbol_selection();

        symbolPtr sym;
        projModule *module = NULL;

        bool yes_to_all = false;
        ForEachS(sym, modules)
        {
            if (sym.is_xrefSymbol())
            {
                if(sym.get_kind() == DD_MODULE)
                   module = projModule_of_symbol(sym);
            }
            else if (is_projModule (sym))
                module = checked_cast (projModule,sym);

            if (module)
                yes_to_all = browser_unget_module(bs->top_level(), module, yes_to_all);
        }
        configurator_to_icon();
    }
    bs->project_browser->deselect();
#endif
}


int execManageTool(genString, int);

genString browserShell::getTool(int n)
{
    genString s;
    if ( 1<= n && n <=8 )
	return cmdTool[n-1];
    else
	return s;
}

char* browserShell::getToolTitle(int n)
{
    char* s="";
    if ( 1<= n && n <=8 )
	return (char*)(titleTool[n-1].str());
    else
	return s;
}

void browserShell::setTool(int n, genString title, genString tool, int outFlag)
{
    if ( 1<= n && n <=8 )
    {
	cmdTool[n-1] = tool;
	titleTool[n-1] = title;
	outMsgFlag[n-1] = outFlag;
    }
}

int browserShell::getOutFlag(int n)
{
    if ( 1<= n && n <=8 )
	return outMsgFlag[n-1];
    else
	return 0;
}

void browserShell::project_execTool( int n, void*  bs_data)
{
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
 
    browserShell* bs = (browserShell*)bs_data;
    execManageTool( bs->getTool(n), bs->getOutFlag(n) );
#endif
}
  
PUSHBUTTONCB(project_Tool1)
{
    Initialize(browserShell::project_Tool1);
    project_execTool( 1, bs_data );
}
PUSHBUTTONCB(project_Tool2)
{
    Initialize(browserShell::project_Tool2);
    project_execTool( 2, bs_data  );
}
PUSHBUTTONCB(project_Tool3)
{
    Initialize(browserShell::project_Tool3);
    project_execTool( 3 , bs_data );
}
PUSHBUTTONCB(project_Tool4)
{
    Initialize(browserShell::project_Tool4);
    project_execTool( 4, bs_data );
}
PUSHBUTTONCB(project_Tool5)
{
    Initialize(browserShell::project_Tool5);
    project_execTool( 5, bs_data );
}
PUSHBUTTONCB(project_Tool6)
{
    Initialize(browserShell::project_Tool6);
    project_execTool( 6, bs_data );
}
PUSHBUTTONCB(project_Tool7)
{
    Initialize(browserShell::project_Tool7);
    project_execTool( 7, bs_data );
}
PUSHBUTTONCB(project_Tool8)
{
    Initialize(browserShell::project_Tool8);
    project_execTool( 8 , bs_data);
}



PUSHBUTTONCB(view_new_viewer)
{
    Initialize(browserShell::view_new_viewer);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop
 
    finalize_all_ste_views();

    viewerShell* vsh = new viewerShell(CREATE_MINIBROWSER);
    view_set_target_viewer(vsh->create_viewer());
    viewerShell::set_current_vs(vsh);
    vsh->bring_to_top();
    vsh->map();
#endif
}


PUSHBUTTONCB(view_text)
{
    Initialize(browserShell::view_text);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop
 
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    bs->open_views(Rep_SmtText);
#endif
}

PUSHBUTTONCB(view_graph)
{
    Initialize(browserShell::view_graph);
#ifndef NEW_UI
    tempCursor the_cursor;

    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    baseBrowser *selected_browser = bs->selected_browser;
    projectBrowser *pb = bs->project_browser;
 
    if (selected_browser == pb) {
	symbolArr  target;
        symbolArr& source = pb->symbol_selection();

	(void) generate_call_graph(source, target);
    } else {
	msg("Please make a selection in the project browser", error_sev) << eom;
    }
#endif
}

PUSHBUTTONCB(view_outline)
{
    Initialize(browserShell::view_outline);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop
 
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    bs->open_views(Rep_VertHierarchy);
#endif
}

//Guy: add the callback for the view->flowchart
PUSHBUTTONCB(view_flowchart)
{
    Initialize(browserShell::view_flowchart);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop 

    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    bs->open_views(Rep_FlowChart);
#endif
}

PUSHBUTTONCB(view_call_tree)
{
    Initialize(browserShell::view_call_tree);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop
 
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    bs->open_views(Rep_Tree);
#endif
}

PUSHBUTTONCB(view_inheritance)
{
    Initialize(browserShell::view_inheritance);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop
 
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    bs->open_views(Rep_OODT_Inheritance);
#endif
}

PUSHBUTTONCB(view_relationships)
{
    Initialize(browserShell::view_relationships);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop
 
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    bs->open_views(Rep_ERD);
#endif
}

PUSHBUTTONCB(view_structures)
{
    Initialize(browserShell::view_structures);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop
 
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    bs->open_views(Rep_DataChart);
#endif
}

PUSHBUTTONCB(view_subsystems)
{
    Initialize(browserShell::view_subsystems);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop
 
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    bs->open_views(Rep_SubsystemMap);
#endif
}


extern int get_modified_objects(symbolArr &, symbolArr & );

static char *make_delta_name() {
  char temp[32], *name;
  static int num = 0;
  symbolArr* existing_group;
  do {
    sprintf(temp, "DELTA_%d", num++);
    existing_group = extGroupApp::find(temp);
  } while ( existing_group );

  name = new char[strlen(temp)+1];
  strcpy(name, temp);
  return name;
}

#include "ImpactComponent.h"
PUSHBUTTONCB(utility_check_impact)
{
  Initialize(browserShell::utility_check_impact);
  browserShell* bs = (browserShell*)bs_data;
  Widget parent = bs->top_level()->rep()->widget();
  CImpactComponent* pComp = new CImpactComponent(parent);
  
  symbolArr& selected = bs->project_browser->symbol_selection();
  symbolArr symbols;
  symbolPtr el;
  
  if(!selected.size()) {
    msg("Nothing is selected.", warning_sev) << eom;
    return;
  } else ForEachS(el, selected) {
    if(el.isnull())  continue;
    
    if(el.is_xrefSymbol()) {
      ddKind el_kind = el.get_kind();
      symbols.insert_last(el);
    }
  }

  pComp->SetSelectedEntities(symbols);
  pComp->SetVisible(true);
}

PUSHBUTTONCB(utility_mod_objects)
{
  Initialize(browserShell::utility_mod_objects);
#ifndef NEW_UI
  tempCursor the_cursor;	// busy cursor push/pop
 
  browserShell* bs = (browserShell*)bs_data;
  finalize_all_ste_views();
    
  if (bs->right_project_browser(0))
  {
    symbolArr& selected = bs->project_browser->symbol_selection();  
    symbolArr *modified = new symbolArr;
    get_modified_objects(selected, *modified);
    projectBrowser* pb = (projectBrowser*)bs->project_browser;
    if (modified->size()) {
      char *name = make_delta_name();
      extGroupApp::create(name,modified);
      delete [] name;
    } else {
      delete modified;
    }
  }  
#endif
}

void expand_groups(symbolArr& selected)
{
    Initialize(expand_groups);

    symbolArr expanded_groups;
    symbolArr groups_to_remove;
    int i;
    for(i = 0; i < selected.size(); i++){
	symbolPtr sym = selected[i].get_xrefSymbol();
	if(sym.xrisnull())
	    continue;
	if(sym->get_kind() == DD_SUBSYSTEM){
	    groups_to_remove.insert_last(selected[i]);
	    sym->get_link(grp_has_pub_mbr, expanded_groups);
	}
    }
    for(i = 0; i < groups_to_remove.size(); i++)
	selected.remove(groups_to_remove[i]);
    selected.insert_last(expanded_groups);
}

PUSHBUTTONCB(utility_simplify_headers) {
    Initialize(browserShell::utility_simplify_headers);
#ifndef NEW_UI 
    tempCursor the_cursor;      // busy cursor push/pop
    browserShell* bs = (browserShell*) bs_data;
    finalize_all_ste_views();
    if (bs->right_project_browser(0)) {
	symbolArr& selected = bs->project_browser->symbol_selection();
	int nSelectedCount = selected.symbols();
	if(nSelectedCount>0) {
	  symbolPtr sym;
	  ForEachS(sym,selected) {
	    int nLang = sym.get_language();
	    if((nLang!=FILE_LANGUAGE_C && nLang!=FILE_LANGUAGE_CPP) ||
	       sym.get_kind()!=DD_MODULE)
	      nSelectedCount--;
	    else {
	      char* pPhysFileName = sym.get_name();
	      if(pPhysFileName==NULL || !xref_not_header_file(pPhysFileName))
		nSelectedCount--;
	    }
	  }
	}
	if(nSelectedCount>0)
	  simplify_headers(selected);
	else 
	  popup_Information("Information","No C/C++ modules has been selected. Nothing to simlify.");
    }
#endif
}

PUSHBUTTONCB(utility_find_dormant_code)
{
    Initialize(browserShell::utility_find_dormant_code);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop
 
    if (!externInfo::enabled()) {
        return;
    }
 
    browserShell* bs = (browserShell*)bs_data;
    if (_lo(LIC_DORMANT)) {
        _lm(LIC_DORMANT);
        return;
    }
 
    finalize_all_ste_views();
    doFindDeadCode(bs->top_level());
#endif
}

//
// The following callback functions are for internal development only.
//

PUSHBUTTONCB(debug_break)
{
    Initialize(browserShell::debug_break);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
 
    apl_PROJ_Debug_Break();
#endif
}

PUSHBUTTONCB(debug_print_selected_symbol)
{
    Initialize(browserShell::debug_print_selected_symbol);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
 
    browserShell* bs = (browserShell*)bs_data;
    projectBrowser *project_browser = bs->project_browser;

    if(bs->selected_browser == project_browser) {
	symbolArr& symbols = project_browser->symbol_selection();
	if(symbols.size()){
           symbolPtr sym;
           ForEachS(sym, symbols){
              node_prtS(sym);
           }
        }
    }
#endif
}

PUSHBUTTONCB(debug_print_selected_node)
{
    Initialize(browserShell::debug_print_selected_node);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
    apl_PROJ_Debug_Print_Selected_Node();
#endif
}

void browserShell::debug_follow(
    gtPushButton* button, gtEvent*, void*, gtReason)
{
    Initialize(browserShell::debug_follow);
#ifndef NEW_UI
    int branch = (int)strtol(
	(char*)button->name() + strlen("debug_follow_"), NULL, 0);
    msg("I\'ll follow $1") << branch << eom;
    apl_PROJ_Debug_Follow(branch);
#endif
}

PUSHBUTTONCB(debug_print_selected_tree)
{
    Initialize(browserShell::debug_print_selected_tree);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
    apl_PROJ_Debug_Print_Selected_Tree();
#endif
}

PUSHBUTTONCB(debug_start_metering)
{
    Initialize(browserShell::debug_start_metering);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
    apl_PROJ_start_metering();
#endif
}

PUSHBUTTONCB(debug_stop_metering)
{
    Initialize(browserShell::debug_stop_metering);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
    Prompt prompt(NULL, TXT("Metering Root Function"),
          TXT("Enter name of root function for metering output:"));
    genString answer;
    if (prompt.ask(answer) > 0) {
       apl_PROJ_stop_metering(answer);
    }
    global_cache.stats();
#endif
}

PUSHBUTTONCB(debug_suspend_metering)
{
    Initialize(browserShell::debug_suspend_metering);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
    apl_PROJ_suspend_metering();
#endif
}

PUSHBUTTONCB(debug_resume_metering)
{
    Initialize(browserShell::debug_resume_metering);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
    apl_PROJ_resume_metering();
#endif
}

PUSHBUTTONCB(debug_edit_help_index)
{
    Initialize(browserShell::debug_edit_help_index);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
    HelpUI_instance_edit_widget_index(); 
#endif
}

PUSHBUTTONCB(debug_reset_help_index)
{
    Initialize(browserShell::debug_reset_help_index);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
    HelpUI_instance_dump_widget_index();
#endif
}

PUSHBUTTONCB(debug_toggle_help_pathnames)
{
    Initialize(browserShell::debug_toggle_help_pathnames);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
    boolean new_value = HelpUI_get_set_show_pathnames();
    msg("++++++++++\nShow Pathnames: $1\n++++++++++") << ((char*)(new_value ? "Enabled" : "Disabled")) << eom;
#endif
}

PUSHBUTTONCB(debug_toggle_use_helper_shell)
{
    Initialize(browserShell::debug_toggle_use_helper_shell);
#ifndef NEW_UI 
    tempCursor the_cursor;    // busy cursor push/pop 
    boolean new_value = HelpUI_get_set_use_helper_shell();
    msg("++++++++++\nUse Helper Shell: $1\n++++++++++") << ((char*)(new_value ? "Enabled" : "Disabled")) << eom;
#endif
}

PUSHBUTTONCB(debug_clear_all_selections)
{
    Initialize(browserShell::debug_clear_all_selections);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
    dr_clear_selection();
#endif
}


PUSHBUTTONCB(debug_new_journal)
{
    Initialize(browserShell::debug_new_journal);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
 
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();
    JournalDlg JournalDlg(bs->top_level());
    genString filename, creator, create_date, pdf_name, project_name;
    if(JournalDlg.ask(filename, creator, create_date, pdf_name, project_name) > 0
       && filename.length()) 
	cmd_journal_start(filename, creator, create_date, pdf_name, project_name);
    browserShell::journal_manage_menus();
#endif
}

PUSHBUTTONCB(debug_insert_journal_command)
{
    Initialize(browserShell::debug_insert_journal_command);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
 
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    Prompt prompt(bs->top_level(), TXT("Journal Command"),
		  TXT("Enter a journal command:"));
    genString jc;
    if(prompt.ask(jc) > 0  &&  jc.length())
	cmd_journal_insert_line(jc);
#endif
}

PUSHBUTTONCB(debug_save_journal)
{
    Initialize(browserShell::debug_save_journal);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
    finalize_all_ste_views();

    cmd_journal_save (NULL);
    browserShell::journal_manage_menus();
#endif
}

PUSHBUTTONCB(debug_resume_execution)
{
    Initialize(browserShell::debug_resume_execution);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
    finalize_all_ste_views();

    cmd_exec_resume ();
#endif
}

PUSHBUTTONCB(debug_save_journal_as)
{
    Initialize(browserShell::debug_save_journal_as);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
 
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();
    Prompt prompt(bs->top_level(), TXT("Save Journal As"),
		  TXT("Enter a name for the journal file:"),
                  TXT("No filename selected."));
    genString filename;
    if(prompt.ask(filename) > 0  &&  filename.length())
	cmd_journal_save(filename);
    browserShell::journal_manage_menus();
#endif
}

PUSHBUTTONCB(debug_execute_journal)
{
    Initialize(browserShell::debug_execute_journal);
#ifndef NEW_UI
    tempCursor the_cursor;    // busy cursor push/pop
 
    browserShell* bs = (browserShell*)bs_data;
    finalize_all_ste_views();

    gtPushButton::next_help_context("Pset.Help.Internals.RunJournal");
    Prompt prompt(bs->top_level(), TXT("Execute Journal"),
		  TXT("Enter a name for the journal file:"),
                  TXT("No filename selected."));
    genString filename;
    if(prompt.ask(filename) > 0  &&  filename.length())
	cmd_journal_exec_within(filename);
#endif
}

