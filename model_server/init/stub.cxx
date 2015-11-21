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
#include <machdep.h>
#include <messages.h>
#include <genString.h>
#include <symbolPtr.h>
#include <objOper.h>
#include <objRawApp.h>
#include <linkTypes.h>
//#include <ldr.h>
#include <RTL.h>
#include <symbolPtr.h>
#include <proj.h>
#include <assoc.h>
#include <ParaCancel.h>
#include <BrowserController.h>
#include <msg.h>
#include <cmd_group.h>
#include <ddict.h>
#include <dd_or_xref_node.h>

#ifdef WIN32
#include <tcl.h>
#include <evalMetaChars.h>
#else
#include <ui_INTERP_info.h>
#endif
#include <dd_boil.h>

#define MINIMUM_SEV warning_sev

#include <proj_save.h>
#include <metric.h>

#include <NewPrompt.h>
#include <dis_iosfwd.h>
#include <ste_smod.h>

static void model_server_stub (const char* func_name, msg_sev sev=error_sev)
{
    if(sev >= MINIMUM_SEV)
	msg("!!! $1 called", sev) << func_name << eom;
}

void cmd_validate (const char *, const char *)
{
    model_server_stub ( "cmd_validate (const char *, const char *)", warning_sev);
}

void cmd_validate_tree(const char *, objTree*)
{
    model_server_stub ( "cmd_validate_tree(const char *, objTree*)", warning_sev);
}

int cmd_validate_string_value(const char *, const char *)
{
    model_server_stub ( "cmd_validate_string_value(const char *, const char *)", warning_sev);
    return 0;
}

void cmd_validate (const char *, class symbolArr&)
{
    model_server_stub ( "cmd_validate (const char* ,symbolArr&)", warning_sev);
}

void cmd_validation_error   (const char *)
{
    model_server_stub ( "cmd_validation_message", warning_sev);
}

void ste_interface_toggle_app(objRawApp *, app *)
{
    model_server_stub ( "ste_interface_toggle_app(objRawApp *, app *)", normal_sev );
}

void logger_start_transaction()
{
    model_server_stub ( "logger_start_transaction()", normal_sev );
}

void logger_end_transaction()
{
    model_server_stub ( "logger_end_transaction()", normal_sev );
}

void ste_smart_view_update(smtHeader *, int)
{
    model_server_stub("ste_smart_view_update(smtHeader *, int)", warning_sev);
}

void ste_set_freeze_status(int)
{
    model_server_stub("ste_set_freeze_status(int)", warning_sev);
}

void add_to_pending_lists(smtHeader *)
{
    model_server_stub("add_to_pending_lists(smtHeader *)", normal_sev);
}

void enable_reparse_buttons(smtHeader *, boolean)
{
    model_server_stub("enable_reparse_buttons(smtHeader *, boolean)", warning_sev);
}

int ste_temp_view_suspend(smtHeader*)
{
    model_server_stub("ste_temp_view_suspend(smtHeader*)", warning_sev);
    return 1;
}

int put_is_running(void)
{
    return 0;
}

void pset_send_own_callback (void (*func)(void *), void *dt)
{
    model_server_stub("pset_send_own_callback(void (*)(void *), void *)", warning_sev);
    if (func) (*func)(dt);
}

void ste_interface_modified(HierarchicalPtr,boolean)
{
    model_server_stub("ste_interface_modified(HierarchicalPtr, boolean)", normal_sev);
}

int update_browser()
{
    return 0;
}

void check_auto_save_copy(char *)
{
    model_server_stub("check_auto_save_copy(char *)", normal_sev);
}

int dis_question(const char *, const char *, const char *, const char *, ...)
{
    model_server_stub("dis_question", error_sev);
    return 0;
}

int cmd_available_p (int)
{
    // This function tells discover whether to use the modern version
    // of various pieces of functionality, or some ancient version.
    model_server_stub ("cmd_available_p (int)", normal_sev);
    return 1;
}

int browserShell_update_existing_project_node(projNode *)
{
    model_server_stub("browserShell_update_existing_project_node(projNode *)", warning_sev);
    return 0;
}

int check_cursor()
{
    return 0;
}

void cleanup_miniBrowsers()
{
    model_server_stub("cleanup_miniBrowsers", warning_sev);
}

int has_view (app *)
{
    model_server_stub("has_view (app *)", warning_sev);
    return 0;
}

int is_aseticons_available()
{
    model_server_stub ("is_aseticons_available()", warning_sev);
    return 0;
}

int is_objRawApp (Relational *)
{
    model_server_stub ("is_objRawApp (Relational *)", normal_sev);
    return 0; // objRawApps are created only in project/ui, api/ui
}

int is_steDocument (Relational *)
{
    model_server_stub ("is_steDocument (Relational *)", warning_sev);
    return 0;
}

int is_InstanceRep (Relational *)
{
    model_server_stub ("is_InstanceRep (Relational *)", warning_sev);
    return 0;
}

extern "C" void errorBrowser_show_errors()
{
    model_server_stub ("errorBrowser_show_errors()", error_sev);
}

extern "C" void errorBrowserClear()
{
    model_server_stub ("errorBrowserClear", warning_sev);
}

extern "C" int Tk_Init(Tcl_Interp *)
{
    model_server_stub ("Tk_Init(Tcl_Interp *)", warning_sev);
    return 0;
}
 
void ste_set_preferences(int, int)
{
    model_server_stub ("void ste_set_preferences(int, int)", warning_sev);
}

int ste_interface_raw_save (objRawApp *)
{
    model_server_stub ("ste_interface_raw_save (objRawApp *)", error_sev);
    return 0;
}

int shell_configurator_reset()
{
    model_server_stub ("shell_configurator_reset()", warning_sev);
    return 0;
}

extern "C" void push_busy_cursor()
{
    model_server_stub ("push_busy_cursor()", normal_sev);
}

extern "C" void pop_cursor()
{
    model_server_stub ("pop_cursor()", normal_sev);
}

objArr *commonTree::search (commonTree *, void *, int, int)
{
    model_server_stub ("commonTree::search (commonTree *, void *, int, int)", error_sev);
    return (objArr *)0;
}

objArr *RTL::search (commonTree *, void *, int, int)
{
    model_server_stub ("RTL::search (commonTree *, void *, int, int)", error_sev);
    return (objArr *)0;
}

int is_real_rename (appTree *, char const *)
{
    model_server_stub ("is_real_rename (appTree *, char const *)", error_sev);
    return 0;
}

void dd_delete_dd_or_xref_node_views (Obj *)
{
    model_server_stub ("dd_delete_dd_or_xref_node_views (Obj *)", normal_sev);
}

extern "C" void cmd_journal_dump_pset(app *)
{
    model_server_stub ("cmd_journal_dump_pset(app *)", warning_sev);
}

char * cmd_current_journal_name = NULL;
char * cmd_execute_journal_name = NULL;

extern "C" int cmd_do_validation(CMD_VALIDATION_GROUP)
{
    model_server_stub ("cmd_do_validation(CMD_VALIDATION_GROUP)", warning_sev);
    return 0;
}

int cmd_logging_on = 0;

void cmd_validate (const char *, int)
{
    model_server_stub ("cmd_validate (const char *, int)", warning_sev);
}

int is_inheritanceSymTreeHdr (Relational *)
{
    model_server_stub ("is_inheritanceSymTreeHdr (Relational *)", warning_sev);
    return 0;
}

ParaCancel *ParaCancel::canceller;

bool get_closure_state() {
    model_server_stub ("get_closure_state()", warning_sev);
    return false;
}

void insert_ldr_headers (dd_or_xref_node *)
{
    model_server_stub ("insert_ldr_headers (dd_or_xref_node *)", warning_sev);
}

int setup_configurator_envs()
{
    return -1;
}

int projModule_set_version (app *)
{
    model_server_stub ("projModule_set_version (app *)", normal_sev);
    return -1;
}

int projModule_set_version_start (projModule *)
{
    model_server_stub ("projModule_set_version_start (projModule *)", warning_sev);
    return -1;
}

bool delete_module_internal_call_cm (projModule *, int, genString, genString,
				     genString, genString)
{
    model_server_stub ("delete_module_internal_call_cm", warning_sev);
    return false;
}
 
void delete_module_internal_call_triggers (projModule *, int, int)
{
    model_server_stub ("delete_module_internal_call_triggers", warning_sev);
}
 
void delete_module_internal_finish (char const *, projNode *)
{
    model_server_stub ("delete_module_internal_finish", warning_sev);
}

void mpb_init()
{
    model_server_stub ("mpb_init()", normal_sev);
}

void mpb_incr (int)
{
    model_server_stub ("mpb_incr (int)", normal_sev);
}

void mpb_info(const char *)
{
    model_server_stub ("mpb_info (const char *)", normal_sev);
}

void mpb_refresh()
{
    model_server_stub ("mpb_refresh()", normal_sev);
}

void mpb_destroy()
{
    model_server_stub ("mpb_destroy()", normal_sev);
}

void mpb_hide()
{
    model_server_stub ("mpb_hide()", normal_sev);
}

void mpb_popup()
{
    model_server_stub ("mpb_popup()", normal_sev);
}

void mpb_incr_values_init(int, int &, int &)
{
    model_server_stub ("mpb_incr_values_init(int, int &, int &)", normal_sev);
}

void mpb_step(int, int, int, int &, int)
{
    model_server_stub ("mpb_step(int, int, int, int &, int)", normal_sev);
}

void mpb_mpb_segment(int, int)
{
    model_server_stub ("mpb_mpb_segment(int, int)", normal_sev);
}

void get_reference_status(smtTree**, objOperType*) 
{
    model_server_stub ("void get_reference_status(smtTree**, objOperType*)", normal_sev);
}

boolean smtHeader::is_correct(appTree*, appTree*) 
{
    model_server_stub ("bool smtHeader::is_correct(appTree*, appTree*)", normal_sev);
    return 0;
}

class Application {
public:
    static Application *findApplication(char const*, char const *); 
};
Application *Application::findApplication(char const*, char const *) 
{ 
    model_server_stub ("static Application* Application::findApplication(char const*, char const *)", normal_sev);
    return 0; 
}

void rcall_dis_DISui_eval_async(Application *,unsigned char *)
{
    model_server_stub ("rcall_dis_DISui_eval_async(Application*, unsigned char *)", normal_sev);
}

bool is_include (appTree *, appTree *)
{
    model_server_stub ("is_include(appTree*, appTree*)", normal_sev);
    return 0;
} 

int is_steTextNode(Relational *)
{
    model_server_stub ("is_steTextNode(Relational *)", normal_sev);
    return 0;
}

bool is_smtRefNode(Relational * const)
{
    model_server_stub ("is_smtRefNode(Relational *const)", normal_sev);
    return 0;
}

void smod_rebuild_node(smtTree *,SmodVisibility,int)
{
    model_server_stub ("smod_rebuild_node(smtTree*,SmodVisibility,int)", normal_sev);
}

void smod_set_visibility(void *)
{
    model_server_stub ("smod_set_visibility(void*)", normal_sev);
}

#ifdef NEW_UI
void set_logical_file_name(genString)
{
    model_server_stub ("set_logical_file_name(genString)", normal_sev);
}

void set_physical_file_name(genString)
{
    model_server_stub ("set_physical_file_name(genString)", normal_sev);
}

void set_project_name(genString)
{
    model_server_stub ("set_project_name(genString)", normal_sev);
}

void set_Date(genString)
{
    model_server_stub ("set_Date(genString)", normal_sev);
}

void set_user_name(genString)
{
    model_server_stub ("set_user_name(genString)", normal_sev);
}

void set_author_name(genString)
{
    model_server_stub ("set_author_name(genString)", normal_sev);
}

void set_target_name(genString)
{
    model_server_stub ("set_target_name(genString)", normal_sev);
}

void set_source_name(genString)
{
    model_server_stub ("set_source_name(genString)", normal_sev);
}

void set_member_name(genString)
{
    model_server_stub ("set_member_name(genString)", normal_sev);
}

void set_struct_name(genString)
{
    model_server_stub ("set_struct_name(genString)", normal_sev);
}

void set_function_name(genString)
{
    model_server_stub ("set_function_name(genString)", normal_sev);
}

void set_class_name(genString)
{
    model_server_stub ("set_class_name(genString)", normal_sev);
}

#else
void set_dd_logical_file_name(genString)
{
    model_server_stub ("set_dd_logical_file_name(genString)", normal_sev);
}

void set_dd_physical_file_name(genString)
{
    model_server_stub ("set_dd_physical_file_name(genString)", normal_sev);
}

void set_dd_project_name(genString)
{
    model_server_stub ("set_dd_project_name(genString)", normal_sev);
}

void set_dd_Date(genString)
{
    model_server_stub ("set_dd_Date(genString)", normal_sev);
}

void set_dd_user_name(genString)
{
    model_server_stub ("set_dd_user_name(genString)", normal_sev);
}

void set_dd_author_name(genString)
{
    model_server_stub ("set_dd_author_name(genString)", normal_sev);
}

void set_dd_target_name(genString)
{
    model_server_stub ("set_dd_target_name(genString)", normal_sev);
}

void set_dd_source_name(genString)
{
    model_server_stub ("set_dd_source_name(genString)", normal_sev);
}

void set_dd_member_name(genString)
{
    model_server_stub ("set_dd_member_name(genString)", normal_sev);
}

void set_dd_struct_name(genString)
{
    model_server_stub ("set_dd_struct_name(genString)", normal_sev);
}

void set_dd_function_name(genString)
{
    model_server_stub ("set_dd_function_name(genString)", normal_sev);
}

void set_dd_class_name(genString)
{
    model_server_stub ("set_dd_class_name(genString)", normal_sev);
}
#endif //NEW_UI

#ifdef NEW_UI
genString Boilerplate::filter_boilerplate(genString )
{
    model_server_stub ("static genString Boilerplate::filter_boilerplate(genString)", normal_sev);
    return 0; 
}
#else
genString dd_boil::filter_boilerplate(genString )
{
    model_server_stub ("static genString dd_boil::filter_boilerplate(genString)", normal_sev);
    return 0; 
}
#endif

//  XXX: new-AST: stub out from ast_dd.cxx; revive when we're back doing linking again.
extern "C" int set_common_tentative_defs (int val)
{
  return val;
}
