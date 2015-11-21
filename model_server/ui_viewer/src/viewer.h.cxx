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
// viewer.h.C

//------------------------------------------
// synopsis:
// Implementation of class viewer
//------------------------------------------

#include <cLibraryFunctions.h>
#include <msg.h>
#include <representation.h>
#include <viewerShell.h>

#include <x11_intrinsic.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>

#include <systemMessages.h>
#include <RTLdialog.h>
#include <driver.h>
#include <steView.h>
#include <viewList.h>
#include <ui-rowcolumn.h>
#include <RTL_externs.h>
#include <smt.h>
#include <objOper.h>
#include <objRawApp.h>
#include <view_creation.h>
#include <symbolLdrHeader.h>
#include <symbolLdrTree.h>
#include <ste_interface.h>
#include <ste_send_message.h>
#include <db.h>
#include <save_app_dialog.h>
#include <gtPushButton.h>
#include <db_intern.h>
#include <ldrOODT.h>
#include <histview.h>
#include <steScreen.h>
#include <steHeader.h>
#include <steShellView.h>
#include <errorBrowser.h>

#ifndef _HelpUI_h
#include <HelpUI.h>
#endif

init_relation(screen_of_viewer,1,relationMode::D,viewer_of_screen,1,NULL);
init_relational(viewer,Relational);

int is_epoch_reporting();

#undef Window

// Functions from UIM/X interface:

extern "C" {
    Widget create_viewer(Widget, viewer*, int);
    void   viewer_activate(Widget);
    void   viewer_manage(Widget);
    void   viewer_show_current(Widget, int);
    void   viewer_show_parse_button(Widget, int, int);
    void   viewer_set_target_toggle(Widget, int);
    void   vs_set_decorate_source(Widget, int);
    void   set_smt_map_header (void*, int, int);
}

// PD-external declarations (01/95)

extern void     set_preferred_viewer(viewer *);

int els_reparse_modules(symbolArr&, symbolArr&);

void viewer::set_screen (steScreen *s)
{
    Initialize(viewer::set_screen);
    if (s == NULL)
	s = db_new(steScreen,((steBuffer *)NULL, (long)wind));

    if (s->get_screen())
	put_relation(screen_of_viewer,this,(Relational *)s);
    else {
	steHeader(Message)
	    << "Delete an empty screen."
	    << steEnd;
	obj_delete (s);
    }
}

steScreen *viewer::get_screen ()
{
    Initialize(viewer::get_screen);
    steScreen *scr = checked_cast(steScreen,get_relation(screen_of_viewer,this));
    return scr;
}

steScreenPtr viewer::get_screen_of_view (viewPtr v)
{
    Initialize(get_screen_of_view);
    steScreenPtr scr = NULL;
    viewerShell *vsh = (v) ? viewerShell::viewerShell_of_view (v) : NULL;
    if (vsh) {
	viewer *vwr = vsh->viewer_of_view (v);
	scr = (vwr) ? vwr->get_screen () : NULL;
    }
    return scr;
}

viewPtr viewer::get_current_view_of_screen (steScreenPtr scr)
{
    Initialize(get_current_view_of_screen);
    viewPtr v = NULL;
    if (scr) {
	viewer *vwr = checked_cast(viewer,get_relation(viewer_of_screen,scr));
	v = (vwr) ? vwr->cView : NULL;
    }
    return v;
}

viewer* viewer::get_viewer_of_view (viewPtr v)
{
    Initialize(viewer::get_viewer_of_view);
    steScreenPtr scr = get_screen_of_view (v);
    viewer* vwr = NULL;
    if (scr)
	vwr = checked_cast(viewer,get_relation(viewer_of_screen, scr));

    return vwr;
} 

RelationalPtr viewer::rel_copy () const
{
    return db_new (viewer, (*this));
}

void viewer::print(ostream& st, int level) const 
{
   gen_print_indent(st, level);
   st << "cView " << (void *)cView << " shell " << (void *)shell
      << " viewNum " << viewNum << " viewType " << (int) current_type
      << " Widget " << (long) wij << " Window " << (long) wind;
}

extern "C" void reset_history_views(viewPtr, viewPtr);
static viewer* the_preferred_viewer; // Target viewer selected by user.

viewer::viewer(viewerShell* vs, Widget parent, int viewer_number)
: cView(NULL), viewNum(-1), shell(vs), current_type(view_Last),
  wij(NULL), viewMenu(NULL), hScroll(NULL), vScroll(NULL), wind(NULL),
  rtl_view(NULL), history_list(NULL), history_index(0), vform(NULL)
//
// Constructor
//
{
    Initialize(viewer::viewer);
    
    wij = create_viewer(parent, this, viewer_number);

    add_view_menu_rtl(shell->get_rtl(), viewer_number);

    history_list = db_new(RTL, (NULL));
}


extern "C" void viewer_init(
    viewer* vw, Window w, Widget vm, Widget hs, Widget vs, Widget vf)
{ vw->init(w, vm, hs, vs, vf); }


void viewer::init(Window w, Widget vm, Widget hs, Widget vs, Widget vf)
//
// Initialize member variables copied from the UIM/X widget in viewer.if
//
{
    Initialize(viewer::init);

    wind = w;
    viewMenu = vm;
    hScroll = hs;
    vScroll = vs;
    vform   = vf;
}

Window viewer::get_win_behind_epoch()
{
    Initialize(viewer::get_win_behind_epoch);

    return wind;
}


void viewer::manage_widget()
//
// Manage the widget.
// This is usually done after we know there is an open view in the viewer.
//
{
    viewer_manage(wij);
}


extern viewer *cur_viewer;

viewer::~viewer()
//
// destructor
//
{
    Initialize(viewer::~viewer);

    steScreen *scr = get_screen ();
    if (scr) {
	rem_relation (screen_of_viewer,this,scr);

	// Epoch has to be informed NOW, not later.  The viewer shell will be gone later!
	scr->edt_delete_frame();

	obj_delete (scr);
    }

    // reset cmd cur_viewer
    if (cur_viewer == this)
	cur_viewer = 0;
    
    XtUnmanageChild(wij);

    view_unset_target_viewer(this);
    viewerShell::not_focused_viewer(this);
    if(the_preferred_viewer == this)	// Unset the_preferred_viewer if necessary.
	the_preferred_viewer = NULL;

    if(rtl_view)
	obj_delete(rtl_view);

    if(history_list)
	obj_delete(history_list);

    close_window();

    XtDestroyWidget(wij);
}


static viewType lookup_viewtype(appType app_type, repType rep_type)
//
// Map an (appType,repType) pair into a viewType.
//
{
    Initialize(lookup_viewtype);

    switch (app_type)
    {
      case App_RAW:
	return view_Raw;

      case App_XREF:
	if (rep_type == Rep_Tree)
	    return view_CallTree;
	else
	    return view_Last;

      case App_SMT:
	if (rep_type == Rep_Tree)
	    return view_CallTree;
	else
	    return view_SMT;

      case App_STE:
	return view_STE;

      case App_DD:
	if (rep_type == Rep_ERD)
	    return view_ERD;
	else if (rep_type == Rep_DataChart)
	    return view_DC;
	else
	    return view_Class;

      case App_SUBSYS:
	if (rep_type == Rep_SubsystemMap)
	    return view_SubsysMap;
	else
	    return view_SubsysBrowser;

      case App_LAST:
	if (rep_type == Rep_ERD)
	    return view_ERD;
	else if (rep_type == Rep_DataChart)
	    return view_DC;
	else
	    return view_Last;

      case App_RTL:
	if (rep_type == Rep_ERD)
	    return view_ERD;
	else if (rep_type == Rep_DataChart)
	    return view_DC;
	else if (rep_type == Rep_MultiConnection ||
		 rep_type == Rep_OODT_Inheritance ||
		 rep_type == Rep_OODT_Relations ||
		 rep_type == Rep_OODT_Scope)
	    return view_Class;
	else if (rep_type == Rep_SubsystemMap)
	    return view_SubsysMap;
	else if (rep_type == Rep_Tree)
	    return view_CallTree;
	else if (rep_type == Rep_DGraph)
	    return view_DGraph;
	else
	    return view_Last;

      default:
	break;
    }
    return view_Last;   
}


void viewer::init_view_type()
//
// Set up the viewType option-menu, reparse button.
//
{
    Initialize(viewer::init_view_type);

    ldrPtr ldr_head = cView ? checked_cast(ldr, cView->get_ldrHeader()) : NULL;
    appPtr app_head = ldr_head ? ldr_head->get_appHeader() : NULL;
    repType rt = cView ? repType(cView->get_type()) : Rep_Last;
    appType at = (appType)(app_head ? app_head->get_type() : App_LAST);

    viewType new_type = lookup_viewtype(at, rt);

    if(current_type != new_type)
	current_type = new_type;
    // Initialize "Reparse" button.

    int at_all = false;
    int enabled = 0;

    if (app_head) {
	if (is_objRawApp(app_head)) {
	    fileLanguage lang = (fileLanguage)app_head->language;
	    if ((lang == FILE_LANGUAGE_C || lang == FILE_LANGUAGE_CPP) ||
		lang == FILE_LANGUAGE_FORTRAN || lang == FILE_LANGUAGE_COBOL ||
		lang == FILE_LANGUAGE_ESQL_C || lang == FILE_LANGUAGE_ESQL_CPP ||
		lang == FILE_LANGUAGE_ELS) {
		genString ln;
		const char* fn = app_head->get_name();
		projNode* project = fn ? projHeader::fn_to_project (fn) : NULL;
		if (project) { 
		    project->fn_to_ln (fn, ln);
		    if (ln.str())
			at_all = enabled = true;
		}
	    }
	} else if (is_smtHeader(app_head)) {
	    smtHeaderPtr smt_head = smtHeaderPtr(app_head);
	    at_all = true;
	    enabled = smt_head->sam | smt_head->arn;
	} else if (ldr_head && is_ldrOODT(ldr_head)) {
	    at_all = true;
	    enabled = ldrOODTPtr(ldr_head)->has_pending();
	}
    }

    viewer_show_parse_button(wij, at_all, enabled);
}

int ui_viewer_focus_viewer (void *ptr)
{
    Initialize(ui_viewer_focus_viewer);
    int res = -1;
    if (ptr) {
	viewer *vwr = (viewer *)ptr;
	vwr->change_view ((view *)NULL);
	res = 0;
    }
    return res;
}

void viewer::change_view(viewPtr new_view, int old_view_deleted)
//
// Change the viewHeader currently displayed in this viewer.
//
{
    Initialize(viewer::change_view);

    if(new_view != cView || (!new_view && !cView))
    {
	dialogPtr dlg = shell->getDialog();
	new_view = shell->request_view(new_view);
	const int new_viewNum = dlg->get_view_num(new_view);

	if (new_viewNum == -1)
	    new_view = NULL;
	
	if(new_view != cView)
	{
	    // Save old viewHeader for later, when it may be deleted.
	    viewPtr old_view = cView;


	    //////////////
	    // If this is the only viewer visting this view and it is about
	    // to go away save the current point.
	    //    (For example: viewing four functions defined in a single 
	    //                  file, in a single viewer.  Since there is 
	    //                  a single buffer on the emacs side the point
	    //                  needs to be stored for eacs view as it is
	    //                  toggled.
	    //if(old_view && is_steView(old_view) && 
	    //   (get_shell()->view_in_use(old_view) == 1))
	    //      old_view->save_current_point();

	    // Close window of old view.
	    close_window();

	    // Store new viewHeader and index of view in dialog.
	    cView   = new_view;
	    viewNum = new_viewNum;

	    // Unmap/Map epoch screen if switching between graphical textual views
	    steScreenPtr scr = checked_cast(steScreen, get_relation(screen_of_viewer, this));
	    int          map = 0;
	    int   do_mapping = 0;
	    if(old_view && new_view && (is_steView(old_view) != is_steView(new_view))){
		map        = is_steView(new_view);
		do_mapping = 1;
	    }else
		if(!new_view && old_view){ // switching from graphical view to epoch view
		    map        = 1;
		    do_mapping = 1;
		}else
		    if(new_view && !old_view){ // switching from epoch view to graphical view
			map        = 0;
			do_mapping = !is_steView(new_view);
		    }else
			if(old_view && new_view && !is_steView(new_view)){
			    do_mapping = 1;
			    map        = 0;
			}
	    if(do_mapping && scr)
		ste_epoch_map_screen(scr->get_screen(), map);


	    // See above note.
	    // if(new_view && is_steView(new_view) && 
	    //   (get_shell()->view_in_use(new_view) == 0))
	    //     new_view->set_current_point();


	    // Initialize viewType option-menu and "Reparse" button.
	    init_view_type();

	    // Inform the shell of the change, so it can update global
	    // menus, panners, etc.
	    shell->viewer_changed(this);

	    // Clean up duplicate views.
	    if(old_view  &&  shell->find_duplicate_view(old_view))
	    {
                // reset_history_views(old_view, cView);
		shell->remove_view_from_menu(old_view);
		if(!old_view_deleted)
		    obj_delete(old_view);
	    }

	    if(!do_mapping || !map)  // No scroll bars for emacs viewers.
		dlg->register_scrollbars(viewNum, hScroll, vScroll);

	    viewer_activate(wij);
	}
	update_views_option_menu();
	manage_widget();
    }
}


extern "C" void viewer_change_view_type(viewer* vwr, repType t)
{ vwr->change_view_type(t); }


void viewer::change_view_type(repType rep_type)
//
// Convert the view currently displayed in this viewer to a new repType.
//
{
    Initialize(viewer::change_view_type);

    if(rep_type >= 0  &&  cView)
    {
	viewer *pref_vw = the_preferred_viewer;
	the_preferred_viewer = NULL;

	view_set_target_viewer (this);

	shell->remove_view_from_menu(cView);

	// Forget that we know about this view, so viewerShell::detach_hook
	// will not try to stuff a different viewHeader into this viewer.
	// save the cView for deleting all history associations
	viewPtr old_view = cView;
	cView            = NULL;

	dialogPtr dlg = shell->getDialog();
	dlg->change_view((void*)wind, viewNum, rep_type);
	// Get the replacement viewHeader.
	if(cView = dlg->get_view_ptr(viewNum))
	{
	    // Unmap/Map epoch screen if switching between graphical textual views
	    steScreenPtr scr = checked_cast(steScreen, get_relation(screen_of_viewer, this));
	    if(old_view && cView)
		if(is_steView(old_view) && !is_steView(cView))
		    if(scr)
			ste_epoch_map_screen(scr->get_screen(), 0);
	    
	    // reset history relations
	    // reset_history_views(old_view, cView);
	    // Tell the viewerShell about it.
	    shell->add_view_to_menu(cView);
	    shell->viewer_changed(this);

	    // Update the current selection in the views option-menu for
	    // this viewer only.
	    update_views_option_menu();
	}
	else
	{
	    const int first = shell->getDialog()->first_view();
	    viewPtr first_view = shell->getDialog()->get_view_ptr(first);
	    viewPtr switch_view = shell->request_view(first_view);
	    change_view(switch_view, 1);
	}
	the_preferred_viewer = pref_vw;
    }
}

void viewer::change_view_from_history(int delta, int delete_flag)
//
// Change the current view using the history of previous views.
//
{
    Initialize(viewer::change_view_from_history);

    symbolArr& rtl_array = RTLNodePtr(history_list->get_root())->rtl_contents();
    const int size = rtl_array.size();
    int old_view_deleted = (delete_flag == 1) ? 1 : 0;
    viewPtr new_view = NULL;
    historyObjPtr hobjPtr;
    if(delta < 0) {
	if(history_index == -1)
	    history_index = size;

	if(history_index > 0) {
	    do {
		--history_index;
		// get the view pointer from the relation
		hobjPtr = (historyObjPtr)(ObjPtr(rtl_array[history_index]));
		new_view = hobjPtr->get_viewPtr(); 
	    }while(!new_view && history_index > 0);

	    if (new_view && history_index >= 0) {
	        change_view(new_view, old_view_deleted);
	        hobjPtr->focus();
	    } else
		if (!delete_flag) msg("WARN: No dispayable views left.") << eom;
	} else
	    if (!delete_flag) msg("WARN: No previous view.") << eom;
    } else if(delta > 0) {
	if(history_index + 1 < size) {
	    viewPtr new_view;
	    historyObjPtr hobjPtr;
	    do {
		++history_index;
		hobjPtr = (historyObjPtr)(ObjPtr(rtl_array[history_index]));
		new_view = hobjPtr->get_viewPtr(); 
	    }while (!new_view && history_index < size -1);

	    if (new_view && history_index < size) {
		change_view(new_view, old_view_deleted);
		hobjPtr->focus();
	    } else 
		if (!delete_flag) msg("WARN: No dispayable views left.") << eom;
	} else
	    if (!delete_flag) msg("WARN: No next view.") << eom;
    }

    if (new_view == NULL && old_view_deleted)
	change_view ((viewPtr)NULL, 1);

    if (old_view_deleted)
	history_index = -1;
}

void viewer::add_view_to_history()
//
// Insert the current view at the top of the history list and reset
// the state of the history mechanism.
//
{
    Initialize(viewer::add_view_to_history);

    if (cView)
    {
        // Make the name unique
        RTLNodePtr rtlnode = checked_cast(RTLNode, history_list->get_root());
        symbolArr& view_array = rtlnode->rtl_contents();
        int found = 0;

        ObjPtr el;
        ForEachS (el, view_array)
        {
            historyObj *v = checked_cast(historyObj, el);
            if (strcmp(cView->get_name(), v->get_name()) == 0)
            {
                found = 1;
                break;
            }
        }

        if (!found)
        {
            historyObj *hObj = new historyObj(cView);
            history_list->obj_insert(FIRST, hObj, RTLNodePtr(history_list->get_root()), NULL);
            history_index = -1;
        }
    }
}

void viewer::update_views_option_menu()
//
// Set the currently selected value in the "List of views" option-menu.
//
{
    Initialize(viewer::update_views_option_menu);

#ifndef NEW_UI
    XtSetSensitive(viewMenu, True);
    int WasManaged = XtIsManaged(viewMenu);

    Widget subMenuId;
    XtVaGetValues(viewMenu, XmNsubMenuId, &subMenuId, NULL);

    WidgetList children;
    int numChildren;
    XtVaGetValues(subMenuId, XmNchildren, &children,
		  XmNnumChildren, &numChildren, NULL);
    if(cView == NULL){
	XtUnmanageChild(viewMenu);
	XtVaSetValues(viewMenu, XmNmenuHistory, children[0], NULL);  // 0 is always "No View"
	XtManageChild(viewMenu);
	if(!WasManaged)
	    XtUnmanageChild(viewMenu);
	return;
    }

// UNCHECKED CAST
    int title_ind = rtl_view->find_obj ( appTreePtr(shell->find_view(cView)));    if(title_ind < 0)
	return;

    // dummy Widget might get destroyed later (boris)
    // shift title_ind to skip the Widget
    if(numChildren > 1  &&  (strcmp(XtName(children[0]), "dummy") == 0))
	title_ind++;
    
    if(title_ind < numChildren)
    {
	Widget menuHistory;
	XtVaGetValues(viewMenu, XmNmenuHistory, &menuHistory, NULL);
	XtUnmanageChild(viewMenu);
	XtVaSetValues(viewMenu, XmNmenuHistory, children[title_ind], NULL);
	// Manange it to update size if title has been changed.
	XtManageChild(viewMenu);
	if(!WasManaged)
	    XtUnmanageChild(viewMenu);
	XtManageChild(viewMenu);
	if(!WasManaged)
	    XtUnmanageChild(viewMenu);
    }
#endif
}


extern "C" void viewer_manipulate_perspective(
    viewer* vw, int operation, int i1, int i2, int i3, int i4 )
{ vw->manipulate_perspective (operation, i1, i2, i3, i4); }

void viewer::manipulate_perspective(int operation, 
				    int i1, int i2, int i3, int i4 )
{
    Initialize (viewer::manipulate_perspective);

    if (cView)
	cView->manipulate_perspective (operation, i1, i2, i3, i4);
}


extern "C" int viewer_get_type(viewer* vw)
{ return vw->get_view_type(); }

extern "C" void viewer_set_screen(viewer* vw)
{ vw->set_screen(); }

int viewer::get_view_type()
{
    Initialize(viewer::get_view_type);

    return cView ? cView->get_type() : -1;
}


static void chooseViewCB(int num, char*, void* clientdata)
//
// Callback for pushButtons in the list of views menu.
//
{
    Initialize(chooseViewCB);
    viewer* vwr = (viewer*)clientdata;

    viewerShell::pre_hook(vwr->get_shell());

    viewListHeaderPtr rtl_view = vwr->get_rtl_view_ptr();
    viewPtr view_head =
	checked_cast(view, RelationalPtr(rtl_view->get_obj( num)));

    // Check for switching to view of the same object.
    if(view_head  &&
       (vwr->get_view_ptr()->get_ldrHeader() != view_head->get_ldrHeader()))
    {
	//temporarily disable prefered viewer
	viewer * pvwr = viewer::preferred_viewer();
	if (pvwr)
	    set_preferred_viewer(0);
	
	view_set_target_viewer (vwr);
	vwr->add_view_to_history();
	vwr->change_view(view_head);
	viewerShell::focused_viewer (vwr);
	
	if (pvwr)
	    set_preferred_viewer(pvwr);
    }

    viewerShell::post_hook();
}


void viewer::add_view_menu_rtl(RTLPtr rtl, int viewer_number)
//
// Add list of views option menu using rtl.
//
{
    Initialize(viewer::add_view_menu_rtl);

    if(rtl == NULL)
	return;

    RTLNodePtr rtlnode = checked_cast(RTLNode, rtl->get_root());
    viewPtr view_head =	dialog::open_view(
	rtlnode, Rep_Menu, 0, 1, rtlDisplayList);
    if(view_head == NULL)
	return;

    Widget pane;
    XtVaGetValues(viewMenu, XmNsubMenuId, &pane, NULL);

    
    ui_rowcolumn* panel = new ui_rowcolumn(pane, chooseViewCB, this, viewer_number == 1);

    rtl_view = checked_cast(viewListHeader, view_head);
    rtl_view->open_window(long(pane));
    rtl_view->set_panel(panel);
}

void viewer::close_window()
{
    Initialize(viewer::close_window);

    if(cView)
	cView->close_window();
}

extern "C" void viewer_open_window(viewer* vw)
{ vw->open_window(); }

void viewer::open_window()
{
    Initialize(viewer::open_window);

    if(viewNum >= 0) {
	viewer *prev_target = view_set_target_viewer(this);
	shell->getDialog()->create_window(viewNum, (void*)wind);
	view_set_target_viewer(prev_target);
    } else {
	steScreen * scr = get_screen ();
	if (scr) {
	  if(!is_epoch_reporting())
	    steHeader(SetCurrentPoint)
		<< -1 << " " << 0 << " " << 0 << " " << scr->get_screen () << steEnd;
	}
    }
}

void viewer::cut()
{
    Initialize(viewer::cut);

    if (cView)
	driver_instance->cut(cView);
}

void viewer::copy()
{
    Initialize(viewer::copy);

    if (cView)
	driver_instance->copy(cView);
}

void viewer::del()
{
    Initialize(viewer::del);

    if (cView)
	driver_instance->del (cView);
}


void viewer::_paste()
{
    Initialize(viewer::_paste);

    if (cView)
	driver_instance->_paste(cView);
}

void viewer::reference()
{
    Initialize(viewer::reference);

    if (cView)
        driver_instance->reference(cView);
}

void viewer::set_current()
//
// Adjust the visual attributes of this viewer so that it looks
// like the current viewer.  Make it the target viewer.
// Perform view-specific initializations.
//
{
    Initialize(viewer::set_current);

    view_set_target_viewer(this);

    viewer_show_current(wij, 1);
    shell->view_type(current_type);
}

void viewer::unset_current()
//
// Adjust the visual attributes of this viewer so that it does not
// look like the current viewer.
//
{
    Initialize(viewer::unset_current);

    viewer_show_current(wij, 0);
}


extern "C" void viewer_set_current(viewer* vwr)
//
// Ask the viewerShell to make vwr the current viewer.
//
{
    Initialize(viewer_set_current);

    viewerShell::focused_viewer(vwr);
}


extern "C" void viewer_reparse(viewer* vw)
{ vw->reparse(); }


static void really_reparse (void* data)
{
    Initialize (really_reparse);

    if (driver_instance)
	driver_instance->unselect(viewPtr(NULL));
 
    start_transaction(){
	viewer* v = (viewer*) data;
	viewPtr cView = v->get_view_ptr();
	ldrPtr ldr_head = cView ? checked_cast(ldr, cView->get_ldrHeader ()) : NULL;
	ldrTreePtr ldrt = ldr_head ? checked_cast(ldrTree, ldr_head->get_root()) : NULL;
	appTreePtr subroot = ldrt ? checked_cast(appTree, ldrt->get_appTree()) : NULL;
	appPtr ah = subroot ? checked_cast(app, subroot->get_header()) : NULL;

	int imported = 0;
	if (ah && is_objRawApp(ah)) {
	    set_smt_asynch_data (0, 0, REALLY_REPARSE_IMPORT, 0);
	    imported = ste_interface_toggle_app (objRawAppPtr(ah));
	    if (get_smt_asynch_data ()->parse_header)
		return;
	    if (imported == -1) {
		clear_smt_asynch_data ();
		genString temp;
		temp.printf ("Failed to find project for %s", ah->get_name());
		ste_print_message ((char*)temp);
		return;
	    }
	    cView = v->get_view_ptr(); // old view has been deleted !
	    ldr_head = cView ? checked_cast(ldr, cView->get_ldrHeader ()) : NULL;
	    ldrt = ldr_head ? checked_cast(ldrTree, ldr_head->get_root()) : NULL;
	    subroot = ldrt ? checked_cast(appTree, ldrt->get_appTree()) : NULL;
	    ah = subroot ? checked_cast(app, subroot->get_header()) : NULL;
	}
 
	if (ah && is_smtHeader(ah))  {
	    if (imported == 1)
		ah->set_modified ();
	    else {
		int modif = ah->is_modified ();
		if (!modif) {
		    char *name = ah->get_phys_name();
		    modif = db_get_bit_need_reparse(name);
		}
		ah->set_modified ();
		set_smt_asynch_data (ah, 0, REALLY_REPARSE_MAKE_AST, modif);
                // Here is the call to reparse the file. Clear error messages buffer before. 
		// Error browser will automatically popup at the end of transaction
		errorBrowserClear();
		errorBrowser_show_errors(); // Now actually display cleared error browser. (In order to prevent clicking to old messages)
		checked_cast(smtHeader,ah)->make_ast();
		if (get_smt_asynch_data ()->make_ast_header)
		    return; 
//		errorBrowser_show_errors();
		if (!modif) ah->clear_modified ();
	    }	    
	}
    } end_transaction();
}

void viewer::reparse()
//
// Called from "Reparse" button callback.
//
{
    Initialize(viewer::reparse);

    ldrPtr ldr_head = cView ? checked_cast(ldr, cView->get_ldrHeader ()) : NULL;
    if (ldr_head && is_ldrOODT(ldr_head)) {
       ldrOODTPtr(ldr_head)->parse_pending();
       ldrOODTPtr(ldr_head)->refresh_pending();
       return;
    }

    gtPushButton::next_help_context ("Viewer.Reparse");
    
    if (ldr_head)
	  {
	    ldrTreePtr ldrt = ldr_head ? checked_cast(ldrTree, ldr_head->get_root()) : NULL;
	    appTreePtr subroot = ldrt ? checked_cast(appTree, ldrt->get_appTree()) : NULL;
	    appPtr ah = subroot ? checked_cast(app, subroot->get_header()) : NULL;
	    projModule* pm = ah ? appHeader_get_projectModule(ah) : NULL;
	    if (!pm)
		pm = projHeader::find_module(ah->get_name());
	    if (pm)
	      {
		symbolArr mods;
		mods.insert_last(pm);
		symbolArr rest_mods;
		int res = els_reparse_modules(mods, rest_mods);
		if (!res)
		  return;
	      }
    }
	
    objArr raw_apps;
    objArr *apps = app::get_modified_headers();

    {                   // need to be in block because of "delete apps"
      Obj* el;
      ForEach (el,*apps) {
        appPtr appptr = checked_cast(app,el);
        if ((is_objRawApp(appptr) || appptr->language == FILE_LANGUAGE_ELS) && appptr->is_modified()) 
	    raw_apps.insert_last (appptr);
      }
    }

    offer_to_save_list (NULL, Flavor_Parsing, &raw_apps, really_reparse, this);

    delete apps;
}


void viewer::enable_reparse(boolean enabled)
//
// Called when smtHeaders parsing is resumed.
//
{
    Initialize(viewer::enable_reparse);

    viewer_show_parse_button(wij, true, enabled);
}


static void viewTypeCB(Widget wid, XtPointer viewer_object, XtPointer)
//
// Callback function for the pushButtons in "viewTypeMenu" (below).
// Assumes userData of widget is a repType, client_data is viewer*
//
{
    Initialize(viewTypeCB);

    viewer* vwr = (viewer*)viewer_object;
    
    repType rep_type;
    XtVaGetValues(wid, XmNuserData, &(XtPointer&)rep_type, NULL);
    if(rep_type != vwr->get_view_ptr()->get_type()){
        viewerShell::pre_hook(vwr->get_shell());
	vwr->add_view_to_history();
	vwr->change_view_type(rep_type);
	viewerShell::post_hook();
    }
}

extern "C" void viewer_target_lock(viewer* vwr, int is_set)
//
// Called by UIM/X viewer interface to report change in target toggle state.
//
{
    Initialize(viewer_target_lock);

    if(is_set)
    {
	if(the_preferred_viewer)
	    the_preferred_viewer->target_lock(0);
	if(the_preferred_viewer = vwr)		// assign & test
	    the_preferred_viewer->target_lock(1);
    }
    else if(the_preferred_viewer == vwr)
	the_preferred_viewer = NULL;
}


void viewer::target_lock(int on)
//
// Set/clear the target-lock button.
//
{
    Initialize(viewer::target_lock);

    viewer_set_target_toggle(wij, on);
}


viewer* viewer::preferred_viewer()	// class static
//
// Return the users preferred viewer, or NULL.
//
{
    Initialize(viewer::preferred_viewer);

    return the_preferred_viewer;
}

void set_preferred_viewer(viewer * vwr)
{
    the_preferred_viewer = vwr;
}

RTLPtr viewer::obtain_history_list()
{
    Initialize(viewer::obtain_history_list);

    return history_list;
}

void viewer::set_history_index(int idx)
{
    Initialize(viewer::set_history_index);

    history_index = idx;
}

/*
   START-LOG-------------------------------------------

   $Log: viewer.h.C  $
   Revision 1.46 2002/03/04 17:25:21EST ktrans 
   Change M/C/D/W/S macros to enums
Revision 1.2.1.80  1994/07/27  22:56:04  bakshi
Bug track: n/a
epoch list project

Revision 1.2.1.79  1994/07/27  00:12:32  jethran
removed quickhelp stuff as part of overall removal of quickhelp

Revision 1.2.1.78  1994/07/21  16:50:26  aharlap
bug #7895

Revision 1.2.1.77  1994/07/07  15:17:31  azaparov
Supressed scratch buffer between two graphicals.
./

Revision 1.2.1.76  1994/06/23  18:34:12  builder
hp Port

Revision 1.2.1.75  1994/05/23  15:19:27  azaparov
Changes to support new debugger interface

Revision 1.2.1.73  1994/04/13  14:23:16  builder
Port - use aset_stdarg first

Revision 1.2.1.72  1994/04/06  23:21:03  azaparov
Changed behavior of the *compilation* buffer appearing

Revision 1.2.1.71  1994/04/06  14:12:23  azaparov
Added functionality needed for error browser

Revision 1.2.1.70  1994/03/31  16:27:19  azaparov
Fixed bug 6669

Revision 1.2.1.69  1994/03/10  15:18:59  builder
Port

Revision 1.2.1.68  1994/02/21  22:52:21  builder
Port

Revision 1.2.1.67  1994/02/16  15:30:37  boris
Bug track: #6310
Fixed cmd_get_cur_view() purify fmr problem

Revision 1.2.1.66  1994/01/27  13:43:01  andrea
Bug track: 6009
typo in help path

Revision 1.2.1.65  1994/01/24  14:32:31  andrea
Bug track: 6009
Hooked up help button

Revision 1.2.1.64  1994/01/21  03:44:54  kws
*** empty log message ***

Revision 1.2.1.63  1994/01/21  03:05:38  kws
New rtl display

Revision 1.2.1.62  1993/12/31  19:43:51  boris
Bug track: Test Validation
Test Validation

Revision 1.2.1.61  1993/12/23  23:40:12  boris
Bug track: No View Problem
Fixed No View for merge

Revision 1.2.1.60  1993/12/13  21:04:51  andrea
Bug track: 5573
fixed viewerShell

Revision 1.2.1.59  1993/12/11  20:47:22  boris
Bug track: #5526, #5559
Fixed "Toggle Arrow" problem

Revision 1.2.1.58  1993/12/03  16:33:09  boris
Bug track: #4969
C-x o Emacs command for non ParaSET buffers

Revision 1.2.1.57  1993/11/24  22:55:35  boris
Bug track: #5325
Fixed synchronization and fmr viewer problems

Revision 1.2.1.56  1993/11/19  03:58:39  boris
Bug track: Emacs windows
Emacs windows for ParaSET viewers

Revision 1.2.1.55  1993/11/02  17:16:17  boris
Bug track: #5114
Fixed restoring previous view after closing the current one

Revision 1.2.1.54  1993/10/26  01:32:04  himanshu
Bug track: 5059
changed create_view_type_menu to block the type_menu for helpershell views

Revision 1.2.1.53  1993/10/25  14:00:31  himanshu
Bug track: 5058
The change_view_from_history function was modified to skip all unavailable views.
Also the history list selection was blocked for the time being. Till some issues are resolved.

Revision 1.2.1.52  1993/10/21  00:38:08  kws
Port fixes

Revision 1.2.1.51  1993/10/13  15:58:56  himanshu
Bug track: 4919
add_<view_to_history and change_view_from_history were modified.

Revision 1.2.1.50  1993/10/09  17:36:26  aharlap
async_parsing

Revision 1.2.1.48  1993/09/09  23:12:47  boris
Fixed synchro problem while parsing

Revision 1.2.1.47  1993/08/19  16:19:07  boris
Disable Reparse button depend on a RawApp Header Language

Revision 1.2.1.46  1993/08/18  00:48:11  aharlap
*** empty log message ***

Revision 1.2.1.45  1993/08/17  22:06:32  andrea
fixed bug 4369

Revision 1.2.1.44  1993/08/06  07:39:11  aharlap
don't call make_ast after import to prevent second parsing
if first parsing failed

Revision 1.2.1.43  1993/08/05  23:21:02  wmm
Fix bug 4185 (allow deferred parsing from OODT views).

Revision 1.2.1.42  1993/07/07  15:16:35  sergey
1. Changed the way filter_cplus() called. 2. Fixed a bug in really_reparse(); closed bug #3868.

Revision 1.2.1.41  1993/06/28  19:15:39  sergey
Put ForEach in block in viewer::reparse(). Fixed bug #3863.

Revision 1.2.1.40  1993/06/26  04:02:42  so
fix bug 3783
when reparse from the view window
if pset head has set the bit of reparse
then set modified for this file

Revision 1.2.1.39  1993/06/24  16:42:20  bakshi
added typecast fot c++3.0.1 port

Revision 1.2.1.38  1993/05/26  16:23:09  sergey
Put a check in filter_cplus to skip system calls. Part of bug#3492.

Revision 1.2.1.37  1993/05/25  14:02:49  sergey
Added filter_cplus call to separate C++ call tree from others. Part of bug #3319.

Revision 1.2.1.36  1993/05/23  21:20:29  sergey
Added support for CallTree in lookup_viewType(). Part of fix of bug #3319.

Revision 1.2.1.35  1993/05/14  15:49:18  glenn
Add viewer::manage_widget().

Revision 1.2.1.34  1993/05/13  22:58:33  glenn
Check for NULL return value from dialog::get_view_ptr in
viewer::change_view_type, revert to first view if necessary.

Revision 1.2.1.33  1993/05/13  18:23:23  glenn
Call viewer_manage() at the end of viewer::change_view().
Unmanage, then re-manage viewMenu after changing the menuHistory.

Revision 1.2.1.32  1993/05/03  19:58:38  glenn
Add view_Raw to lookup_viewtype.

Revision 1.2.1.31  1993/05/01  00:00:43  kws
Fix reparse to ask about saving raw views
Fix bug #3557

Revision 1.2.1.30  1993/04/28  18:55:11  aharlap
set imported flag after ste_toggle

Revision 1.2.1.29  1993/04/24  21:17:46  davea
bug 3492 - added return stmt to lookup_view_type()

Revision 1.2.1.28  1993/04/14  13:59:52  glenn
Add viewer::help_mode.

Revision 1.2.1.27  1993/04/13  19:13:22  kws
Ask saving of raw app on a reparse if modified.
Part of bug #3326

Revision 1.2.1.26  1993/04/09  14:07:48  sergey
Prevent parallel Reparse and propagation of rename. Fixed bug #3236.

Revision 1.2.1.25  1993/04/02  22:29:51  kws
Allow raw views to r/w mode dependent on file - and make reparse convert file to smode
Fixes Bug #3105

Revision 1.2.1.24  1993/02/28  01:28:37  aharlap
added restore for modified flag in reparse

Revision 1.2.1.23  1993/02/25  00:16:35  aharlap
set_modified in reparse

Revision 1.2.1.22  1993/02/24  14:20:22  glenn
ctor now interprets the last argument as the index of this viewer
in the containing viewrShell, and passes it on to create_viewer.
viewNum is always initialized to -1 in ctor.
Clear the_preferred_viewer in dtor if necessary.
Allow NULL views in init_view_type, update_views_option_menu.
Add viewer_target_lock, viewer::target_lock, viewer::preferred_viewer.

Revision 1.2.1.21  1993/02/21  00:00:04  glenn
Add flag to change_view to prevent extra deletion when called
from detach_hook.
Call ui_row_column ctor with callback function and data.
Add empty function viewer_target_lock.
Declare viewer_set_target_toggle.

Revision 1.2.1.20  1993/02/19  06:19:16  glenn
Avoid switching to identical view in chooseViewCB.

Revision 1.2.1.19  1993/02/12  20:54:43  glenn
Set cView to null in change_view_type.

Revision 1.2.1.18  1993/02/12  01:07:16  glenn
Rename set_obj_type to init_view_type.
Call viewerShell::not_focused_viewer in dtor.
Remove delete_original argument from change_view.
Call viewerShell::focused_viewer instead of set_current_viewer.

Revision 1.2.1.17  1993/02/08  16:24:13  jon
Fixed change_view_from_history so that a delta of -1 moved to the
last opened view instead of the first opened view (Bug #2167)

Revision 1.2.1.16  1993/01/28  16:29:24  wmm
Fix bug 2330.

Revision 1.2.1.15  1993/01/24  01:16:27  wmm
Support XREF-based class browser view.

Revision 1.2.1.14  1993/01/22  22:54:59  wmm
Support XREF-based ERDs and DCs.

Revision 1.2.1.13  1993/01/15  15:14:32  wmm
Make viewerShell menu capabilities consistent with popup menu
capabilities for subsystems.

Revision 1.2.1.12  1993/01/07  02:06:23  smit
get_view_type now returns -1 if there is no view for the viewer.

Revision 1.2.1.11  1992/12/21  20:08:27  glenn
Implement history-list dialog with RTLdialog.
Fix bug where viewTypes option-menu was not updated unless
switching to a view of a different app-type.
Same for Reparse button.
Remove decorate_source calls.

Revision 1.2.1.10  1992/12/19  20:34:03  wmm
Allow opening empty ERDs and DCs.

Revision 1.2.1.9  1992/12/17  21:38:46  wmm
Support Data Charts; fix compilation error under -O resulting from
conversion from symbolPtr to view.

Revision 1.2.1.8  1992/12/17  14:04:09  jon
*** empty log message ***

Revision 1.2.1.7  1992/12/17  00:13:06  jon
Changed viewer::refer to viewer::reference

Revision 1.2.1.6  1992/12/14  22:22:07  glenn
Add history_list, history_index to ctor and dtor.
Rename set_typeMenu_history to update_viewTypes_option_menu.
Rename set_viewMenu_history to update_views_option_menu.
Add change_view_from_history.
Add add_view_to_history.
Change rtl_view to viewUIHeaderPtr.
Call pre_hook, post_hook in chooseViewCB.

Revision 1.2.1.5  1992/12/12  07:07:06  glenn
New view creation interface.

Revision 1.2.1.4  1992/11/23  22:37:53  wmm
typesafe casts.

Revision 1.2.1.3  1992/10/30  21:01:03  sharris
*** empty log message ***

Revision 1.2.1.2  92/10/09  20:21:00  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/


 
