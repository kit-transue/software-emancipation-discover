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
// INCLUDE FILES
#include <cLibraryFunctions.h>
#include <msg.h>
#if defined(sun5)
#include <cstdlib>
using namespace std;
#else
#include <math.h>      // 'real' systems put 'abs' here.
#endif
#include <xxinterface.h>
#include <messages.h>
#include <Question.h>
#define _xref_h
#define _dd_forward_h
#include <cmd.h>
#include <cmd_enums.h>
#include <db.h>
#include <gtTopShell.h>
#include <gtMainWindow.h>
#include <gtForm.h>
#include <gtPanedForm.h>
#include <gtCascadeB.h>
#include <gtPushButton.h>
#include <gtStringEd.h>
#include <gtSepar.h>
#include <gtList.h>
#include <gtRTL.h>
#include <gtDialog.h>
#include <gtMenuBar.h>
#include <gtOptionMenu.h>
#include <gtDlgTemplate.h>
#include <help.h>
#include <group.h>
#include <gtBaseXm.h>
#include <ParaCancel.h>
#include <top_widgets.h>
#include <view_creation.h>
#include <shell_calls.h>
#include <NewPrompt.h>
#include <path.h>
#include <xref_queries.h>
#include <viewerShell.h>
#include <ldrList.h>
#include <ldrSelection.h>
#include <steSlot.h>
#include <steTextNode.h> 
#include <OODT_apl_entries.h>
#include <autosubsys.h>
#include <closure.h>
#include <SetsUI.h>
#include <ldrSubsysMapHierarchy.h>
#include <popup_QueryAndFetch.h>
#include <driver.h>
#include <gtTogB.h>
#include <browserShell.h>
#include <ddKind_readable_names.h>
#include <LanguageController.h>
#include <BrowserController.h>
#include <BrowserData.h>
#include <Interpreter.h>

static int file_cat(FILE *, FILE *);

// VARIABLE DEFINITIONS
static gtTopLevelShell *dlg = NULL;
gtToggleButton* projectBrowser::browse_group_filter_toggle_button = NULL;
projectBrowser *projectBrowser::groupParent = NULL;
static gtToggleButton* global_groups_button = NULL;
static const int SCAN_LIST_ROWS = 10;
bool projectBrowser::_closure = false;
static gtToggleButton* group_mgr_closure_button = NULL;

// FUNCTION DEFINITIONS

//------------------------------------------
// closure manipulations
//------------------------------------------

bool projectBrowser::get_closure_state()
{
  Initialize(get_closure_state);
  return _closure;
}

void projectBrowser::set_closure_state(bool enabled) {
   Initialize(set_closure_state);

   _closure = enabled;
   static loop_preventer = false;
   if (!loop_preventer) {
      loop_preventer = true;
      browserShell::set_closure_state(enabled);
      viewerShell::set_closure_state(enabled);
      ::set_closure_state(enabled);
      if (group_mgr_closure_button) {
#ifndef NEW_UI
	 group_mgr_closure_button->set(enabled, 0);
#endif
      }
      loop_preventer = false;
   }
}

static symbolArr selected;


static bool consistent_member_filter(bool wants_publics, bool wants_privates,
				     bool wants_files, bool wants_directs,
				     bool wants_indirects, bool wants_locals,
				     bool wants_transitives) {
   Initialize(consistent_member_filter);

   if (!(wants_publics || wants_privates)) {
      pop_cursor();
      msg("The specified filter excludes both public and private members", error_sev) << eom;
      return false;
   }
   if (!(wants_files || wants_directs || wants_indirects)) {
      pop_cursor();
      msg("The specified filter excludes both file and program entity members", error_sev) << eom;
      return false;
   }
   if (!(wants_locals || wants_transitives)) {
      pop_cursor();
      msg("The specified filter excludes both local and transitive members", error_sev) << eom;
      return false;
   }
   if (wants_transitives && !wants_publics && !wants_locals) {
      pop_cursor();
      msg("Cannot filter for transitive members when there are no public members.", error_sev) << eom;
      return false;
   }
   return true;
}



const char* projectBrowser::group_name(unsigned int g)
//
// Return name of group based on index.
//
{
    Initialize(projectBrowser::group_name);

    return (g < group_items.size()) ? *group_items[g] : NULL;
}


const char* projectBrowser::query_name(unsigned int q)
//
// Return name of query based on index.
//
{
    Initialize(projectBrowser::query_name);

    return (q < show_items.size()) ? *show_items[q] : NULL;
}


void projectBrowser::select_group(const char* gname)
//
// If gname is non-NULL, select a group by name.
// Do not act on the new selection until gname is NULL.
//
{
    Initialize(projectBrowser::select_group);
#ifndef NEW_UI
    if(gname)
    {
	group_list->sel_type(gtMultiple);
	group_list->select_item(gname, 0);
	group_list->sel_type(gtExtended);
    }
     else
 	group_select(group_list, NULL, this, gtReason(0));
#endif
 }
 
 
void projectBrowser::select_query(const char* qname)
// If qname is non-NULL, select a query by name.
// Do not act on the new selection until qname is NULL.
//
{
    Initialize(projectBrowser::select_query);
#ifndef NEW_UI
    if(qname)
    {
	show_list->sel_type(gtMultiple);
	show_list->select_item(qname, 0);
	show_list->sel_type(gtExtended);
    }
     else
 	show_select(show_list, NULL, this, gtReason(0));
#endif
}

gtBase* projectBrowser::build_xref_query(gtBase* parent)
//
// Create the GT components of the project query interface.
//
{
#ifndef NEW_UI

     gtPanedForm* form = gtPanedForm::create(parent, "xref_query");

     group_list = gtList::create(
         (gtBase *)form, (const char *)"group_list", (const char *)(TXT("Categories")), 
         (gtListStyle)gtExtended, NULL, 0);
 
     domain_list = gtRTL::create(
         form, "domain_list", TXT("Elements"), gtExtended, NULL, 0);
 
     show_list = gtList::create(
         (gtBase *)form, (const char *)"show_list", (const char *)(TXT("Ask")), 
         (gtListStyle)gtExtended, NULL, 0);
 
     result_list = gtRTL::create(
         form, "result_list", 
	 TXT("Results"), gtExtended, NULL, 0);
 
     group_list->num_rows(SCAN_LIST_ROWS);
     
     form->add_child(group_list, 180);
 
     domain_list->num_rows(SCAN_LIST_ROWS);
     domain_list->gt_list()->width(200);

     pBController = LanguageController::GetBrowserController();

     update_categories();
     update_ask();
          
     form->add_child(domain_list, 200);
 
     show_list->num_rows(SCAN_LIST_ROWS);
     
     form->add_child(show_list, 180);
     
     result_list->num_rows(group_items.size());
     result_list->gt_list()->width(200);
     
     form->add_child(result_list, 240);

#if 1
     group_list->select_callback(group_select, this);
     group_list->action_callback(all_groups_action_CB, this);
#endif
     domain_list->select_callback(domain_select, this);
     domain_list->action_callback(rtl_action, this);
     show_list->select_callback(show_select, this);
     result_list->select_callback(result_select, this);
     result_list->action_callback(rtl_action, this);

     groupHandle handle;
     handle.a_project_browser = this;
     bsh.toAllBrowsers(&projectBrowser::getGroups, &handle);

     capture_button->manage();
     reset_button->manage();
     closure_button->manage();
     group_list->manage();
     domain_list->manage();
     show_list->manage();
     result_list->manage();
     
     form->layout(); 

     return form;
#else
     return NULL;
#endif
}
 
 
void projectBrowser::reset_CB(gtPushButton*, gtEventPtr, void* pb, gtReason)
//
// Callback for "Reset" button.
//
{
#ifndef NEW_UI
     ((projectBrowser*)pb)->reset();
#endif
}
 
void projectBrowser::closure_CB(gtToggleButton* but, gtEvent*, void*, gtReason) {
   Initialize(projectBrowser::closure_CB);
#ifndef NEW_UI
   if (but) {
      set_closure_state(but->set());
   }
#endif
}
 
void projectBrowser::reset()
//
// Clear all group and query selections.
//
{
     Initialize(projectBrowser::reset);
#ifndef NEW_UI 
     delete_selection();

     group_list->deselect_all();
 
     RTLPtr result_rtl = result_list->RTL();
     if(result_rtl)
     {
         checked_cast(RTLNode, result_rtl->get_root())->clear();
         result_list->delete_all_items();
         result_list->deselect_all();
	 symbolArr dummy_arr;
	 result_list->set_rtl(dummy_arr);
	 obj_delete(result_rtl);
     }
 
     show_list->deselect_all();
 
     RTLPtr domain_rtl = domain_list->RTL();
     if(domain_rtl)
     {
         checked_cast(RTLNode, domain_rtl->get_root())->clear();
         domain_list->delete_all_items();
         domain_list->deselect_all();
	 symbolArr dummy_arr;
	 domain_list->set_rtl(dummy_arr);
	 obj_delete(domain_rtl);
     }
     group_list->deselect_all();
#endif

     update_ask();
}

static gtStringEditor *nm_text;
static void set_temporary_group_name(int num)
{
#ifndef NEW_UI
   if (nm_text) {
      genString name;
      name.printf("RESULT_%d", num);
      nm_text->text((char *)name);
      nm_text->set_selection(0, name.length());
   }
#endif
}

static int group_counter = 1;

//
// Update the dialog when a new group is created:
//
static void subsys_gui_refresh(int add, const char *name)
{
  if (dlg) {
    if (add)
      projectBrowser::add_group_name(name, 0);
    set_temporary_group_name(group_counter);
  }
}

//dialog box for New Group
static gtList *groups_rtl = NULL;
extern "C" void popup_Print(void *cv);
extern "C" int Prompt_oneshot(struct _WidgetRec **, const char *, const char *, char **);



 // Origin of the list (groups_rtl)

void projectBrowser::capture_CB(
     gtPushButton*, gtEventPtr, void* pb_data, gtReason)
//
// Callback for "New Group" button.
// Creates a new GROUP using the current selection.
//
{
     Initialize(projectBrowser::capture_CB);

#ifndef NEW_UI
	 selected.removeAll();
	 browserShell::get_all_browser_selections(selected, true);
	 if(selected.size())
	     SetsUI::CaptureAndInvoke(NULL, selected);
	 else 
	     msg("Nothing selected for capture.", error_sev) << eom; 
#endif
}

 
void projectBrowser::init_search_path()
{
    Initialize(projectBrowser::init_search_path);

    projList::domain_reset();

    char* pathlist = gt_string_ed->text();
    projList::search_list->parse_names_init(pathlist);
#ifndef NEW_UI
    gtFree(pathlist);
#endif

//    const char* proj_name;
//    while(proj_name = parse_names_next())
//	projList::domain_add(proj_name);
    projList::search_list->domain_string();
}

/*unsigned int*/
boolean find_action(int & action, int & ind, const char *, actionSet *, int);

int cli_eval_query(const char* cmd, symbolArr& scope, symbolArr&);

void projectBrowser::group_select(
    gtList* list, gtEventPtr, void* pb_data, gtReason)
//
// Callback for "Group" list.
// Changes the contents of the "Domain" list.
//
{
    Initialize(projectBrowser::group_select);
#ifndef NEW_UI
    projectBrowser* pb = (projectBrowser*)pb_data;

    if (!ParaCancel::canceller) new ParaCancel(TXT("Cancel"), 0, 0,
		   (int)XtWindow(pb->reset_button->rep()->widget()));

    int* indices, size;
    list->get_selected_pos(&indices, &size);

    if(indices)
        gtFree(indices);

    push_busy_cursor();

    pb->delete_selection();

    char** categories = pb->group_list->selected();
    int num_cat = pb->group_list->num_selected();
    symbolArr scope;
    const objArr& cur_scope = pb->get_scope()->domain;
    scope = cur_scope;
        
    int i;

    genArrCharPtr cmd_arr;
    for (i = 0; i < num_cat; i++)
      {
	const char* cmd = pb->pBController->get_category_command(categories[i]);
	if (cmd && strlen(cmd))
	  cmd_arr.append((char**)&cmd);
      }

    size = cmd_arr.size();
    genString* cmds = new genString[size];
    genString defines;
    ddSelector defs_dd;

    pb->pBController->process_commands(cmd_arr, cmds, defines, defs_dd);

    pb->init_search_path();
    browserRTLPtr rtl_head = new browserRTL(pb->domain_list, defs_dd, pb->get_scope());

    symbolArr& results = RTLNodePtr(rtl_head->get_root())->rtl_contents();
    symbolArr temp;

    for (i = 0; i < size; ++i)
      {
	if (ParaCancel::is_cancelled()) 
	  break;
	
	genString cmd = cmds[i];

	if (cmd.is_null())
	  break;

	if (cli_eval_query(cmd, scope, temp) == TCL_OK)
	  {
	    results.insert_last(temp);
	  }
	else
	  {
	    genString msg;
	    msg.printf("Error executing %s", cmd);
	    browserShell::display_assist_line(msg, 1);
	  } 
      }

    results.remove_dup_syms();

    //need this to protect against possible deletions of not xref symbols
    symbolPtr sym;
    ForEachS(sym, results)
      {
	if (sym.relationalp())
	  put_relation(app_of_rtl, rtl_head, RelationalPtr(sym));
      }

    RTLPtr old_rtl_head = pb->domain_list->RTL();
    pb->domain_list->set_rtl(rtl_head);

    if(old_rtl_head)
        obj_delete(old_rtl_head);

    delete [] cmds;

    if (categories)
      gtFree(categories);

    pb->update_ask();

    pop_cursor();

#endif
}

void projectBrowser::show_select(
    gtList* list, gtEventPtr, void* pb_data, gtReason)
//
// Callback for "Query" list.
// Changes contents of "Results" list.
//
{
    Initialize(projectBrowser::show_select);
    projectBrowser* pb = (projectBrowser*)pb_data;

    char** selections = list->selected();
    int    size       = list->num_selected();
    
#ifndef NEW_UI
    if(selections)
        gtFree(selections);
#endif

    pb->update_links();

}

extern int add_one_level_syms(linkType lt, ddSelector sel_src, ddSelector sel_trg, symbolArr & syms);

extern int get_modified_objects(symbolArr &, symbolArr & );
extern void cli_defines_or_uses(symbolArr&src_arr, symbolArr&result_arr, ddSelector&sel, int def_flag);

void projectBrowser::update_links()
// Reconstruct the
// "Results" list as the union of the results of all sub-queries.
//
{
    Initialize(projectBrowser::update_links);
#ifndef NEW_UI

    symbolArr selection = selected_symbols;
    selection.insert_last(selected_projects);

    if (selection.size() == 0)
      {
	browserShell::display_assist_line("Nothing selected", 1);
	return;
      }

    push_busy_cursor();

    if (!ParaCancel::canceller) new ParaCancel(TXT("Cancel"), 0, 0,
					       (int)XtWindow(reset_button->rep()->widget()));

    char** queries = show_list->selected();
    int num_selected = show_list->num_selected();
    
    int i;
    genArrCharPtr cmd_arr;
    for (i = 0; i < num_selected; i++)
      {
	const char* cmd = pBController->get_query_command(queries[i]);
	if (cmd && strlen(cmd))
	  cmd_arr.append((char**)&cmd);
      }

    int size = cmd_arr.size();
    genString* cmds = new genString[size];
    genString defs;
    ddSelector defs_dd;
    
    pBController->process_commands(cmd_arr, cmds, defs, defs_dd);
   
    symbolArr results;
    genString cmd;
    symbolArr temp;
    if (defs.not_null())
      {
	pBController->handle_closure(defs, cmd);
	if (cli_eval_query(cmd, selection, temp) == TCL_OK)
	  {
	    results.insert_last(temp);
	  }
	else
	  {
	    genString msg;
	    msg.printf("Error executing %s", defs);
	    browserShell::display_assist_line(msg, 1);
	  }
      }

    for (i = 0; i < size; ++i)
      {
	if (ParaCancel::is_cancelled()) 
	  break;	

	if (cmds[i].is_null())
	  break;

	genString cmd;
	pBController->handle_closure(cmds[i], cmd);

	if (cli_eval_query(cmd, selection, temp) == TCL_OK)
	  {
	    results.insert_last(temp);
	  }
	else
	  {
	    genString msg;
	    msg.printf("Error executing %s", cmd);
	    browserShell::display_assist_line(msg, 1);
	  } 
      }

    results.remove_dup_syms();
    
    insert_array_in_results(results);

    delete [] cmds;

    gtFree(queries);

    pop_cursor();

#endif
}

objArr* browser_get_scope(projectBrowser*pb)
{
  symbolScope*sc = pb->get_scope();
  return &(sc->domain);
}

void browser_set_rtl(gtRTL* result_list, symbolArr& array)
{
    Initialize(project_set_rtl);

    result_list->deselect_all();
    RTLPtr rtl_head = result_list->RTL();
    if(rtl_head == NULL)
	rtl_head = new browserRTL(result_list);
    else
    {
	symbolArr dummy_arr;
	result_list->set_rtl(dummy_arr);
    }

    RTLNodePtr rtl_root = checked_cast (RTLNode, rtl_head->get_root());
    rtl_root->clear();
    rtl_root->rtl_contents() = array;
    result_list->set_rtl(rtl_head);

    //need this to protect against possible deletions of not xref symbols
    symbolPtr sym;
    ForEachS(sym, array)
      {
	if (sym.relationalp())
	  put_relation(app_of_rtl, rtl_head, RelationalPtr(sym));
      }
}

void projectBrowser::insert_array_in_results(symbolArr& array)
{
   browser_set_rtl(result_list, array);
}
void projectBrowser::insert_array_in_domain(symbolArr& array)
{
   browser_set_rtl(domain_list, array);
}



  // List of groups in ParaSET Groups
static projectBrowser *origin_browser = NULL;

// Check if the browser is in scan mode and switch to that if not & user wants to 
// Busy cursor assumed on entry 
static int origin_browser_in_scan() {
  Initialize(origin_browser_in_scan);

  if (!strcmp(origin_browser->mode(), "scan")) return true;
  pop_cursor();
  int ans = (dis_question3(CO_GROUPSSWITCH, T_NULL,  B_YES, NULL, B_NO, Q_SWITCHTOSCAN) == 1);
  if (ans) {
    push_busy_cursor();
    origin_browser->scan_mode();
  }
  return ans;
}

static void display_results(symbolSet& results, gtRTL* alist) {
   Initialize(display_results);
#ifndef NEW_UI
   origin_browser->reset();
   RTLPtr old_rtl = alist->RTL();
   RTLPtr new_rtl = new RTL((old_rtl) ? old_rtl->get_name() : NULL);
   RTLNodePtr new_rtl_node = checked_cast(RTLNode, new_rtl->get_root());
   symbolPtr res;
   ForEachT(res, results) {
      new_rtl_node->rtl_insert(res, false);	// don't check, guaranteed unique
   }
   alist->set_rtl(new_rtl);
   if (old_rtl) obj_delete(old_rtl);
#endif
}


void get_link_add_results(symbolPtr sym, linkType lt, symbolSet& results) {
   Initialize(get_link_add_results);

   symbolArr res_arr;
   if (sym->get_link(lt, res_arr)) {
      results.insert(res_arr);
   }
}



static void get_groups_add_results(symbolPtr sym, symbolSet& results) {
   Initialize(get_groups_add_results);

   symbolArr res_arr;
   sym->get_link(grp_pub_mbr_of, res_arr);
   sym->get_link(grp_pri_mbr_of, res_arr);
   results.insert(res_arr);
}


//  Callback for the complement of group(s)
void projectBrowser::complement_groups_CB(gtPushButton*, gtEventPtr, void* pb_data, gtReason)
{
    Initialize(projectBrowser::complement_groups_CB);
#ifndef NEW_UI
    projectBrowser* pb = (projectBrowser*)pb_data;
    push_busy_cursor();
    if (pb) {
	int* indices, size, group_count, i=0;
	groups_rtl->get_selected_pos(&indices, &size);
	group_count = origin_browser->group_lists.size();

	RTLPtr list;
	symbolArr union_array, other_array;
	int *groups_seen = new int[group_count];

	for(i = 0; i < group_count; i++)
	    groups_seen[i] = 0;
	
	for(i = 0; i < size && i < group_count; i++) {
	    const int index = indices[i] - 1;
	    list = *(origin_browser->group_lists)[index];
	    symbolArr& array = RTLNodePtr(list->get_root())->rtl_contents();
	    union_array.insert_last(array);
	    groups_seen[index] = 1;
	}

	for (i = 0; i < group_count; i++) 
	    if (!groups_seen[i]) {
		const int index = i;
		list = *(origin_browser->group_lists)[index];
		symbolArr& array = RTLNodePtr(list->get_root())->rtl_contents();
		other_array.insert_last(array);
	    }

	delete groups_seen;

	union_array.usort();
	other_array.usort();
	symbolPtr sym;
	ForEachS(sym, union_array) {
	    if (other_array.includes_cross_xref(sym))
		other_array.remove_cross_xref(sym);
	}

	origin_browser->reset();
	origin_browser->domain_list->set_rtl(other_array);
	if(indices)
	    gtFree(indices);
    }
    pop_cursor();
#endif
}
// -------------------------------


// callback to display the selected group and display it in the domain_list
void projectBrowser::group_action_CB(gtList*, gtEventPtr, void *pb_data, gtReason)
{
    Initialize(projectBrowser::group_action_CB);
#ifndef NEW_UI
    projectBrowser *pb = (projectBrowser *)pb_data;
    if (pb) {
	int* indices, size;
	groups_rtl->get_selected_pos(&indices, &size);
	if (size > 0) {
	    origin_browser->reset();
	    origin_browser->group_list->select_pos(pb->group_items.size()+indices[0], 1);
	    set_temporary_group_name(group_counter);
	}
    }
#endif
}

// callback to handle double-click on a group and open groupManager
void projectBrowser::all_groups_action_CB(gtList*, gtEventPtr, void* pb_data, gtReason)
{
    Initialize(projectBrowser::all_groups_action_CB);
#ifndef NEW_UI
    projectBrowser *pb = (projectBrowser *)pb_data;
    if (pb) {
	int* indices, size, pos;
	pb->group_list->get_selected_pos(&indices, &size);

	if (size > 0) {	    
	    if ((pos = indices[0] - pb->group_items.size()) > 0) {
		if (dlg)
		    pb->attach_groupManager(pb);
		else {
#if 0
		    pb->create_new_group_dlg();
#endif
		    dlg->popup();
		}
		groups_rtl->select_pos(pos, 1);
	    }
	}
    }
#endif
}

void group_mgr_closure_CB(gtToggleButton* but, gtEvent*, void*, gtReason) {
   Initialize(group_mgr_closure_CB);
#ifndef NEW_UI
   if (but) {
      set_closure_state(but->set());
   }
#endif
}

static int groupManager_destroy_callback(void *)
{
#ifndef NEW_UI
    dlg->popdown();
    dlg = NULL;
    group_mgr_closure_button = NULL;
    origin_browser = NULL;
    groups_rtl = NULL;
#endif
    return 1;
}

// dynamic
int projectBrowser::exists(char *name)
{
    Initialize(projectBrowser::exists);
#ifndef NEW_UI
    if (!groups_rtl)
        return false;
    int  count   = groups_rtl->num_items();
    char **items = groups_rtl->all_items();
    for (int i=0; i<count; i++)
    {
        if (strcmp(name,items[i]+1) == 0)
            return true;
    }
    gtFree(items);
    return false;
#else
    return 0;
#endif
}

// called by browserShell destructor
// re-attach Group Manager when browser gets closed
void projectBrowser::reset_group_dialog(projectBrowser *pb)
{
#ifndef NEW_UI
    if ((!pb || is_groupParent(this)) && dlg) {
	dlg->popdown();
	set_groupParent(NULL);
	dlg = NULL;
	group_mgr_closure_button = NULL;
	origin_browser = NULL;
	groups_rtl = NULL;
    } else
	pb->attach_groupManager(pb);
#endif
}

//  attach_groupManager() : routine to switch the browsers
void projectBrowser::attach_groupManager(projectBrowser *pb)
{
    Initialize(projectBrowser::attach_groupManager);
#ifndef NEW_UI
    if (pb && dlg && origin_browser != pb) {
	origin_browser = pb;

	int manager_num = 0;
	const char *layout = (char *)pb->bsh.get_layout_name();
	if (layout) {
	    manager_num = OSapi_atoi(layout);
	}
#if ALLOW_MULTIPLE_SUBSYSTEM_MANAGERS
	groupHandle handle;
	handle.a_project_browser = pb;
	handle.group_list_index = -1;
	bsh.toAllBrowsers(&projectBrowser::getGroupTitleNumber, &handle);

	handle.group_list_index = 0;
	int manager_num = getGroupTitleNumber(&handle);
	genString name;
	name.printf(TXT("DISCOVER Subsystems: %d"), manager_num);
	dlg->title((char *)name);
#endif    // ALLOW_MULTIPLE_SUBSYSTEM_MANAGERS

	groups_rtl->delete_all_items();
	projNodePtr pr = projNode::get_home_proj();
	Xref* xr = (pr) ? pr->get_xref() : NULL;
	if (xr) {
	   symbolArr groups;
	   ddSelector sel;
	   sel.add(DD_SUBSYSTEM);
	   xr->add_symbols(groups, sel);
	   symbolPtr sym;
	   ForEachS(sym, groups) {
	      if (sym.xrisnotnull() && sym.get_has_def_file()) {
		 add_group_name(sym.get_name(), sym->get_attribute(IS_SUBSYS_ATT, 1));
	      }
	   }
	}
	if (global_groups_button) {
	   global_groups_button->set(0, 0);
	}
	set_temporary_group_name(group_counter);
    }
#endif
}


// ---------------- GLOBAL (propagate change to all browsers) ----------------
// ---------------------------------------------------------------------------
// called by browserShell.toAllBrowsers( _ , handle )
// dynamic (global mechanism)
int projectBrowser::collectSelected(groupHandle *handle)
{
    int ret_val = 1;

    if (handle) {
	symbolArr &array = selectedSymbols();
	symbolArr *target = handle->sym_array;
	target->insert_last(array);
    } else
	ret_val = 0;

    return ret_val;
}

// dynamic (global mechanism)
int projectBrowser::selectGroup(groupHandle *handle)
{
    int ret_val = 1;
#ifndef NEW_UI
    if (handle) {
	reset();
	if (handle->selected_count > 0) {
	    for (int i = 0; i < handle->selected_count; i++)
		group_list->select_pos(handle->selected_indices[i], 1);
	} else
	    group_list->select_pos(handle->group_list_index, 1);
    } else
	ret_val = 0;
#endif
    return ret_val;
}

// dynamic (global mechanism)
int projectBrowser::getGroupTitleNumber(groupHandle *handle)
{
    static int title_number = 1;
    int ret_val = 0;

    if (handle) {
	if (handle->group_list_index < 0) {
	    if (handle->a_project_browser != this) {
		title_number++;
		ret_val = 1;
	    }
	} else {
	    ret_val = title_number;
	    title_number = 1;
	}
    }

    return ret_val;
}

// dynamic (global mechanism)
int projectBrowser::getGroups(groupHandle *handle)
{
    int ret_val = 0;
#ifndef NEW_UI
    if (handle) {
	projectBrowser *pb = handle->a_project_browser;
	if (pb && this != pb) {
	    int size = group_lists.size();
	    for (int i = 0; i < size; i++) {
		RTLPtr list = *(group_lists[i]);
		char *nm = list->get_name();
		pb->group_lists.append(&list);
		pb->group_list->add_item(nm, 0);
	    }
	} else
	    ret_val = 1;
    }
#endif
    return ret_val;
}

// dynamic (global mechanism)
int projectBrowser::addGroup(groupHandle *handle)
{
    int ret_val = 1;
#ifndef NEW_UI
    if (handle) {
	group_lists.append(&(handle->a_group));
	group_list->add_item(handle->a_group_name, 0);

	sel_rtl = (browserRTL *)handle->a_group;
    } else
	ret_val = 0;
#endif
    return ret_val;
}

// dynamic (global mechanism)
int projectBrowser::renameGroup(groupHandle *handle)
{
    int ret_val = 1;
#ifndef NEW_UI
    if (handle) {
	int index = handle->group_list_index;
	group_list->delete_pos(index);
	group_list->add_item(handle->a_group_name, index);
    } else
	ret_val = 0;
#endif
    return ret_val;
}

// dynamic (global mechanism)
int projectBrowser::deleteGroup(groupHandle *handle)
{
    int ret_val = 1;
#ifndef NEW_UI
    if (handle) {
	int *indices = handle->selected_indices;
	for (int i = handle->selected_count-1; i >= 0; i--) {
	    const int index = indices[i];
	    group_lists.remove(index-1);
	    group_list->delete_pos(group_items.size()+index);
	}
	reset();
    } else
	ret_val = 0;
#endif
    return ret_val;
}
// ------end-of-GLOBAL-----------------

void projectBrowser::add_group_name(const char *name, bool)
{
    Initialize(projectBrowser::add_group_name);
#ifndef NEW_UI
    if (groups_rtl)
    {
        int count = groups_rtl->num_items();
        char** items = groups_rtl->all_items();
        size_t i;
        int items_free = 1;
        for (i = 0; i < count; i++)
        {
            int status = strcmp(name, items[i]+1);
            if (status == 0)
            {
                groups_rtl->delete_pos(i+1);
                break;
            }
            else if (status < 0)
                break;
        }
        if (items_free)
            gtFree(items);

        genString new_name;
        extern int is_aseticons_available();
        if (is_aseticons_available())
            new_name.printf("%c%s",PIX_XSUBSYSTEM,name);
        else
            new_name.printf("%s", name);
        groups_rtl->add_item(new_name, i+1);
    }
#endif
}

// -- IMPORTING UTILITIES: (.rtl files) -- begin

void view_refresh_window(viewGraHeaderPtr v);


extern "C" void fill_selected_objects (symbolArr *selection);

//  ----------------------------------------
//  returns selected symbols from a browser
//  ---------------------------------------
symbolArr& projectBrowser::selectedSymbols()
{
    Initialize(projectBrowser::selectedSymbols);

    symbolArr &sel = selected_symbols;
 
    int no_sel = sel.size();
//  replace ddElements in selected array by their xrefSymbols
    for (int i = 0; i < no_sel; i++ )
	sel[i] = sel[i].get_xrefSymbol();
 
    symbolArr  selection;
    fill_selected_objects (&selection);
 
    symbolPtr el;
    ForEachS(el, selection){
        symbolPtr  xref = el.get_xrefSymbol();
	
        if(! xref.isnull())
	    sel.insert_last(xref);
    }
  
    if(no_sel==0) do {
      viewerShell * vs = viewerShell::get_current_vs(1);
      if(!vs) break;
      view* vvv = vs->get_current_view();
      if(!vvv) break;
      el = (Obj *)vvv->get_appHeader();
      if(el.isnull()) break;
      el = el.get_xrefSymbol();
      if(el.isnull()) break;
      sel.insert_last(el);
    } while(0);

    sel.usort();

    return sel;
}

void projectBrowser::getSelectedSymbols(symbolArr &symbols)
{
    symbols.removeAll();
    groupHandle handle;
    handle.sym_array = &symbols;
    bsh.toAllBrowsers(&projectBrowser::collectSelected, &handle);
    symbols.remove_dup_syms();
}

//  ---------------------------
//  Callbacks for Group Manager
//  ---------------------------
// cancel callback
void projectBrowser::quit_groupManager_CB(gtPushButton*, gtEvent*, void* cd, gtReason)
{
    Initialize(projectBrowser::quit_group_manager_CB);
#ifndef NEW_UI
    projectBrowser *pb = (projectBrowser *)cd;
    if (pb) {
	dlg->popdown();
	dlg = NULL;
	group_mgr_closure_button = NULL;
	origin_browser = NULL;
	groups_rtl = NULL;
    }
#endif
}


void groupManager_unmap ()
{
#ifndef NEW_UI
    if (dlg)
      dlg->popdown();
#endif
}

void update_rem_group_manager(const char *name)
{
    Initialize(update_rem_group_manager);
#ifndef NEW_UI
    if (!groups_rtl)
        return;
    int  count   = groups_rtl->num_items();
    char **items = groups_rtl->all_items();
    for (int i=0; i<count; i++)
    {
        if (strcmp(name,items[i]+1) == 0)
	{
            groups_rtl->delete_pos(i+1);
            break;
        }
    }
    gtFree(items);
#endif
}

static void process_cm_report(FILE* cm_report_file,
			      symbolSet& mod_entities) {
   Initialize(process_cm_report);

   projNodePtr proj = projNode::get_home_proj();
   Xref* xr = (proj) ? proj->get_xref() : NULL;
   if (xr) {
      char buff[1024];
      genString filename;
      while (OSapi_fgets(buff, 1024, cm_report_file)) {
	 char* colon = strchr(buff, ':');
	 char* nl = strchr(buff, '\n');
	 if (nl) {
	    *nl = 0;	// strip \n
	 }
	 if (*buff == '%') {	// new file
	    // % MODULE : /logical/file/name.C
	    if (colon) {
	       filename = colon + 2;
	    }
	 }
	 else if (*buff == '@') {	// modified entity
	    // @ FUNCTION : foo(int)
	    if (colon) {
	       *(colon - 1) = 0;
	       ddKind k = ddKind_of_readable_name(buff + 2);
	       switch (k) {
	       case DD_FUNC_DECL:
	       case DD_CLASS:
	       case DD_ENUM:
	       case DD_MACRO:
	       case DD_VAR_DECL:
	       case DD_PARAM_DECL:
	       case DD_FIELD:
	       case DD_TEMPLATE:
	       case DD_TYPEDEF:
	       case DD_UNION: {
		  symbolArr trash;
		  symbolPtr sym = xr->find_symbol(trash, k, colon + 2,
						  filename);
		  if (sym.xrisnotnull()) {
		     mod_entities.insert(sym);
		  }
		  break;
	       }
               default:
		  break;
	       }
	    }
	 }
      }
   }
   else msg("Internal error: cannot find model for home project", error_sev) << eom;
}      


const int FILE_READ_BUF_SIZE = 512;

int file_cat( FILE *file1, FILE *file2 )
{
  int nRet = 0;
  char *buff = new char[FILE_READ_BUF_SIZE];
  if( buff )
  {
    long lPos2 = OSapi_ftell( file2 );
    OSapi_fseek( file2, 0, SEEK_SET );
    OSapi_fseek( file1, 0, SEEK_END );
    size_t nRead = 0;
    while ( nRead = OSapi_fread( (void *)buff, sizeof(char), 
          FILE_READ_BUF_SIZE, file2 ) )
      nRet += OSapi_fwrite( (void *)buff, sizeof(char), nRead, file1 );
    //Restore file pointer position for a source file
    OSapi_fseek( file2, lPos2, SEEK_SET );
    delete buff;
  }
  return nRet;
}

int get_arr_index(const genArrCharPtr& arr, const char* str);

void projectBrowser::update_ask()
// Reconstruct the "Ask" list given selected symbols
{
    Initialize(projectBrowser::update_ask);
     
#ifndef NEW_UI

    char** old_selection = show_list->selected();
    int num_selected = show_list->num_selected();

    symbolArr selection = selected_symbols;
    selection.insert_last(selected_projects);
    
    show_items.reset();
    genMask dd_mask;
    if (selection.size() >0 )
      pBController->get_queries(selection, show_items);
    else //selection is empty, get selection from Categories list
      {
	int num_selected = group_list->num_selected();
	if (num_selected > 0)
	  {
	    char** selected = group_list->selected();
	    for (int i = 0; i < num_selected; i++)
	      {
		int kind = pBController->get_category_kind(selected[i]);
		if (kind >= 0)
		  dd_mask += kind;
	      }
	    gtFree(selected);
	    pBController->get_queries(dd_mask, show_items);
	  }
	if (show_items.size() == 0)
	  pBController->get_all_queries(show_items);
      }

    fill_list(show_list, show_items);
				
    //restore old selection
    show_list->sel_type(gtMultiple);
    for (int i = 0; i < num_selected; i++)
    {
	if (get_arr_index(show_items, old_selection[i]) >= 0)
	{
	    show_list->select_item(old_selection[i], 0);
	}
    }
    show_list->sel_type(gtExtended);

    //show_select(show_list, NULL, this, gtReason(0));

    if (old_selection)
	gtFree(old_selection);
#endif
}

void projectBrowser::update_categories()
{
    pBController->get_categories(group_items);
    fill_list(group_list, group_items);
}

void projectBrowser::fill_list(gtList* list, const genArrCharPtr& items)
{
    list->deselect_all();
    list->delete_all_items();

    const char** temp = new const char* [items.size()];
 
    for (int i = 0; i < items.size(); i++)
    {	
	temp[i] = *items[i];
    }
    list->add_items(temp, items.size(), -1);

    delete [] temp;
}

void projectBrowser::rtl_action(gtList* l, gtEventPtr e, void* pb, gtReason r)
{
  Initialize(projectBrowser::rtl_action);

  projectBrowser* browser = (projectBrowser*)pb;
  symbolArr selection = browser->selected_symbols;
  selection.insert_last(browser->selected_projects);  

  BrowserData* bd = LanguageController::GetBrowserData();

  for (int i = 0; i < selection.size(); i++)
    {
      const char* cmd = bd->get_action_cmd(selection[i].get_kind());
      if (cmd == NULL)
	{
	  baseBrowser::list_action(l, e, pb, r);
          break;
	}
      else
	{
	  Interpreter* intr = GetActiveInterpreter();
	  if (strlen(cmd) > 0 && intr)
	    { 
	      symbolArr sel, dummy;
	      sel.insert_last(selection[i]);
	      intr->EvalQuery((char*)cmd, sel, dummy);
	    } 
	}
    }
}
