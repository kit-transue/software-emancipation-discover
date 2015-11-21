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
#ifndef _browserShell_h
#define _browserShell_h

// browserShell.h
//------------------------------------------
// synopsis:
// High-level container for file and project browsers.
//
//------------------------------------------

#ifndef _gt_h
#include <gt.h>
#endif

#ifndef _representation_h
#include <representation.h>
#endif

//#ifndef _directory_h
//#include <directory.h>
//#endif

#ifndef _genArr_h
#include <genArr.h>
#endif

#ifndef _Relational_h
#include <Relational.h>
#endif

#ifndef _projectBrowser_h
#include <projectBrowser.h>
#endif

#ifndef _ddict_h
#include <ddict.h>
#endif

#ifndef _symbolArr_h
#include <symbolArr.h>
#endif
#include <transaction.h>

class CCMIntegratorCtrlr;
class baseBrowser;
class projectBrowser;
class projectHeader;
class symbolArr;

RelClass(RTL);
RelClass(projNode);


typedef class browserShell* browserShellPtr;
genArr(browserShellPtr);

typedef void (*projectSelectionCB)(void*, symbolArr&);

typedef struct _select_explode_data {
    ddKind    kind;            // kind of current view node     
    symbolArr symbols;        // array of Xrefs for the future;
                               // for now contains just one symbol
} SELECT_EXPLODE_DATA;

struct cliQueryData;
struct cliMenu;
struct browserQueryMenu : public Relational {
    browserShell *browser;
    gtCascadeButton*button;
};
struct browserQueryData {
    browserQueryMenu *menu;
    cliQueryData *data;
};

class loadedFiles;
class browserShell
{
  public:
    static const char* const DEFAULT_LAYOUT_FILE;
    static const char* const LAYOUT_FORMAT_VERSION;

    static void startup();
    static const char* scanned_projects();

    static void browser_manage_menus (int turn_on=0);
    static void journal_manage_menus ();
    static void reset_all();       // reset all the browsers
    static void unmap_all();       // unmap all the browsers
    void toAllBrowsers(projectBrowserMember, groupHandle *);
    static void set_closure_state(bool closure_enabled);

    static CCMIntegratorCtrlr* m_pcmController;

    static void get_all_browser_selections (symbolArr&, bool include_projects = false);
    static void clear_all_browser_selections (browserShell* = NULL);
    static void add_group_to_projectBrowser(symbolArr& arr, genString& name);

    browserShell();
    browserShell(gtBase* parent, const char* prompt, projectSelectionCB, void* data);
                 
    ~browserShell();

    bool right_project_browser(int file_browser_flag);

    gtShell* top_level();

    gtBase* container();
    void select(projectBrowser*);
    void default_action();

    RTLPtr browsed_project();
    void disable_apply_button();
    void update_domains();

    void obtain_all_projects(symbolArr &pn, int &top_proj);
    void force_up(int up_level);

    static void quit();
    const char *get_layout_name() { return (char *)layout_name; }
    static void update_existing_project_node(projNode *pn);

    void build_query_menu(cliMenu *, gtCascadeButton *);

    static void project_execTool( int nTool, void*  bs);
    char*     getToolTitle(int);
    int       getOutFlag(int);
    genString getTool(int);
    void      setTool(int, genString, genString, int);

  private:
    static genArrOf(browserShellPtr) instance_array;
    static const char* BROWSER_CONFIG_VERSION;

    genString cmdTool[8];
    genString titleTool[8];
    int       outMsgFlag[8];

    baseBrowser* selected_browser;
    projectBrowser* project_browser;

    gtShell* shell;
    gtBase* browser_container;

    projectSelectionCB project_selection;
    void* project_selection_data;

    gtCascadeButton* manage_button;
    int num_make_menu_entries;

    genString layout_name;

    gtCascadeButton* cascade_button;

    void buildCMMenu();
    void browser_manage_menu (int turn_on=0);
    void journal_manage_menu ();

    void project_get_by_version_date(char date_type);

    // level = 1: one level
    // level = 0: all levels
    projNode *cur_projnode();

    void project_merge(int level);


#undef PUSHBUTTONCB
#define PUSHBUTTONCB(name)\
    static void name(gtPushButton*, gtEvent*, void*, gtReason)

    PUSHBUTTONCB(browser_about_paraset);
    PUSHBUTTONCB(browser_new_browser_window);
    PUSHBUTTONCB(browser_close_window);
    PUSHBUTTONCB(browser_cm);
    PUSHBUTTONCB(browser_preferences);
    PUSHBUTTONCB(browser_configurator);
    PUSHBUTTONCB(browser_messages);
    PUSHBUTTONCB(browser_quit);
    PUSHBUTTONCB(file_save_all_files);
    PUSHBUTTONCB(file_preload);
    PUSHBUTTONCB(file_update);
    PUSHBUTTONCB(file_unload);
    PUSHBUTTONCB(file_language);
    PUSHBUTTONCB(project_home_project);
    PUSHBUTTONCB(project_get);
    PUSHBUTTONCB(project_get2);
    PUSHBUTTONCB(project_get_version);
    PUSHBUTTONCB(project_get_date);
    PUSHBUTTONCB(project_copy);
    PUSHBUTTONCB(project_diff);
    PUSHBUTTONCB(project_put);
    PUSHBUTTONCB(project_quick_merge);
    PUSHBUTTONCB(project_detail_merge);
    PUSHBUTTONCB(project_lock);
    PUSHBUTTONCB(project_unlock);
    PUSHBUTTONCB(project_unget);
    PUSHBUTTONCB(project_obsolete);
    
    PUSHBUTTONCB(cm_command);

    PUSHBUTTONCB(project_Tool1);
    PUSHBUTTONCB(project_Tool2);
    PUSHBUTTONCB(project_Tool3);
    PUSHBUTTONCB(project_Tool4);
    PUSHBUTTONCB(project_Tool5);
    PUSHBUTTONCB(project_Tool6);
    PUSHBUTTONCB(project_Tool7);
    PUSHBUTTONCB(project_Tool8);

    PUSHBUTTONCB(view_new_viewer);
    PUSHBUTTONCB(view_text);
    PUSHBUTTONCB(view_graph);
    PUSHBUTTONCB(view_outline);
    PUSHBUTTONCB(view_flowchart);    
    PUSHBUTTONCB(view_call_tree);
    PUSHBUTTONCB(view_inheritance);
    PUSHBUTTONCB(view_relationships);
    PUSHBUTTONCB(view_structures);
    PUSHBUTTONCB(view_subsystems);
    PUSHBUTTONCB(view_document);
    PUSHBUTTONCB(utility_check_impact);
    PUSHBUTTONCB(utility_find_dormant_code);
    PUSHBUTTONCB(utility_metric_analysis);
    PUSHBUTTONCB(utility_error_browser);
    PUSHBUTTONCB(utility_mod_objects);
    PUSHBUTTONCB(utility_move_entities);
    PUSHBUTTONCB(utility_simplify_headers);
    PUSHBUTTONCB(report_stat_report);
    PUSHBUTTONCB(debug_new_journal);
    PUSHBUTTONCB(debug_insert_journal_command);
    PUSHBUTTONCB(debug_save_journal);
    PUSHBUTTONCB(debug_resume_execution);
    PUSHBUTTONCB(debug_save_journal_as);
    PUSHBUTTONCB(debug_execute_journal);
    PUSHBUTTONCB(debug_break);
    PUSHBUTTONCB(debug_eval);
    PUSHBUTTONCB(debug_print_selected_node);
    PUSHBUTTONCB(debug_print_selected_symbol);
    PUSHBUTTONCB(debug_follow);
    PUSHBUTTONCB(debug_print_selected_tree);
    PUSHBUTTONCB(debug_start_metering);
    PUSHBUTTONCB(debug_stop_metering);
    PUSHBUTTONCB(debug_suspend_metering);
    PUSHBUTTONCB(debug_resume_metering);
    PUSHBUTTONCB(debug_edit_help_index);
    PUSHBUTTONCB(debug_reset_help_index);
    PUSHBUTTONCB(debug_toggle_help_pathnames);
    PUSHBUTTONCB(debug_toggle_use_helper_shell);
    PUSHBUTTONCB(debug_check_parametric_model);
    PUSHBUTTONCB(debug_clear_all_selections);
    PUSHBUTTONCB(utility_groupManager);
    PUSHBUTTONCB(utility_subsystemManager);
    PUSHBUTTONCB(utility_qualityCockpit);
    PUSHBUTTONCB(utility_temp_null_function);


    static int  wm_close(void*);

    static void remember(browserShellPtr);
    static void forget(browserShellPtr);

    static void ok_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void apply_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void cancel_CB(gtPushButton*, gtEventPtr, void*, gtReason);

    void open_views(repType);
    void open_assoc_browser (void);
    void open_assoc_remover (void);

    void build_dialog(gtBase*, const char*);
    void build_interface();
    gtMenuBar* build_body();
    void build_file_menu(gtMenuBar*);
    void build_project_menu(gtMenuBar*);
    void build_view_menu(gtMenuBar*);
    void build_utility_menu(gtMenuBar*);
    void build_journal_menu(gtMenuBar*);
    void build_debug_menu(gtMenuBar*);
    void build_query_menu(cliMenu*cli_menu) { build_query_menu(cli_menu, NULL); }
    void build_query_menus(gtMenuBar*);
    void build_help_menu(gtMenuBar*);
    
  public:
    projectBrowser *get_project_browser() { return project_browser; }
    PUSHBUTTONCB(project_merge_from);
#undef PUSHBUTTONCB
    static genArrOf(browserShellPtr)* get_browsers_list() { return &instance_array; };
    static void display_assist_line( const char *txt, int);
    static void add_item_to_cliMenu(cliMenu*, const char *name, void *callback);
    static void add_query_submenu(cliMenu*, int item_ind);
    static void add_query_menu(cliMenu*cli_menu);

    static void change_scope_txt(char*string);
    static void change_scope_mode(char*string);

  private:
    gtStringEditor *assist_line;

  public:
    gtCascadeButton* query_cascade_btn;
    gtMenuBar*top_menubar;
    
};

void browser_set_rtl(gtRTL* result_list, symbolArr& array);
inline gtShell* browserShell::top_level() { return shell; }

extern void browser_merge_modules(symbolArr& modules, int level, int from_put);
void   browserShell_project_merge(symbolArr& modules, int level);

#endif
