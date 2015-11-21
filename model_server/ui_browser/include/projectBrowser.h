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
#ifndef _projectBrowser_h
#define _projectBrowser_h

// projectBrowser.h
//------------------------------------------
// synopsis:
//------------------------------------------

#ifndef _genArr_h
#include <genArr.h>
#endif
#ifndef _genArrCharPtr_h
#include <genArrCharPtr.h>
#endif
#ifndef _gt_h
#include <gt.h>
#endif
#ifndef _genMask_h
#include <genMask.h>
#endif
#ifndef _symbolArr_h
#include <symbolArr.h>
#endif
#ifndef _baseBrowser_h
#include <baseBrowser.h>
#endif
#ifndef _Relational_h
#include <Relational.h>
#endif
#ifndef _browserRTL_h
#include <browserRTL.h>
#endif
#ifndef _gtDlgTemplate_h
#include <gtDlgTemplate.h>
#endif
#ifndef _gtLabel_h
#include <gtLabel.h>
#endif
#ifndef _symbolScope_h
#include <symbolScope.h>
#endif
      
#ifndef _groupHdr_h
#include <groupHdr.h>
#endif      

#ifndef FILE
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */
#endif

class BrowserController;
class projectBrowser;
RelClass(ldrSubsysMapHierarchy);

struct groupHandle {
    projectBrowser *a_project_browser;
    symbolArr      *sym_array;
    RTLPtr         a_group;
    char           *a_group_name;
    int            *selected_indices;
    int            selected_count;
    int            group_list_index;
};

typedef int (projectBrowser::*projectBrowserMember)(groupHandle *);

RelClass(RTL);
genArr(RTLPtr);
class browserShell;


class projectBrowser : public baseBrowser
{
  public:
    projectBrowser(browserShell*, RTLPtr);
    ~projectBrowser();

    void capture_RTL(char *);
    void capture_RTL(char *, int);

    RTLPtr browsed_project();
    symbolArr& project_selection();
    symbolArr& module_selection();
    symbolArr& symbol_selection();
    void open(symbolPtr);
    void browse(RTLPtr);
    void browse_mode();
    void scan_mode();
    void reset();
    void closure(bool enabled);
    void init_search_path();

    gtRTL * get_domain_list() {return domain_list; }
    const char* mode();
    const char* project_pathname(genString&);
    const char* selected_groups(genString&);
    const char* selected_queries(genString&);
    void        select_group(const char*);
    void        select_query(const char*);

    virtual void filter(const char*);
    virtual void deselect();

    void obtain_all_projects(symbolArr &pn, int &top_proj);
    void force_up(int up_level);

    int exists(char *);
    static void create_group_subsystem(const char *, groupHdrPtr client, ldrSubsysMapHierarchyPtr ldrh);
    symbolArr& selectedSymbols();
    void getSelectedSymbols(symbolArr &);

    int addGroup(groupHandle *);
    int renameGroup(groupHandle *);
    int deleteGroup(groupHandle *);
    int getGroups(groupHandle *);
    int getGroupTitleNumber(groupHandle *);
    int collectSelected(groupHandle *);
    int selectGroup(groupHandle *);

    void set_groupParent(projectBrowser *pb) { groupParent = pb; }
    int is_groupParent(projectBrowser *pb) { return (pb == groupParent); }
    gtTopLevelShell *create_new_group_dlg();

    void         reset_group_dialog(projectBrowser *);
    void         attach_groupManager(projectBrowser *);
    static void  bring_groupManager(projectBrowser *);
    void         capture_group_from_query(symbolArr& , projectBrowser*);
    gtRTL        *get_hierarchy_list() { return hierarchy_list; }
    gtPushButton *get_reset_button() { return reset_button; }
    void         create_group_from_arr(symbolArr &, char *group_name = NULL);
    symbolScope  *get_scope(void) { return &scope; };
    void         insert_array_in_results(symbolArr& array);
    void         insert_array_in_domain(symbolArr& array);

    void edit_wildcard_group();

    static void set_browse_group_filter_toggle_button();

    void convert_group();
    void split_group();
    void merge_group();

    void move_element();

    void public_group();
    void private_group();
    void vis_group();
    void invis_group();
    void refresh_browsed_project();

    BrowserController* get_browser_controller() {return pBController;}
    void update_categories();
    void update_ask();

    static bool get_closure_state();
    static void set_closure_state(bool enabled);

    static void print_group(const char *name);
    static void add_group_name(const char *name, bool is_subsys);
    static void batch_root_impact( const char* root_grp, const char* mod_grp,FILE*cm_report_file, FILE* report_file);

    static void hierarchy_select(gtList*, gtEventPtr, void*, gtReason);
    static void domain_select(gtList*, gtEventPtr, void*, gtReason); 
    static void result_select(gtList*, gtEventPtr, void*, gtReason);
    static void rtl_action(gtList*, gtEventPtr, void*, gtReason);

  protected:
    virtual gtPrimitive* build_body(gtBase*);

  private:
    static projectBrowser *groupParent;
    void difference_groups(int, int);

    static void up_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void scan_mode_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void browse_mode_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void reset_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void capture_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void closure_CB(gtToggleButton*, gtEventPtr, void*, gtReason);
    static void print_group_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void save_model_group_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void save_scratch_group_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void import_group_rtl_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void global_groups_CB(gtPushButton*, gtEventPtr, void*, gtReason);
    static void group_select(gtList*, gtEventPtr, void*, gtReason);
    static void all_groups_action_CB(gtList*, gtEventPtr, void*, gtReason);
    static void show_select(gtList*, gtEventPtr, void*, gtReason);

    static void add_elements_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void remove_elements_CB(gtPushButton*, gtEvent*, void*, gtReason);

    static void intersect_groups_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void union_groups_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void difference_groups_ab_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void difference_groups_ba_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void complement_groups_CB(gtPushButton*, gtEvent*, void*, gtReason);

    static void create_group_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void rename_group_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void delete_group_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void convert_group_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void split_group_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void merge_group_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void relate_group_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void move_element_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void public_group_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void private_group_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void vis_group_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void invis_group_CB(gtPushButton*, gtEvent*, void*, gtReason);

    static gtToggleButton* browse_group_filter_toggle_button;

    static void browse_group_members_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void browse_group_clients_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void browse_group_servers_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void browse_group_peers_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void browse_group_members_used_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void browse_group_using_entities_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void browse_group_members_using_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void browse_group_used_entities_CB(gtPushButton*, gtEvent*, void*, gtReason);
    static void browse_group_filter_CB(gtPushButton*, gtEvent*, void*, gtReason);

    static void group_action_CB(gtList*, gtEvent*, void*, gtReason);
    static void quit_groupManager_CB(gtPushButton*, gtEvent*, void*, gtReason);

    static bool _closure;

    const char* group_name(unsigned int);
    const char* query_name(unsigned int);

//    static void  parse_names_init(const char*);
//    static const char* parse_names_next();

    gtForm*             gt_form_top;
    gtPushButton*       up_button;
    gtPushButton*       reset_button;
    gtPushButton*       capture_button;
    gtRadioBox*         browse_scan_radiobox;
    gtToggleButton*	closure_button;
    gtForm*             gt_form_hierarchy;
    gtRTL*              hierarchy_list;
    RTLPtr              top_proj;
    RTLPtr              cur_proj;

    gtForm*             gt_form_xref;
  public:
    gtRTL*              domain_list;    
    gtList*             group_list;
    gtList*             show_list;
    gtRTL*              result_list;
    genMask		show_links;
  public:
    symbolArr           selected_projects;
    symbolArr           selected_modules;
    symbolArr           selected_symbols;
    symbolArr           result_selection;
    symbolArr           domain_selection;
    symbolArr           hier_selection;
  private:
    genArrOf(RTLPtr)    group_lists;

    browserRTL *sel_rtl;
    gtLabel *nm_label;
    
    genArrCharPtr group_items;
    genArrCharPtr show_items;
    BrowserController* pBController;
    static void fill_list(gtList* list, const genArrCharPtr& items);

    symbolScope scope;

    void update_links();
    void switch_xref(int);

    gtBase* build_xref_query(gtBase*);
    void delete_selection(gtRTL* keep = NULL);
    void rtl_select(int);
};


inline RTLPtr projectBrowser::browsed_project() { return cur_proj; }

extern symbolPtr sym_of_subsys(symbolPtr sym);
extern char *ddKind_external_name(ddKind kind);
extern projModule *projModule_of_symbol(symbolPtr& sym);
extern void get_link_add_results(symbolPtr sym, linkType lt, symbolSet& results);
extern void get_temporary_group_name(genString& name);
extern void update_rem_group_manager(const char* name);

/*
    START-LOG-------------------------------

    $Log: projectBrowser.h  $
    Revision 1.33 2000/10/17 14:18:52EDT sschmidt 
    Removal of Extract, DefectLink, Testlink, Package, Modularity, Partition
 * Revision 1.2.1.28  1994/06/12  12:16:28  so
 * Bug track: n/a
 * add cancel button for refresh projects
 *
 * Revision 1.2.1.27  1994/04/06  14:23:19  jon
 * Re-Packaging ParaSET and it's use of License Management
 *
 * Revision 1.2.1.26  1994/03/31  00:37:58  so
 * Bug track: 6659
 * fix bug 6659
 *
 * Revision 1.2.1.25  1994/03/15  15:02:43  davea
 * Bug track: 6615
 * new functions  get_hierarchy_list,
 * refresh_loaded_files,
 *
 * Revision 1.2.1.24  1993/12/15  18:12:17  andrea
 * Bug track: 5028
 * I implemented functionality allowing the user to create groups from query
 *
 * Revision 1.2.1.23  1993/12/02  02:58:25  pero
 * New Group functionality
 *
 * Revision 1.2.1.22  1993/08/20  13:39:38  dmitry
 * fixed RTL print/save
 *
// Revision 1.1  1993/08/19  18:04:40  dmitry
// Initial revision
//
 * Revision 1.2.1.21  1993/08/12  21:04:45  dmitry
 * part of save list option fix
 *
 * Revision 1.2.1.20  1993/08/11  15:16:53  dmitry
 * Parts of print list and save list options implementation
 *
 * Revision 1.2.1.19  1993/07/15  22:09:12  sergey
 * Added get_domain_list() method.
 *
 * Revision 1.2.1.18  1993/06/15  20:13:58  andrea
 * fixed bug 3613
 *
 * Revision 1.2.1.17  1993/06/06  17:50:25  mg
 * capture_group
 *
 * Revision 1.2.1.16  1993/06/04  18:50:29  andrea
 * model option works and allows user inputted projects as well as multiple projects
 *
 * Revision 1.2.1.15  1993/05/07  00:15:04  glenn
 * Add parse_names_init, parse_names_next, init_search_path.
 *
 * Revision 1.2.1.14  1993/02/18  03:52:21  glenn
 * Remove "refresh" argument from update_links.
 * Add "extend" argument to rtl_select.
 *
 * Revision 1.2.1.13  1993/01/20  03:40:32  glenn
 * Add scan_mode, mode, project_pathname, selected_groups, selected_queries,
 * select_group, select_query, group_name, query_name, browse_scan_radiobox.
 *
 * Revision 1.2.1.12  1993/01/17  21:48:01  glenn
 * Remove delete_project_browser.
 *
 * Revision 1.2.1.11  1993/01/16  04:30:46  glenn
 * Remove unused include files and typedefs from previous interface.
 * Remove unused member functions and variables.
 *
 * Revision 1.2.1.10  1993/01/13  06:16:32  glenn
 * Add browsed_project() inline.
 *
 * Revision 1.2.1.9  1993/01/12  03:42:47  glenn
 * Remove default argument in update_links.
 *
 * Revision 1.2.1.8  1993/01/10  06:09:29  glenn
 * Add browse, global_local, reset_button, capture_button.
 * Remove top_CB.
 *
 * Revision 1.2.1.7  1993/01/08  21:23:12  glenn
 * Remove old and/or disabled code.
 * Add use_local_xref, switch_xref, to_global_CB, to_local_CB.
 *
 * Revision 1.2.1.6  1993/01/07  14:51:16  glenn
 * New browserShell interface.
 *
 * Revision 1.2.1.5  1992/12/15  20:37:37  aharlap
 * changed for new xref
 *
 * Revision 1.2.1.4  1992/12/12  07:09:57  glenn
 * Add view_structures, view_subsystems.
 *
 * Revision 1.2.1.3  1992/11/07  19:48:41  sharris
 * changed function prototype definition of file_commit_files (BUG 1613)
 *
 * Revision 1.2.1.2  1992/10/09  19:57:12  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif // _projectBrowser_h
