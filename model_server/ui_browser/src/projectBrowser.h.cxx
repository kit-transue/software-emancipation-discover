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
// projectBrowser.h.C
//------------------------------------------
// synopsis:
//
// ParaSET browser for projects,
// modules (logical filenames), and XREF entries.
//------------------------------------------

// INCLUDE FILES
#include <cLibraryFunctions.h>
#include <msg.h>
#include <messages.h>

#include <genError.h>

#include <objOper.h>
#include <RTL.h>
#include <RTL_externs.h>
#include <proj.h>
#include <ddict.h>

#include <gtForm.h>
#include <gtRadioBox.h>
#include <gtTogB.h>
#include <gtPushButton.h>
#include <gtStringEd.h>
#include <gtHorzBox.h>
#include <gtRTL.h>

#include <browserShell.h>
#include <top_widgets.h>
#include <view_creation.h>
#include <driver.h>
#include <path.h>
#include <dd_utils.h>

#include <scopeMgr.h>
#include <genStringPlus.h>

#include <BrowserController.h>
#include <cliUI.h>

// VARIABLE DEFINITIONS

static const char PROJECT_SEPARATOR[] = "/";

// FUNCTION DEFINITIONS

projectBrowser::projectBrowser(browserShell* b, RTLPtr p)
//
// Constructor.
//
: baseBrowser(b, TXT("Project")),
  gt_form_top(NULL),
  up_button(NULL),
  reset_button(NULL),
  closure_button(NULL),
  capture_button(NULL),
  browse_scan_radiobox(NULL),
  gt_form_hierarchy(NULL),
  hierarchy_list(NULL),
  top_proj(p),
  cur_proj(NULL),
  gt_form_xref(NULL),
  group_list(NULL),
  domain_list(NULL),
  show_list(NULL),
  result_list(NULL),
  pBController(NULL)
{
    Initialize(projectBrowser::projectBrowser);

    build_interface(0);
    browse_mode();
    browse(top_proj);
}

projectBrowser::~projectBrowser()
//
// Destructor.
//
{
    Initialize(projectBrowser::~projectBrowser);

    RTLPtr domain_rtl = domain_list->RTL();
    if(domain_rtl)
	obj_unload(domain_rtl);

    RTLPtr result_rtl = result_list->RTL();
    if(result_rtl)
	obj_unload(result_rtl);
    
    delete_selection(NULL);
    
//    if (group_list)
//	psetfree(group_list);
//    if (show_list)
//	psetfree(show_list);
}


void projectBrowser::browse_mode()
//
// Force projectBrowser into "browse" (hierarchical projects & modules) mode.
//
{
    Initialize(projectBrowser::browse_mode);
#ifndef NEW_UI
    browse_scan_radiobox->item_set(0, 1, 1);
    if (cur_proj && is_projHeader(cur_proj))
	up_button->set_sensitive(cur_proj != top_proj);
    else
	up_button->set_sensitive(0);
    closure_button->set_sensitive(0);
#endif
}


void projectBrowser::scan_mode()
//
// Force projectBrowser into "scan" (XREF query) mode.
//
{
    Initialize(projectBrowser::scan_mode);
#ifndef NEW_UI

    if(cur_proj && !is_projHeader(cur_proj))
    { 
      browse(top_proj);                         // in loaded files
      browse_scan_radiobox->set_sensitive(1);
    }

    browse_scan_radiobox->item_set(1, 1, 1);
    up_button->set_sensitive(0);
    closure_button->set_sensitive(1);

#endif
}

void projectBrowser::closure(bool enabled) {
   Initialize(projectBrowser::closure);
#ifndef NEW_UI
   closure_button->set(enabled, 0);
#endif
}

const char* projectBrowser::mode()
//
// Return a string describing the current mode.
//
{
    Initialize(projectBrowser::mode);
#ifndef NEW_UI
    return gt_form_hierarchy->is_mapped() ? "browse" : "scan";
#else
    return NULL;
#endif
}


const char* projectBrowser::project_pathname(genString& pathname)
//
// Return the full pathname of the current project, or "/".
//
{
    Initialize(projectBrowser::project_pathname);

    if(cur_proj  &&  is_projHeader(cur_proj))
    {
	projNodePtr proj_root = projNodePtr(cur_proj->get_root());
	proj_root->refresh();
	pathname = proj_root->get_header()->get_name();
    }
    else
	pathname = PROJECT_SEPARATOR;

    return pathname;
}


const char* projectBrowser::selected_groups(genString& groups)
//
// Set groups equal to a textual representation of the selected groups and
// return the string.
//
{
    Initialize(projectBrowser::selected_groups);
#ifndef NEW_UI
    ostrstream ostr;
    int not_first = 0;

    int* indices, size;
    group_list->get_selected_pos(&indices, &size);

    for(int i = 0; i < size; ++i)
    {
	const int index = indices[i] - 1;
	const char* name = group_name(index);
	if(name)
	{
	    if(not_first)
		ostr << ", ";
	    ostr << group_name(index);
	    not_first = 1;
	}
    }
    if(indices)
	gtFree(indices);

    ostr << ends;

    groups = ostr.str();
    ostr.rdbuf()->freeze(0);

    return groups;
#else
    return NULL;
#endif
}


const char* projectBrowser::selected_queries(genString& queries)
//
// Set queries equal to a textual representation of the selected queries and
// return the string.
//
{
    Initialize(projectBrowser::selected_queries);

    ostrstream ostr;
    int not_first = 0;

    char** selection = show_list->selected();
    int num_selected = show_list->num_selected();

    for (int i = 0; i < num_selected; i++) {
	if (not_first)
	    ostr << ", ";
	ostr << selection[i];
	not_first = 1;
    }

    ostr << ends;

    queries = ostr.str();
    ostr.rdbuf()->freeze(0);

    if (selection)
      gtFree(selection);

    return queries;
}


void projectBrowser::browse(RTLPtr rtl_head)
//
// Set the project/XREF currently being browsed and used as a domain.
//
{
    Initialize(projectBrowser::browse);
#ifndef NEW_UI
#define NM(x) (x ? x->get_name() : "<>")
     int is_scan = gt_form_xref->is_mapped();
     int is_hier = gt_form_hierarchy->is_mapped();

    if(cur_proj && (rtl_head != top_proj) && (!is_hier) && is_scan) {  // scan mode
        scopeNode* scnode  = NULL;
	if(is_scopeNode(rtl_head)){
	  scnode = (scopeNode*) rtl_head;
	  rtl_head = 0;
	}
        appTree*nnn= NULL;
        if(rtl_head){
	  RTLNode*node = RTLNodePtr(rtl_head->get_root());
	  if(is_projNode(node)){
	    nnn = node;
	    projNodePtr(node)->refresh();
	  }

	  symbolArr array;
	  scopeMgr_get_proj_contents ((projNode *)node,array);

	  if(domain_selection.size())
	    insert_array_in_results(array);
	  else
	    insert_array_in_domain(array);
	} else {
	  symbolArr array;
	  nnn = scnode;
	  scnode->get_contents(array);
	  if(domain_selection.size())
	    insert_array_in_results(array);
	  else
	    insert_array_in_domain(array);
	}
	if(nnn){
	  gt_string_ed->text(nnn->get_name());
	  objArr subproj_Arr;
	  subproj_Arr.insert_last(nnn);
	  scope.set_domain(subproj_Arr);
	}
    } else if(cur_proj != rtl_head) {
	push_busy_cursor();

	RTLPtr last_proj = cur_proj;
	cur_proj = rtl_head;

	genString pathname;

	if(is_projHeader(cur_proj))
	{
	    projNodePtr proj_root = projNodePtr(cur_proj->get_root());
	    proj_root->refresh();
            pathname = proj_root->get_header()->get_name();

	    projNodePtr last_proj_root = NULL;
	    if(last_proj && is_projHeader(last_proj))
		last_proj_root = projNodePtr(last_proj->get_root());
	    if(!last_proj_root ||
	       (proj_root->get_xref() != last_proj_root->get_xref()))
		reset();
	}
	else if(rtl_head == top_proj)
	{
	    pathname = PROJECT_SEPARATOR;
	    reset();
	}
	else
	{
	    pathname = "<>";
	    reset();
	}

	deselect();

        gt_string_ed->text(pathname);
	up_button->set_sensitive(rtl_head != top_proj);
	hierarchy_list->set_rtl(cur_proj);

	pop_cursor();
    }
#else
    OSapi_fprintf (stderr, "Error: projectBrowser::browse called but not implemented\n");
#endif
}


void projectBrowser::force_up(int up_level)
{
    Initialize(projectBrowser::force_up);

    projectBrowser* pb = this;

    for(int i=0;i<up_level;i++)
    {
        RTLPtr new_proj = NULL;

        if(pb->cur_proj && is_projHeader(pb->cur_proj))
        {
            projNodePtr proj_root =
                checked_cast(projNode, pb->cur_proj->get_root());
            projNodePtr proj_parent = proj_root->find_parent();
            if(proj_parent)
                new_proj = checked_cast(projHeader, proj_parent->get_header());
        }
        pb->browse(new_proj ? new_proj : pb->top_proj);
    }
}

void projectBrowser::up_CB(gtPushButton*, gtEventPtr, void* pb_data, gtReason)
//
// Callback to browse the parent of the current project.
//
{
    Initialize(projectBrowser::up_CB);
#ifndef NEW_UI
    projectBrowser* pb = (projectBrowser*)pb_data;

    RTLPtr new_proj = NULL;
    const char* old_proj_name = NULL;

    if(pb->cur_proj && is_projHeader(pb->cur_proj))
    {
        old_proj_name = pb->cur_proj->get_name();
	projNodePtr proj_root =
	    checked_cast(projNode, pb->cur_proj->get_root());
	projNodePtr proj_parent = proj_root->find_parent();
	if(proj_parent)
	    new_proj = checked_cast(projHeader, proj_parent->get_header());
    }
    pb->browse(new_proj ? new_proj : pb->top_proj);
    if (old_proj_name) {
       gtBase::flush_output();	// make new proj RTL available
       int num_items = pb->hierarchy_list->num_items();
       char** strs = pb->hierarchy_list->gt_list()->all_items();
       size_t name_len = strlen(old_proj_name);
       for (int i = 0; i < num_items; i++) {
	  if (strlen(strs[i]) == name_len + 1 && strstr(strs[i], old_proj_name)) {
             pb->hierarchy_list->select_pos(i + 1, true);
	     pb->hierarchy_list->gt_list()->make_pos_visible(i + 1);
          }
       }
       gtFree(strs);
    }
    pb->browse_scan_radiobox->set_sensitive(1);
    pb->gt_string_ed->set_sensitive(1);
#endif
}

void projectBrowser::obtain_all_projects(symbolArr &pn, int &is_top_proj)
{
    Initialize(projectBrowser::obtain_all_projects);
#ifndef NEW_UI
    is_top_proj = 0;
    char* text = gt_string_ed->text();
    projList::search_list->parse_names_init(text);
    const char* proj_name;
    genString not_found;
    symbolPtr sym;

    while(proj_name = projList::search_list->parse_names_next(&sym))
    {
        if (strcmp(proj_name, "/") == 0)
            is_top_proj = 1;
        else
        {
          projNode* proj_node;
          if ( !sym.isnotnull() )
             proj_node = checked_cast(projNode,((Relational *)sym));
          else
             proj_node = projNode::find_project(proj_name);
          if(proj_node)
          {
              // remove duplicate
              int found = 0;
              int sz = pn.size();
              for (int i=sz-1; i>=0; i--)
              {
                projNode *node = checked_cast(projNode, pn[i]);
                if (proj_node == node)
                {
                  found = 1;
                  break;
                }
              }
              if (!found)
                pn.insert_last(proj_node);
          }
          else
          {
            if(not_found.not_null())
                not_found += ", ";
            not_found += proj_name;
          }
        }
    }
    if(not_found.not_null())
	msg("Following project(s) not found: $1", error_sev) << not_found.str() << eom;
#endif
}

void projectBrowser::scan_mode_CB(
    gtPushButton* button, gtEventPtr, void* pb_data, gtReason)
//
// Callback to switch to "scan" mode.
//
{
    Initialize(projectBrowser::scan_mode_CB);
#ifndef NEW_UI
    projectBrowser* pb = (projectBrowser*)pb_data;
    gtToggleButton* toggle = (gtToggleButton*)button;

    if(toggle->set())
    {
	pb->gt_form_top->unmanage();
	pb->gt_form_hierarchy->set_sensitive(0);  // make browse list insensitive
	pb->gt_form_hierarchy->unmap();           // so they cannot be traversed

	pb->reset_button->set_sensitive(1);
	pb->closure_button->set_sensitive(1);
	pb->gt_form_xref->set_sensitive(1);  // set scan lists sensitive so that
	pb->gt_form_xref->map();             // they can be traversed
	pb->gt_form_top->manage();

	char* text = pb->gt_string_ed->text();
	pb->filter(text);
	gtFree(text);

	pb->init_search_path();
	pb->up_button->set_sensitive(0);
	pb->reset();
    }
#endif
}


void projectBrowser::browse_mode_CB(
    gtPushButton* button, gtEventPtr, void* pb_data, gtReason)
//
// Callback to switch to "browse" mode.
//
{
    Initialize(projectBrowser::scan_mode_CB);
#ifndef NEW_UI
    projectBrowser* pb = (projectBrowser*)pb_data;
    gtToggleButton* toggle = (gtToggleButton*)button;

    if(toggle->set())
    {
	pb->gt_form_top->unmanage();
	pb->gt_form_xref->set_sensitive(0); // so scan mode widgets do not get traversed
	pb->gt_form_xref->unmap();
	pb->reset_button->set_sensitive(0);
	pb->closure_button->set_sensitive(0);
	
	pb->gt_form_hierarchy->set_sensitive(1); // so browse widgets can be traversed
	pb->gt_form_hierarchy->map();
	pb->gt_form_top->manage();

	char* text = pb->gt_string_ed->text();
	pb->filter(text);
	gtFree(text);

	if (pb->cur_proj && is_projHeader(pb->cur_proj))
	    pb->up_button->set_sensitive(pb->cur_proj != pb->top_proj);
	else
	    pb->up_button->set_sensitive(0);
    }
#endif
}


gtPrimitive* projectBrowser::build_body(gtBase* parent)
//
// Build GT components of project hierarchy interface.
//
{
    Initialize(projectBrowser::build_body);
#ifndef NEW_UI
    gt_form_top = gtForm::create(
	parent, "project_browser");

    //
    // Create interface shared by hierarchy and xref sides.
    //

    gtHorzBox* hbox0 = gtHorzBox::create(
	gt_form_top, "project_controls");
    hbox0->margins(0, 0);
//    gtForm::vertStack(hbox0);

    browse_scan_radiobox = gtRadioBox::create(
	hbox0, "browse_scan", NULL,
	TXT("Browse"), "browse", browse_mode_CB, this,
	TXT("Scan"), "scan", scan_mode_CB, this,
	NULL);
    browse_scan_radiobox->num_columns(2);
    browse_scan_radiobox->item_set(0, 1, 0); // Set first item, no notification.

    gtHorzBox* hbox1 = gtHorzBox::create(
       hbox0, "project_buttons");
    hbox1->margins(0, 0);
    hbox1->packing(gtPackColumn);

    up_button = gtPushButton::create(
	hbox1, "up", TXT("Up"), up_CB, this);
    
    capture_button = gtPushButton::create(
	hbox1, "group", TXT("Group"), capture_CB, this);

    reset_button = gtPushButton::create(
	hbox1, "reset", TXT("Reset"), reset_CB, this);

    closure_button = gtToggleButton::create(
        hbox1, "closure", TXT("Closure"), closure_CB, this);

    up_button->set_sensitive(0);
    up_button->manage();
    reset_button->set_sensitive(0);
    reset_button->manage();
    closure_button->set(get_closure_state(), 0);
    closure_button->set_sensitive(0);
    closure_button->manage();
    capture_button->manage();    
    browse_scan_radiobox->manage();
    hbox0->manage();
    hbox1->manage();

    //
    // Create Browse Mode
    //

    gt_form_hierarchy = gtForm::create(
	gt_form_top, "hierarchy_mode");
    gt_form_hierarchy->attach(gtTop, hbox0);
    gt_form_hierarchy->attach(gtLeft);
    gt_form_hierarchy->attach(gtRight);
    gt_form_hierarchy->attach(gtBottom);

    hierarchy_list = gtRTL::create(
	gt_form_hierarchy, "proj_list", NULL, gtExtended, NULL, 0);
    hierarchy_list->action_callback(baseBrowser::list_action, this);
    hierarchy_list->select_callback(hierarchy_select, this);

    gtForm::vertStack(hierarchy_list->gt_list());
    hierarchy_list->attach(gtBottom);

    hierarchy_list->manage();
    gt_form_hierarchy->manage();

    //
    // Create Scan Mode
    //

    gt_form_xref = (gtForm*)build_xref_query(gt_form_top);
    gt_form_xref->attach(gtTop, hbox0);
    gt_form_xref->attach(gtLeft);
    gt_form_xref->attach(gtRight);
    gt_form_xref->attach(gtBottom);
    gt_form_xref->manage();

    // Initialize for Browse mode.

    gt_form_xref->set_sensitive(0); // so scan widgets do not get traversed
    gt_form_xref->unmap();

    return gt_form_top;
#else
    return NULL;
#endif
}


symbolArr& projectBrowser::project_selection()
//
// Return reference to the array of selected projects.
//
{
    Initialize(projectBrowser::project_selection);

    return selected_projects;
}


symbolArr& projectBrowser::module_selection()
//
// Return reference to the array of selected modules.
//
{
    Initialize(projectBrowser::module_selection);

    return selected_modules;
}


symbolArr& projectBrowser::symbol_selection()
//
// Return reference to the array of selected symbols (everything but projects).
//
{
    Initialize(projectBrowser::symbol_selection);

    return selected_symbols;
}

void projectBrowser::delete_selection(gtRTL* keep)
//
// Clear all selections.
//
{
    Initialize(projectBrowser::delete_selection);

    selected_symbols.removeAll();
    selected_modules.removeAll();
    selected_projects.removeAll();

    if(hierarchy_list != keep)
	hierarchy_list->deselect_all();
   if(domain_list != keep)
	domain_list->deselect_all();
    if(result_list != keep)
	result_list->deselect_all();
}


inline int shift_or_control(gtEventPtr e)
//
// Return true iff the SHIFT or CONTROL keys were pressed in the event.
//
{
#ifndef NEW_UI
    return e->xbutton.state & (ShiftMask | ControlMask);
#else
    return 0;
#endif
}


void projectBrowser::hierarchy_select(gtList*, gtEventPtr e, void* pb, gtReason)
//
// Selection callback for project hierarchy list.
//
{
#ifndef NEW_UI
    projectBrowser *browser = (projectBrowser *)pb;
    if (e){
	int extend = shift_or_control(e);
	if(!extend){
	    browser->delete_selection(browser->hierarchy_list);
	    browser->domain_selection.removeAll();
	    browser->result_selection.removeAll();
	}
	browser->hier_selection.removeAll();
	browser->hierarchy_list->app_nodes(&browser->hier_selection);
	browser->rtl_select(extend);
    } else {
	browser->delete_selection(browser->hierarchy_list);
	browser->result_selection.removeAll();
	browser->domain_selection.removeAll();
	browser->hier_selection.removeAll();
	browser->hierarchy_list->app_nodes(&browser->hier_selection);
	browser->rtl_select(0);
    }
#else
OSapi_fprintf (stderr, "Error: projectBrowser::hierarchy_select called but not implemented\n");
#endif
}

void projectBrowser::domain_select(gtList*, gtEventPtr e, void* pb, gtReason)
//
// Selection callback for project query-domain list.
//
{
#ifndef NEW_UI
    projectBrowser *browser = (projectBrowser *)pb;

    SetNextRtl(browser->result_list);

    symbolArr old_selection = browser->selected_symbols;
    old_selection.insert_last(browser->selected_projects);

    if (e){
	int extend = shift_or_control(e);
	if(!extend){
	    browser->delete_selection(browser->domain_list);
	    browser->result_selection.removeAll();
	    browser->hier_selection.removeAll();
	}
	browser->domain_selection.removeAll();
	browser->domain_list->app_nodes(&browser->domain_selection);
	browser->rtl_select(extend);
    } else {
	browser->delete_selection(browser->domain_list);
	browser->result_selection.removeAll();
	browser->hier_selection.removeAll();
	browser->domain_selection.removeAll();
	browser->domain_list->app_nodes(&browser->domain_selection);
	browser->rtl_select(0);
    }

    symbolArr selection = browser->selected_symbols;
    selection.insert_last(browser->selected_projects);
    
    if (browser->pBController->need_update_queries(old_selection, selection))
	browser->update_ask();

    browser->update_links();

#else
OSapi_fprintf (stderr, "Error: projectBrowser::domain_select called but not implemented\n");
#endif
}


void projectBrowser::result_select(gtList*, gtEventPtr e, void* pb, gtReason)
//
// Selection callback for project query-results list.
//
{
#ifndef NEW_UI
    projectBrowser *browser = (projectBrowser *)pb;
    
    SetNextRtl(browser->domain_list);

    symbolArr old_selection = browser->selected_symbols;
    old_selection.insert_last(browser->selected_projects);

    if(e){
	int extend = shift_or_control(e);
	if(!extend){
	    browser->delete_selection(browser->result_list);
	    browser->domain_selection.removeAll();
	    browser->hier_selection.removeAll();
	}
	browser->result_selection.removeAll();
	browser->result_list->app_nodes(&browser->result_selection);
	browser->rtl_select(extend);
    } else {
	browser->delete_selection(browser->result_list);
	browser->hier_selection.removeAll();
	browser->domain_selection.removeAll();
	browser->result_selection.removeAll();
	browser->result_list->app_nodes(&browser->result_selection);
	browser->rtl_select(0);
    }

    symbolArr selection = browser->selected_symbols;
    selection.insert_last(browser->selected_projects);
    
    if (browser->pBController->need_update_queries(old_selection, selection))
	browser->update_ask();

#else
OSapi_fprintf (stderr, "Error: projectBrowser::result_select called but not implemented\n");
#endif
}

static void convert_to_local_module(symbolArr &selection)
{
    Initialize(convert_to_local_module);

    for(int i = 0; i < selection.size(); ++i)
    {
        symbolPtr sym = selection[i];
        if (sym.is_xrefSymbol() || sym.is_instance() || sym.is_ast() || sym.is_dataCell())
            continue;

        if (sym.get_kind() == DD_PROJECT)
            continue;
        if (sym.get_kind() == DD_SCOPE)
            continue;

        symbolPtr sym2 = sym.get_xrefSymbol();

        if (sym2.isnull())
            continue;

        projModule *mod =0;
	if (is_app(sym))
	    mod = appHeader_get_projectModule(sym);
	else if(is_projModule(sym))
	    mod = checked_cast(projModule, sym);

	if (!mod)
	    continue;
	
        genString fn;
        genString module_ln;
        mod->get_phys_filename(fn);
        projNode *pn = mod->get_project();

        projNode *lpn = pn;
        genString ln = mod->get_name();
        if (fn.length())
//
//	    projHeader::fn_to_ln(fn, ln, &lpn, 0, 1);
//
// Go through all writable projects, and find the one which has same physical
// and logical names as the names that we already have (in other words, find
// local project corresponding to these two names). Then use its projModule,
// not the system one. If not found, it means basically that the file is not
// checked out, so use the system projModule, which we already have.
//
// The commented statement used to find the first local project which has fn
// (ignoring logical name); we look for one which has both fn and ln.  
//	    
 	{ 	 
	    projNode *proj; 	 
	    int done = 0; 	 
	    for (int j = 1; (proj = projList::search_list->get_proj(j)) && done == 0; j++) 	 
	    { 
// 
// Find out whether proj has this module.  
//	 		
		module_ln = NULL; 		
		if (proj->is_writable()) 		 
		    proj->fn_to_ln(fn, module_ln, FILE_TYPE, 0, 1);
		if (module_ln.length()) 		 
		    if (module_ln == ln) 		 
		    { 			
			lpn = proj; 			
			done = 1; 		 
		    } 	 
	    } 	 
	    if (pn && lpn) 	 
		if (pn != lpn || 1) 		
		{ 		 
//		    projModule *lmod = lpn->find_module(ln); 		 
		    projModule *lmod = NULL;
		    symbolArr& proj_contents = lpn->rtl_contents();
		    symbolPtr sym;
		    ForEachS(sym, proj_contents)
		    {
			if (ln == sym.get_name())
			{
			    lmod = checked_cast(projModule, sym);
			    break;
			}
		    }
		    if (lmod)
			selection[i] = lmod; 		
		} 	 
	}     
    }     
    return; 
} 

void projectBrowser::rtl_select(int extend)
//
// Store selection in list and report to browserShell.
//
{
    Initialize(projectBrowser::rtl_select);
    
    push_busy_cursor();

    if (!extend) {
	browserShell::clear_all_browser_selections (&bsh);
	driver_instance->clear_selection (false);
    }

    selected_symbols.removeAll();
    selected_modules.removeAll();
    selected_projects.removeAll();

    symbolArr selection;
    selection.insert_last(hier_selection);
    selection.insert_last(result_selection);
    selection.insert_last(domain_selection);
    if(selection.size())
    {
        convert_to_local_module(selection);
	for(int i = 0; i < selection.size(); ++i)
	{
	    fsymbolPtr  sym = selection[i];              
	    if (sym.is_ast() || sym.is_dataCell()) {
		if (!selected_symbols.includes(sym))
		    selected_symbols.insert_last(sym);
	    } else if (sym.without_xrefp()) {
		ddKind knd = sym.get_kind();
		switch(knd)
		{
		    case DD_MODULE:
			selected_modules.insert_last(selection[i]);
			selected_symbols.insert_last(selection[i]);
			break;

		    case DD_PROJECT:
		      case DD_SCOPE:
			selected_projects.insert_last(selection[i]);
			break;

		      case DD_BUGS:
			selected_symbols.insert_last(selection[i]);
			break;

		    default:
		       break;
		}
	    } else {
		fsymbolPtr  xsym = selection[i].get_xrefSymbol();
		if (xsym.xrisnull()) {
		    if (selected_projects.includes(sym))
			continue;

		    Relational *rel_ptr = (Relational *)sym;
		    if (is_RTL (rel_ptr)) {
			RTL *rtl = (RTL *)rel_ptr;
			RTLNode *instance_nodes =
			    checked_cast(RTLNode,((RTL *)rel_ptr)->get_root());
			::obj_insert(rtl, REPLACE, instance_nodes, instance_nodes, NULL);
			selected_projects.insert_last(sym);
		    }
		} else {
		    if (dd_can_be_selected(xsym.get_kind()))
		    {
			if (xsym.get_kind() == DD_MODULE)
			{
			    selected_modules.insert_last(selection[i]);
			    selected_symbols.insert_last(selection[i]);
			}
			else if (xsym.get_kind() == DD_PROJECT ||
				 xsym.get_kind() == DD_SCOPE)
			    selected_projects.insert_last(selection[i]);
			else
			    selected_symbols.insert_last(selection[i]);
		    }
		}
	    }
	}

	symbolPtr sym   = selection[selection.size() - 1];
        genString info_msg;
	if(sym.without_xrefp())
	    (void) get_display_string_from_symbol(sym, info_msg);
	else {
	    fsymbolPtr xsym = sym.get_xrefSymbol();
	    if (xsym.xrisnotnull())
		(void) get_display_string_from_symbol(xsym, info_msg);
	}

	if (info_msg.length())
	    browserShell::display_assist_line( (char *)info_msg, 1);
    }

    bsh.select(this);
    pop_cursor();
}
int cli_eval_query(const char*cmd, symbolArr&selection, symbolArr&answer);
void  cli_get_roots(symbolArr&roots);

void lookup_scope_or_project (const char *, symbolPtr&);
void projectBrowser::filter(const char* input)
//
// Attempt to browse the project(s) named in input.
//
{
    Initialize(projectBrowser::filter);
#ifndef NEW_UI
    genString not_found;	// Accumulates names of missing projects.
    RTLPtr first_proj_head = NULL;

    projList::search_list->parse_names_init(input);
    const char* proj_name;
    symbolArr scopes;
    objArr   projects;
    int expanded = 0;
    symbolPtr scope_sym;
    while(proj_name = projList::search_list->parse_names_next(&scope_sym)){
	RTLPtr proj_head = NULL;

	if(!strcmp(proj_name, "/")){
	    proj_head = top_proj;
	} else {

	  //boris: looks first for the scope if the scope_mode is not zero,
	  //       otherwise looks first for the project
          if ( !scope_sym.isnotnull() )
	      lookup_scope_or_project (proj_name, scope_sym);

	  if (scope_sym.isnotnull() && scope_sym.get_kind() == DD_SCOPE) {
	      scopes.insert_last(scope_sym);
	  } else {
	    projNode *proj_node = NULL;
	    if(scope_sym.isnotnull() && scope_sym.get_kind() == DD_PROJECT){
		proj_node = checked_cast(projNode,((Relational *)scope_sym));
		projects.insert_last(proj_node);
	    } else {
	      symbolArr projs;
              symbolArr roots;
	      cli_get_roots(roots);
	      genString cmd;
	      cmd.printf("find_project %s", proj_name);
              cli_eval_query(cmd.str(),  roots, projs);
              int sz = projs.size();
              if(sz==0){
		if(not_found.not_null())
		    not_found += ", ";
		not_found += proj_name;
	      } else {
		expanded = 1;
		proj_node = checked_cast(projNode, (Relational*)projs[0]);
		for(int ii=0; ii<sz; ++ii)
		  projects.insert_last((Relational*)projs[ii]);
	      }
	    }

	    if(proj_node)
	      proj_head = checked_cast(RTL,proj_node->get_header());
	    if(first_proj_head == NULL)
	      first_proj_head = proj_head;
	  }
	}
    }
     
    if(not_found.not_null())
	msg("Following project(s) not found: $1", error_sev) << not_found.str() << eom;
    else if(scopes.size() && first_proj_head)
	msg("Following project(s) not found: $1", error_sev) << "scopes and projects" << eom;
    if(gt_form_hierarchy->is_mapped()) {
	if(first_proj_head  &&  first_proj_head != hierarchy_list->RTL())
	    browse(first_proj_head);
    } else if(gt_form_xref->is_mapped()){
      init_search_path();
      objArr   subproj_Arr;
      if(scopes.size()){
	int sz = scopes.size();
	for(int i = 0; i < sz; ++i)
	  subproj_Arr.insert_last((Relational*)scopes[i]);
        if(sz == 1){
	  genString txt;
	  get_display_string_from_symbol(scopes[0], txt);
	  browserShell::display_assist_line(txt, 1);
	}
      } else {
        int sz = projects.size();
        if(sz > 0) {
	  for (int i = 0; i < sz; ++i)
	    subproj_Arr.insert_last(projects[i]);
	  if(expanded){
	    genStringPlus names;
	    for (int i = 0; i < sz; ++i){
	      char* nm = RelationalPtr(projects[i])->get_name();
	      if(i>0)
		names += ' ';
	      names += nm;
	    }
	    gt_string_ed->text(names);
	  }
	} else {   // slash pseudo-project "/"
          projNode*pr;
	  for (int i = 0; (pr = projList::search_list->get_scan(i)); ++i)
	    subproj_Arr.insert_last(pr);
	}
      }
      scope.set_domain(subproj_Arr);
      select_group(NULL);
    }
#else
OSapi_fprintf (stderr, "Error: projectBrowser::filter called but not implemented\n");
#endif
}


void projectBrowser::open(symbolPtr sym)
//
// Create a view of the selected object, or browse the selected project.
//
{
    Initialize(projectBrowser::open);

    if (sym.relationalp() && is_RTL(sym)){         //Loaded Files case
      RTL * rel_ptr = checked_cast(RTL, sym);
#ifndef NEW_UI
      hierarchy_list->set_rtl(rel_ptr );
      up_button->set_sensitive(1);
      browse_scan_radiobox->set_sensitive(0);
      gt_string_ed->text(rel_ptr->get_root()->get_name());
      gt_string_ed->set_sensitive(0);
#endif
      cur_proj = rel_ptr;
    } else if (sym.relationalp()){
      Relational*node = sym;
      if(is_projNode(sym)) {
	projNode *prn = (projNode *)node;
	projHeader *prh = checked_cast(projHeader,prn->get_header());
	browse (prh);
      } else if (is_scopeNode(node)){
	browse((RTL*)node);
      }
    } else {
#ifndef NEW_UI
	gt_string_ed->set_sensitive(1);
#endif
	switch(sym.get_kind())
	{
	  case DD_PROJECT:
	    browse(checked_cast(projHeader, sym.get_def_app()));
	    break;
	    
	  case DD_MODULE:
	  {
	      appPtr app_head = checked_cast(app, sym.get_def_app());
	      appTreePtr app_root = checked_cast(appTree, app_head->get_root());
	      view_create(app_root);
	      view_create_flush();
	  }
	  break;
	}
    }
}


void projectBrowser::deselect()
//
// Clear current selection.
//
{
    Initialize(projectBrowser::deselect);

    delete_selection(NULL);
}

void regenerate_rtl_and_refresh_views(RTL *rtl); // defined in RTL.h.C

//******************************************
// projectBrowser::refresh_browsed_project
//----------
// rebuild RTL from scratch
//******************************************

void projectBrowser::refresh_browsed_project() {
    Initialize(projectBrowser::refresh_browsed_project);

    if (gt_form_hierarchy->is_mapped()) {
	// not in scan mode
	RTL *rtl = hierarchy_list->RTL();
	if(rtl){
	    regenerate_rtl_and_refresh_views(rtl);
	    
	}
    }
}
