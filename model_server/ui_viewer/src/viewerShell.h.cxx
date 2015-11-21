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
// ViewerShell.h.C
//------------------------------------------
// synopsis:
// Implementation of class viewerShell
//------------------------------------------

// INCLUDE FILES
#include "msg.h"
#define _cLibraryFunctions_h
 
#include "objOperate.h"
#include "systemMessages.h"
#include "gtShell.h"
#include "gtPushButton.h"
#include <motifString.h>
#include <top_widgets.h>
#include <Xm/CascadeB.h>
#include <Xm/RowColumn.h>

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include "viewerShell.h"
#include "viewerShell_manage.h"
#include "button_bar.h"
#include "perspective.h"
#include <Panner.h>
#include "Question.h"
#include "save_app_dialog.h"
#include "Quick_Search.h"
#include "ddict.h"
#include "driver.h"
#include "dialog.h"
#include "button.h"
#include "dd_or_xref_node.h"
#include "db.h"
#include "graWindow.h"
#include "view_creation.h"
#include "ldrHeader.h"
#include "viewGraHeader.h"
#include "viewList.h"
#include "RegExp.h"
#include "cmd.h"
#include "cmd_enums.h"
#include "RTL_externs.h"
#include "steView.h"
#include "steShellView.h"
#include "ste_interface.h"
#include "steRawView.h"
#include "openFromBrowser.h"
#include "putUI.h"
#include "HelpUI.h"
#include "NewPrompt.h"
#include "merge_dialog.h"
#include "viewFlowchartHeader.h"
#include "miniBrowser.h"
#include <Xm/PushB.h>
#include "ste_interface.h"
#include "genStringPlus.h"
#include "machdep.h"
#include "ste_fonts.h"
#include "getUI.h"
#include "feedback.h"
#include "graInterface.h"
#include "groupHdr.h"
#include "attribute.h"

//------------------------------------------
// viewRTL
//------------------------------------------

RelClass(viewRTL);

class viewRTL : public RTL
{
  public:
    viewRTL(char* name);
    define_relational(viewRTL, RTL);

    virtual void notify(int, Relational*, objPropagator*, RelType*);
    virtual void xref_notify (const symbolArr& modified, 
				   const symbolArr& added, 
				   const symbolArr& deleted);
  
};
generate_descriptor(viewRTL, RTL);

init_relational(viewRTL, RTL);

// viewRTL

viewRTL::viewRTL (char *nm) : RTL(nm)
{ }

void viewRTL::xref_notify(const symbolArr& changed, const symbolArr& added, const symbolArr& deleted)
{
    Initialize(viewRTL::xref_notify);

    DBG { 
        obj_prt_obj(this);
        msg("  mad\n") << eom;
	changed.print ();
	added.print ();
	deleted.print ();
    }

    RTL::xref_notify (changed, added, deleted);
}

extern char* get_name_o_view(view*);

void viewRTL::notify (int flag, Relational *target, objPropagator*, RelType*)
{
    Initialize (viewRTL::notify);
    if (!target || !is_view(target))
	return;
    
    if (flag != relationMode::M)
	RTL::notify (flag, target, (objPropagator *)0, (RelType *)0);
    else {
	view *v = checked_cast(view, target);
	genString new_name = get_name_o_view (v);
	add_view_type_to_name(v, new_name);
	int res = v->reset_name (new_name);
	if (res == 1)
	    obj_insert (REPLACE, v, v, NULL);
    }
}

// end viewRTL



//  -------------------

// EXTERNAL VARIABLES

extern Display* UxDisplay;

// EXTERNAL FUNCTIONS

void view_flush_on_top();
void		gra_finalize(void);
smtTreePtr	dd_get_token_def(smtTreePtr);

int		configurator_to_icon();
extern void     set_preferred_viewer(viewer *);

void   cmd_record_viewer_validate (viewer *vwr);
extern "C"  void cmd_record_top_level(Widget wg);
extern bool browser_unget_module (gtBase* parent, projModule *module, bool yes_to_all);
extern void browserShell_project_merge(symbolArr& modules, int level);

extern int create_view_of_def (symbolPtr);

extern int is_viewTreeHeader (Relational*);

extern "C"
{
    Widget create_ViewerShell(viewerShell*, int);
    Widget vs_status_line_color(Widget, char *);
    void   create_menu_children(Widget, void*, va_list*);

    void   vs_set_buttons(Widget, viewType, viewType);
    void   vs_enable_decorate_source(Widget, int sensitive, int state);
    void   vs_enable_manager(Widget, int);
    void   vs_hyper_menus(Widget, int);
    void   vs_special_buttons(Widget, vsSpecialButtons);
    void   vs_destroy(Widget);
    int isServer();
}


// FUNCTION DEFINITIONS

static Widget vs_status_line (Widget w, int hilite)
{
    Initialize(vs_status_line);
    const char *color = (hilite) ? ste_editor_background (): "gray" ;
    return vs_status_line_color (w, (char *)color);
}

extern "C" char * ste_c_editor_background ()
{
    char *color_name = (char *)ste_editor_background ();
    if (!color_name || !color_name[0])
	color_name = "white";
    
    return color_name;
}

int gra_type()
{
    Initialize(gra_type);

    viewerShell *vh = viewerShell::get_current_vs(1);
    if (vh)
    {
        int vidx   = vh->get_current_viewer();
        viewer *vr = vidx >= 0 ? vh->get_viewer(vidx) : NULL;
        if (vr)
        {
            view *v = vr->get_view_ptr();
            if (v)
                return v->get_type();
        }
    }

    return Rep_Last;
}

//------------------------------------------

inline int max_int(int a, int b) { return a > b ? a : b; }

appPtr get_create_app_by_name (const char* f_name);

Widget CreatePulldown(
    Widget menuBar, void* client, char* cascBtnName,
    XtCallbackProc menuMapCallback ...)
//
// Create a new pulldown menu for the given menu bar.  It creates the
// pulldowns rowcolumn widget, and passes remaining arguments to
// create_menu_children (in ui/misc/generic-popup-menu.c) which creates the
// menus elements
//
{
    Initialize(CreatePulldown);

    Widget menuCascBtn = NULL;

    if (cascBtnName != NULL)
    {
	char pd_menu_name[64];
	strcpy(pd_menu_name, cascBtnName);
	strcat(pd_menu_name, "_menu");

	Widget pullMenu = XmCreatePulldownMenu(menuBar, pd_menu_name, NULL, 0);

	if (menuMapCallback)
	    XtAddCallback(pullMenu, XmNmapCallback,
			  menuMapCallback, XtPointer(client));

	va_list ap;
	va_start(ap, menuMapCallback);
	create_menu_children(pullMenu, client, &ap);
	va_end(ap);

	menuCascBtn = XmCreateCascadeButton(menuBar, cascBtnName, NULL, 0);
	motifString namex((char*)gettext(cascBtnName));
	XtVaSetValues(menuCascBtn,
		      XmNlabelString, XmString(namex),
		      XmNsubMenuId, pullMenu,
		      NULL);
    }

    return menuCascBtn;
}

//------------------------------------------

viewerShell::viewerShell(int create_minibrowser)
//
// viewerShell constructor
//
{
    Initialize(viewerShell::viewerShell);

    push_busy_cursor();

    edit_mode = VE_base;
    cust_mode = VC_normal;
    mini_browser         = NULL;

    dlg = new dialog("vs_dialog");
    driver_instance->add_dialog(dlg);
    dlg->set_detach_hook(this, detach_hook);

    rtl = db_new(viewRTL, (""));

    int i;
    for(i = 0; i < MAX_VIEWERS; i++)
	viewers[i] = NULL;

    current_type = view_Last;
    num_viewers = 0;
    current_viewer = -1;

    for(i = 0; i <= view_Last; i++)
    {
	buttonBar[i] = NULL;
	pullMenu[i] = NULL;
    }

    // create_ViewerShell not only creates the widget,
    // it also initializes some of the members (see initWidgetSlots).
    wij = create_ViewerShell(this, create_minibrowser);
    gtShell_init_focus_handler(wij);

    Panner_perspective(panner, 0, 0, 100, 100, 0, 0, 100, 100);

    // create all the pulldown menus, so that the custom buttons can be loaded.
    for (i = 0; i <= view_Last; i++)
    {
	const slotfill_func* f_base = create_pullMenu[i];
	if(!f_base) f_base = create_pullMenu[view_Last];

	const slotfill_func *f_ptr;
	for(f_ptr = f_base; f_ptr && *f_ptr; ++f_ptr) {}
	const int num_menus = f_ptr - f_base;

	pullMenu[i] = (Widget*)calloc(num_menus + 1, sizeof(Widget));
	for(int j = 0; j < num_menus; ++j)
	    pullMenu[i][j] = (*f_base[j])(menuBar, this);
    }
    load_buttonbars();

    enable_manager(projNode::home_is_writeable());

    if(create_minibrowser)
	mini_browser = new miniBrowser(menuBar, icon_place, debugCommandWidget);
    else
	mini_browser = NULL;

    // init menu sensitivity
    vs_set_buttons (wij, view_Last, view_Last);


    cm_button = XmCreatePulldownMenu(menuBar,"cmPulllDown",NULL,0);
    XmString cmLabel = XmStringCreateLocalized("CM");
    cmMenu = XtVaCreateManagedWidget("CM",
	    xmCascadeButtonWidgetClass,menuBar,
	    XmNlabelString,cmLabel,
	    XmNsubMenuId,cm_button,
	    NULL);
    XtSetSensitive(cmMenu,False);
    XmStringFree(cmLabel);

    buildCMMenu();
    
    pop_cursor();
}

#include "CMIntegratorCtrlr.h"
#ifndef _browserShell_h
#include <browserShell.h>
#endif

void viewerShell::cm_action(Widget menu_item, XtPointer client_data, XtPointer call_data) {
  Initialize(viewerShell::cm_command);
  viewerShell* pThis = (viewerShell*) client_data;

  string** filenames = NULL;
  string** projnames = NULL;
  
  viewPtr cView = pThis->get_current_view();
  bool bMinibrowserSelection = false;
  
  symbolArr files;
  symbolArr prjs;
  symbolArr filter;
  // trying to get selected nodes from the minibrowser
  driver_instance->fill_array_with_selected_appnodes(NULL,filter,0);
  symbolPtr el;
  { // HP and Irix 
  ForEachS( el, filter) {
      symbolPtr xsym = el.get_xrefSymbol();
      if(xsym.xrisnull()) {
          if(is_projModule(el)) files.insert_last(el);
	  else if(is_RTL(el)) prjs.insert_last(el);
      } else {
          if(xsym.get_kind() == DD_MODULE) {
	      projModule* mod = projModule_of_symbol(xsym);
	      if(mod) files.insert_last(mod);
	  } else 
	      if(xsym.get_kind() == DD_PROJECT) 
	          prjs.insert_last(el);
      }
  }
  if(files.size() || prjs.size()) bMinibrowserSelection = true;
  }

  if(bMinibrowserSelection) {      
      int len = files.size();

      filenames = (string**)new string[len+1];
      
      int idx;      	
      for(idx=0;idx<len;idx++)
          filenames[idx]=new string(ATT_physical_filename(files[idx]));
      filenames[len]=NULL;

      len = prjs.size();
      projnames = (string**)new string[len+1];
      for(idx=0;idx<len;idx++)
          projnames[idx]=new string(ATT_physical_filename(prjs[idx]));      
      projnames[len]=NULL;
  } else {
      projModule* pm = NULL;
      ldrPtr ldr_head = cView ? checked_cast(ldr, cView->get_ldrHeader ()) : NULL;
      if(ldr_head) {
         ldrTreePtr ldrt = ldr_head ? checked_cast(ldrTree, ldr_head->get_root()) : NULL;
         appTreePtr subroot = ldrt ? checked_cast(appTree, ldrt->get_appTree()) : NULL;
         appPtr ah = subroot ? checked_cast(app, subroot->get_header()) : NULL;
         pm = ah ? appHeader_get_projectModule(ah) : NULL;
         if(!pm)  pm = projHeader::find_module(ah->get_name());
      }

      if(!pm) return;
      
      filenames = (string**)new string[1+1];
      filenames[0]=new string(pm->get_phys_filename());
      filenames[1]=NULL;

      projnames = (string**)new string[1];
      projnames[0]=NULL;
  }
  
  browserShell::m_pcmController->OnCMCommand(pThis->get_widget(),XtName(menu_item),projnames,filenames);
  
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

bool viewerShell::buildCMMenu() {
#ifndef NEW_UI
  if(browserShell::m_pcmController==NULL || !browserShell::m_pcmController->m_CMIntegrator->IsConnected()) 
      return false;
  
  XtSetSensitive(cmMenu,False);
  
  Widget manageMenu = XtNameToWidget(wij,"manage_menu");
  if(manageMenu!=NULL) {
    XtUnmanageChild(manageMenu);
    XtDestroyWidget(manageMenu);
  }
  
  // remove all commands
  Cardinal nNumCommands = 0;
  if(nNumCommands > 0) {
    Widget* menuItems;
    XtVaGetValues(cm_button, XmNnumChildren,&nNumCommands,XmNchildren, &menuItems, NULL);
    for(int i=0;i<nNumCommands;i++) {
      XtUnmanageChild(menuItems[i]);
      XtDestroyWidget(menuItems[i]);
    }
  }

  CCMCommandsList::iterator iPos = browserShell::m_pcmController->m_pCMCommands->begin();
  CCMCommandsList::iterator iEnd = browserShell::m_pcmController->m_pCMCommands->end();
  while(iPos!=iEnd) {
    CCMCommand* pCommand = *iPos;
    string& cmdName = pCommand->GetName();
    if(!cmdName.empty()) {
	Widget menuItem = XtVaCreateManagedWidget((char*)cmdName.c_str(), 
	    xmPushButtonWidgetClass, cm_button, 
	    NULL);
	XtAddCallback(menuItem,XmNactivateCallback,cm_action,this);
    }
    iPos++;
  }
  XtSetSensitive(cmMenu,True);
#endif
  return true;        
}

extern "C" void viewerShell_initWidgetSlots(
    viewerShell* vs, Widget bbs, Widget vw, Widget mb, Widget dcw, Widget icon_place, Widget p)
{ vs->initWidgetSlots(bbs, vw, mb, dcw, icon_place, p); }

extern "C" void viewerShell_set_miniBrowser(viewerShell *vs, int set)
{
    miniBrowser *browser = vs->get_mini_browser();
    if(browser)
	browser->visible(set);
}

// cleaned
viewPtr viewerShell::next_view (viewPtr v) const
{
    Initialize(viewerShell::next_viewer);

    viewPtr next_v = NULL;
    int ii;
    for ( ii = 0 ; ii < num_viewers ; ii++ )
        if ( (viewers[ii])->get_view_ptr() == v )
	    break;

    if (ii < num_viewers) {
	if (++ii == num_viewers)
	    ii = 0;

	next_v = (viewers[ii])->get_view_ptr();
    }
    return next_v;
}

// cleaned:
steScreenPtr viewerShell::next_screen (steScreenPtr curr_scr, viewPtr& v) const
{
    Initialize(viewerShell::next_screen);
    
    steScreenPtr nxt_scr = NULL;
    viewerPtr nxt_vwr    = NULL;
    v = NULL;
    
    int ii;
    if (current_viewer >= 0)
	ii = current_viewer;
    else {
	if (!curr_scr)
	    ii = num_viewers;
	else {
	    viewer *vwr = checked_cast(viewer,get_relation(viewer_of_screen,curr_scr));
	    if (!vwr)
		ii = num_viewers;
	    else {
		for ( ii = 0 ; ii < num_viewers ; ii++ )
		    if ((viewers[ii]) == vwr)
			break;
	    }
	}
    }

    if (ii < num_viewers) {
	if (++ii == num_viewers)
	    ii = 0;
	nxt_vwr = viewers[ii];	
    }

    if (nxt_vwr) {
	v = nxt_vwr->get_view_ptr();
	nxt_scr = checked_cast(steScreen,get_relation(screen_of_viewer,nxt_vwr));
    }

    return nxt_scr;
}

void viewerShell::initWidgetSlots(
    Widget bbs, Widget vw, Widget mb, Widget dcw, Widget icon, Widget p)
//
// Callback to initialize Widget members from UIM/X interface.
//
{
    Initialize(viewerShell::initWidgetSlots);

    buttonBarSlot       = bbs;
    viewWindow          = vw;
    menuBar             = mb;
    debugCommandWidget  = dcw;
    this->icon_place    = icon;
    panner              = p;
}

extern "C" void viewerShell_unmap_widget (Widget w)
{
  if (w) {
    XtUnrealizeWidget(w);
  }
}

extern "C" void viewerShell_unmap (viewerShell *vs)
{
  if (vs) {
    viewerShell_unmap_widget (vs->get_widget());
  }
}

extern "C" void viewerShell_delete(viewerShell* vs)
{ delete vs; }

extern "C" void viewerShell_flush() { XFlush(UxDisplay); }
extern "C" void viewerShell_sync(int discard) { XSync(UxDisplay, discard); }

// cleaned:
viewerShell::~viewerShell()
//
// viewerShell destructor
//
{
    Initialize(viewerShell::~viewerShell);

#ifdef PMG
    for(int ii = 0; ii < get_num_viewers(); ++ii)
    {
	viewerPtr vwr = get_viewer(ii);
	viewPtr   vw  = vwr->get_view_ptr();
	steScreenPtr scr = checked_cast(steScreen, get_relation(screen_of_viewer, vwr));
	if(scr)
	    ste_epoch_map_screen(scr->get_screen(), 0);
    }

#endif

    if(mini_browser)
	delete mini_browser;

    // Remove this viewerShell from the list of available viewerShells.
    vsh_array.remove(this);
    set_current_vs(NULL);

    for(int i = 0; i < view_Last; ++i)
	if(pullMenu[i])
	    OSapi_free(pullMenu[i]);

    while(num_viewers > 0)
	unsplit();
    
    // The above unspilt will communicate to epoch to delete a window.
    // Don't unmap until then!
    XtUnmapWidget(wij);
    XFlush(UxDisplay);

    driver_instance->close_dialog(dlg);

    // Clean up special views 
    if(this == hook_shell)	// Shell used in pre_hook, post_hook.
	hook_shell = NULL;

    XtDestroyWidget(wij);
}


void viewerShell::map()
//
// Make the viewerShell visible on the display.
//
{
    Initialize(viewerShell::map);

    for(int i = 0; i < num_viewers; ++i)
	viewers[i]->manage_widget();

    XtMapWidget(wij);
}


viewerShell* viewerShell::get_vs_by_idx(int idx)
// get the viewerShell by index: idx
{
    Initialize(viewerShell::get_vs_by_idx);
 
    viewerShell* retval = NULL;
 
    if(!vsh_array.empty() && idx < vsh_array.size())
         retval = (viewerShell*)(vsh_array[idx]);
 
     return retval;
}

extern "C" vs_set_status_line_message(Widget,char*);
void viewerShell::set_status_line(const char *str) // class static
{
    // Dont create one if it doesnt exist.
    viewerShell* vs = viewerShell::get_current_vs(1);
    if(vs)
      {
	genStringPlus tmp(str);
	for(int i=0, len=tmp.size(); i < len; i++)
	  if(tmp[i]=='\n') tmp[i] = ' ';
	vs_set_status_line_message(vs->get_widget(), (char*)tmp);
      }
}
 
viewerShell* viewerShell::get_current_vs(int no_create) // class static
//
// Return the focused viewerShell, or create one if none exists.
//
{
    Initialize(viewerShell::get_current_vs);
 
    viewerShell* retval = NULL;
 
    if(!vsh_array.empty())      // Get the top element.
        retval = (viewerShell*)(vsh_array[vsh_array.size() - 1]);
    else if (no_create <= 0){        // Create a new viewer shell.
        retval = new viewerShell(CREATE_MINIBROWSER);
	if(retval && no_create < 0){
	    view_set_target_viewer(retval->create_viewer());
	    viewerShell::set_current_vs(retval);
	    retval->bring_to_top();
	    retval->map();
	}
    }
    return retval;
}


viewerShell* viewerShell::get_current_vs()		// class static
//
// Return the focused viewerShell, or create one if none exists.
//
{
    Initialize(viewerShell::get_current_vs);

    return get_current_vs(0);
}




extern "C" void viewerShell_set_current_vs(void* vs)
{ viewerShell::set_current_vs(viewerShellPtr(vs)); }


void viewerShell::set_current_vs(viewerShell* vs)	// class static
//
// Set the focused viewerShell.
//
{
    Initialize(viewerShell::set_current_vs);

    // Find the current viewerShell.
    const int top_index = vsh_array.size() - 1;
    viewerShell* top_vs = (viewerShell*)(
	top_index >= 0 ? vsh_array[top_index] : NULL);

    if(vs == NULL)
    {
	vs = top_vs;		// Pretend there was no viewerShell set.
	top_vs = NULL;
    }

    if(vs != top_vs)	// Change the current viewerShell.
    {
//	if(top_vs)
//	    vs_status_line(top_vs->wij, 0);	// Disable current status line.

	if(top_vs = vs)		// Still have a viewerShell
	{
	    vsh_array.remove(top_vs);
	    vsh_array.insert_last(top_vs);

//	    Widget w =
//		vs_status_line(top_vs->wij, 1); // Enable new status line.
	    
	}

	not_focused_viewer(the_focused_viewer);
    } 

    if (top_vs) {
	Widget w = top_vs->wij;
	// Report top_vs
	cmd_record_top_level (w);
    }
}


void viewerShell::not_focused_viewer(viewer* vwr)	// class static
//
// If vwr is the focused viewer, find a different viewer to focus on.
//
{
    Initialize(viewerShell::not_focused_viewer);

    if(vwr == the_focused_viewer)
    {
	viewer* new_current_viewer = NULL;
	if(!vsh_array.empty())
	{
	    viewerShell* vsh =
		(viewerShell*)(vsh_array[vsh_array.size() - 1]);
	    if(vsh->current_viewer >= 0)
		new_current_viewer = vsh->viewers[vsh->current_viewer];
	}
	focused_viewer(new_current_viewer);
    }
}


void viewerShell::focused_viewer(viewer* vwr)		// class static
//
// Set the focused viewer, focused viewHeader, and current viewerShell.
//
{
    Initialize(viewerShell::focused_viewer);

    int vwr_changed = 0;
    int view_changed = 0;
    
    if(the_focused_viewer != vwr)
    {
	if(the_focused_viewer)
	    the_focused_viewer->unset_current();
	if(vwr)
	    set_current_vs(vwr->get_shell());

	the_focused_viewer = vwr;
	vwr_changed = 1;
    }

    viewPtr view_header =
	the_focused_viewer ? the_focused_viewer->get_view_ptr() : NULL;

    if(the_focused_view_header != view_header)
    {
	view_changed = 1;
	the_focused_view_header = view_header;
	Quick_Search::update_search_dialogs(the_focused_view_header);

	if(the_focused_viewer)
	    the_focused_viewer->get_shell()->init_interface(the_focused_viewer);

	steScreenPtr screen = checked_cast(steScreen, get_relation( screen_of_viewer, vwr ));
	if( screen )
	    if (the_focused_view_header == NULL || is_steView(the_focused_view_header))
		ste_set_current_focus( NULL, screen );
    }
    else if(vwr_changed &&the_focused_viewer)
    {
	    the_focused_viewer->get_shell()->init_interface(the_focused_viewer);
    } else if (the_focused_viewer) {
	// If the focused viewer has not actually changed,
	// just force the keyboard focus back onto it.
	the_focused_viewer->set_current();
    }
    
    if (vwr_changed || view_changed)
	cmd_record_viewer_validate (the_focused_viewer);
}


void viewerShell::focused_view(viewer* vwr, viewPtr view_header)	// class static
//
// Set the focused viewer, focused viewHeader, and current viewerShell.
//
{
    Initialize(viewerShell::focused_view);

    if(the_focused_viewer != vwr)
	focused_viewer (vwr);

    if (view_header == 0)
	view_header = the_focused_viewer ? the_focused_viewer->get_view_ptr() : NULL;

    if(the_focused_view_header != view_header)
    {
	the_focused_view_header = view_header;
	Quick_Search::update_search_dialogs(the_focused_view_header);

	if(the_focused_viewer)
	    the_focused_viewer->get_shell()->init_interface(the_focused_viewer);
    }
    else if(the_focused_viewer)
    {
	// If the focused viewer has not actually changed,
	// just force the keyboard focus back onto it.

	the_focused_viewer->set_current();
    }
}

void viewerShell::viewer_changed(viewer* vwr)
//
// If vwr is the current viewer, re-initialize the interface.
//
{
    Initialize(viewerShell::viewer_changed);

    if(current_viewer >= 0 && viewers[current_viewer] == vwr)
	focused_viewer(vwr);
}


void viewerShell::init_interface(viewer* vwr)
//
// Initialize this viewerShell interface appropriately for the specified viewer.
//
{
    Initialize(viewerShell::init_interface);

    for(int i = 0; i < num_viewers; i++)
    {
	viewPtr view_header = viewers[i]->get_view_ptr();
	const int view_num = dlg->get_view_num(view_header);

	if(vwr == viewers[i])
	{
	    current_viewer = i;
	    vwr->set_current();

	    // Attach the (shared) panner to the current view.
	    dlg->register_panner(view_num, panner);
	    vwr->manipulate_perspective(VIEW_UPDATE, 0, 0, 0, 0);
	}
	else
	{
	    viewers[i]->unset_current();

	    // Detach the panner from this view.
	    dlg->register_panner(view_num, NULL);
	}
    }
}

void viewerShell::view_type(viewType type)
//
// Set the viewershells button bar, mode buttons, and pulldown menus
// according to the type of object being viewed.
//
{
    Initialize(viewerShell::view_type);

    if(current_type != type)
    {
	    // Unmanage current button-bar and type-specific menus.
	    if(current_type >= 0)
	    {
		if(buttonBar[current_type])
		    XtUnmanageChild(buttonBar[current_type]);

		int cnt;
		for(cnt = 0; pullMenu[current_type][cnt]; ++cnt) ;
		if(cnt)
		    XtUnmanageChildren(pullMenu[current_type], cnt);
	    }

	    // Manage new button-bar and type-specific menus.
	    if(type >= 0)
	    {
		if(buttonBar[type])
		{
		    XtUnmanageChild(buttonBarSlot);
		    XtManageChild(buttonBar[type]);
		    manage_buttonbar_buttons();
		    XtManageChild(buttonBarSlot);
		}

		int cnt;
		for(cnt = 0; pullMenu[type][cnt]; ++cnt) ;
		if(cnt)
		    XtManageChildren(pullMenu[type], cnt);
	    }

	// Change sensitivity of pull-down menus according to new type.
	set_mode(VE_base);
	vs_set_buttons(wij, type, current_type);

	// Remember new type.
	current_type = type;
    }
}


viewer* viewerShell::create_viewer()
//
// Create a new viewer for this viewerShell.
//
{
    Initialize(viewerShell::create_viewer);
    
    viewer* vwr = NULL;
    if(num_viewers < MAX_VIEWERS)
    {
	vwr = new viewer(this, viewWindow, num_viewers + 1);
	viewers[num_viewers++] = vwr;
    }
    return vwr;
}

viewPtr viewerShell::find_existing_view(appTreePtr app_tree, viewerPtr * out)
{
    viewerPtr vwr   = viewerPtr(0);
    viewPtr ret_val = viewPtr(0);

    if (app_tree) {
        // Suggest target viewer, then fetch the real one.
	vwr = viewer::preferred_viewer();
	if (vwr)
	    ret_val = find_existing_view(vwr, app_tree);
	else {
	    for (int ii = 0 ; ii<MAX_VIEWERS && (ret_val==0); ii++) {
		if ((vwr = viewers[ii]) != 0)
		    ret_val = find_existing_view(vwr, app_tree);
	    }
	}
    }
    if (out)
	*out = (vwr!=0) ? vwr : viewers[0];

    return ret_val;
}

viewPtr viewerShell::find_existing_view(viewer * vwr, appTreePtr app_tree)
{
    Initialize(viewerShell::find_existing_view);

    appPtr app_head;
    if (app_tree == NULL || !(app_head = checked_cast(app,app_tree->get_header())))
	return NULL;

    Obj* ldrtrees = apptree_get_ldrtrees(app_tree);
    viewPtr view_head = vwr ? vwr->get_view_ptr() : NULL;
    ldrPtr ldr_head;

    OperPoint pt(app_tree,FIRST);
    pt.offset_of = 0;
    ldrSelection ls;

    // Check if specified viewer already sees app_tree.
    if(view_head)
    {
	ldr_head = view_get_ldr(view_head);
	if (ldr_head && is_steDisplayHeader(ldr_head))
	{
	    (checked_cast(steDisplayHeader,ldr_head))->find_selection (pt,ls);
	    if (ls.ldr_node)
		return view_head;
	}

	if (ldrtrees)
	{
	    Obj* el1;
	    ForEach(el1, *ldrtrees)
	    {
		ldrTreePtr ldr_tree = checked_cast(ldrTree,(RelationalPtr)(el1));
		ldrPtr ldr_head = ldr_tree->get_header();
		Obj* views = ldr_get_views(ldr_head);
		if(views)
		{
		    Obj *el2;
		    ForEach(el2, *views) {
			if (el2 == view_head)
			    return view_head;
		    }
		}
	    }
	}
    }

    // check Smod Short view first
    Obj *ldrs = app_get_ldrs(app_head);
    Obj *el4;
    ForEach(el4,*ldrs)
    {
	ldr_head = checked_cast(ldr,el4);
	if (ldr_head && is_steDisplayHeader(ldr_head))
	{
	    (checked_cast(steDisplayHeader,ldr_head))->find_selection (pt,ls);
	    if (ls.ldr_node && (view_head = find_view(ldr_head)))
		return view_head;
	}
    }
    // Check any other view in the viewerShell.
    
    if (ldrtrees)
    {
	Obj* el3;
	ForEach(el3, *ldrtrees)
	{
	    ldrTreePtr ldr_tree = checked_cast(ldrTree,(RelationalPtr)(el3));
	    ldrPtr ldr_head = ldr_tree->get_header();
	    if(view_head = find_view(ldr_head))
		return view_head;
	}
    }
    return NULL;
}


void viewerShell::split(viewPtr view_head)
//
// Add a new viewer to the viewerShell layout.
//
{
    Initialize(viewerShell::split);

    viewer* vwr = create_viewer();
    if(vwr)
    {
	vwr->change_view(view_head);
	focused_viewer(vwr);
    }
}


void viewerShell::unsplit()
//
// Remove a viewer from the viewerShell layout.
//
{
    Initialize(viewerShell::unsplit);

    if(num_viewers > 0)
    {
	viewer*& old_viewer = viewers[--num_viewers];

	ste_interface_raw_finalize (old_viewer->get_view_ptr());

	not_focused_viewer(old_viewer);

	if(current_viewer >= num_viewers)
	    current_viewer = num_viewers - 1;

	delete old_viewer;
	old_viewer = NULL;
    }
}


viewer* viewerShell::available_viewer()
//
// Find or create a viewer and return it.
//
{
    Initialize(viewerShell::available_viewer);

    if(num_viewers <= 0)
	return create_viewer();
    else
	return viewers[max_int(0, current_viewer)];
}


viewer* viewerShell::find_target_viewer()		// class static
//
// Return a viewer suitable for a new view, creating a viewer
// and/or viewerShell as needed.
//
{
    Initialize(viewerShell::find_target_viewer);

    viewerShell* vsh = NULL;
    const int num_viewer_shells = vsh_array.size();

    if (isServer())
	return NULL;

    // If there are no viewerShells, or only the helper-shell, make a new one.
    if(num_viewer_shells <= 1)
    {
	if(num_viewer_shells == 0 ){
	    vsh = new viewerShell(CREATE_MINIBROWSER);
	    view_set_target_viewer(vsh->create_viewer()); 
	    set_current_vs(vsh);
	    vsh->bring_to_top();
	    vsh->map();
	}
	else
	    vsh = (viewerShell*)vsh_array[0];
    }
    // Otherwise, get the most recently used viewerShell.
    else
    {
	vsh = (viewerShell*)vsh_array[num_viewer_shells - 1];
    }

    view_flush_on_top();
    return vsh->available_viewer();
}


viewPtr viewerShell::find_duplicate_view(viewPtr view_head)
//
// Returns first existing view of the same ldr in this viewerShell.
//
{
    Initialize (viewerShell::find_duplicate_view);

    ldrPtr ldr_to_match = view_head->get_ldrHeader();
    for(int i = 0; i < num_viewers; i++)
    {
	if(viewers[i])
	{
	    viewPtr other_view_head = viewers[i]->get_view_ptr();
	    if(other_view_head  &&  other_view_head != view_head  &&
	       other_view_head->get_ldrHeader() == ldr_to_match)
		return other_view_head;
	}
    }
    return NULL;
}


viewPtr viewerShell::duplicate_view(viewPtr old_view)
//
// Create and return a new view of the same ldr as the given view.
//
{
    Initialize(viewerShell::duplicate_view);

    viewPtr dup_view = NULL;
    if(old_view)
    {
//	if(is_steShellView(old_view) || is_steView(old_view) || is_steRawView(old_view))
	if(is_steShellView(old_view))
	    dup_view = old_view;   // Use the same view for debugger and text views
	else{
	    ldrPtr ldr_head = old_view->get_ldrHeader();
	    const int view_index = dlg->empty_view();
	    if(ldr_head  &&  (ldr_head->get_id() > 0) && view_index >= 0)
	    {
		dup_view =
		    dlg->open_view(ldr_head, repType(old_view->get_type()), 0);
		dlg->set_view(view_index, dup_view);
	    }
	}
    }
    return dup_view;
}


int viewerShell::view_in_use(viewPtr test_view)
//
// Return true iff test_view is currently in use in a viewer.
//
{
    Initialize(viewerShell::view_in_use);

    for(int i = 0; i < num_viewers; i++)
	if(viewers[i]  &&  viewers[i]->get_view_ptr() == test_view)
	    return (i+1);	// Found it.

    // If we get to this point, the view is not being used.
    return 0;
}

static void really_close_viewer_shell(void* ViewerShell)
{
    vs_destroy ((Widget)ViewerShell);
}

extern "C" void viewerShell_close_viewer_shell(void* ViewerShell, viewerShell* vsh)
// Function returns 1 if viewer closed & -1 if operation was canceled    
{
    Initialize (viewerShell_close_viewer_shell);

    objArr saveable_raw_apps;
    RTLNodePtr rtlnode    = checked_cast(RTLNode, vsh->get_rtl()->get_root());
    symbolArr& view_array = rtlnode->rtl_contents();
    int        sz         = view_array.size();

    for(int i = 0 ; i < sz ; i++) {
        viewPtr viewptr = checked_cast(view,view_array[i]);
	if (viewptr) {
	    appPtr  appptr = viewptr->get_appHeader();
	    appType type   = (appType)appptr->get_type();
	    if (appptr  && appptr->is_modified() && type == App_RAW)
		saveable_raw_apps.insert_last (appptr);
	}
    }

    gtPushButton::next_help_context ("Viewer.File.CloseWindow");
    offer_to_save_list_wait(NULL, Flavor_ViewerClosing, &saveable_raw_apps,
                 really_close_viewer_shell, ViewerShell, 1);
}

static void print_wij_tree (Widget w, const char *txt)
{
    msg("Widget tree") << eom;
    if (txt && txt[0])
	msg(" for $1") << txt << eom;

    msg("") << eom;
    Widget tmp = w;
    while (tmp) {
	fprintf (stderr, "  w = %d\n", tmp);
	tmp = XtParent(tmp);
    }
    msg("") << eom;
}

static viewerShell *viewerShell_get_vs_by_widget (Widget w)
{
    viewerShell *ret = NULL;
    viewerShell *vsh = viewerShell::get_current_vs(1);
    if (w && vsh) {
	if (vsh->get_widget() == w)
	    ret = vsh;
	else {
	    int idx = 0;
	    while ( (vsh = viewerShell::get_vs_by_idx(idx)) ) {
		if (vsh->get_widget() == w) {
		    ret = vsh;
		    break;
		}

		idx ++ ;
	    }
	}
    }
    return ret;
}

extern "C" void viewerShell_close_button_callback (Widget w, XtPointer client_data, XtPointer call_data)
{
    if (!w || !client_data)
	return;

    Widget vsh_wij   = (Widget)client_data;
    viewerShell *vsh = viewerShell_get_vs_by_widget (vsh_wij);
    if (vsh) {
	viewerShell::pre_hook (vsh);
	viewerShell_close_viewer_shell (vsh_wij, vsh);
	viewerShell::post_hook();
    }
}

extern "C" void viewerShell_close_current_view(viewerShell* vsh)
{ vsh->close_current_view(); }


void viewerShell::close_current_view()
//
// Delete the view in the current viewer.
//
{
    Initialize(viewerShell::close_current_view);

    if(current_viewer >= 0) {
        // Deleting the viewHeader eventually results in a call to
        // detach_hook(), see below.
        viewPtr viewptr = viewers[current_viewer]->get_view_ptr();
        appPtr appptr = viewptr->get_appHeader();
	if (!appptr)
	    return;
        if (is_objRawApp(appptr) && appptr->is_modified()) {
            gtPushButton::next_help_context ("Viewer.File.CloseView");
            int answer = popup_Question(TXT("Close Current View"),
                TXT("If you close this view, you will lose your changes"),
                   TXT("Save then Close"), TXT("Close Anyway"), TXT("Cancel"));
            switch (answer) {
            case 1:
                if (db_save_or_cancel(appptr, NULL,1))
                    obj_delete (viewptr);
                break;
            case 2:
                obj_delete (viewptr);
                break;
            default:
                break;
            }
        } else {

	    //-----------------------------------------------
	    // *** JJS 6/12/95 ***
	    // Fix for 10151.  When closing a call tree view,
	    // delete the LDR as well.  This way, if the view
	    // is opened again, a new one will be created.
	    //-----------------------------------------------

	    if (is_viewTreeHeader (viewptr)) {
		ldr* ldrptr = viewptr->get_ldrHeader();
		if (ldrptr != NULL)
		    obj_delete (ldrptr);
		else obj_delete (viewptr);
	    }
            else obj_delete(viewptr);
	}
    }         
}


void viewerShell::detach_hook(void* vsh_data, int view_num) // class static
//
// Notify function from dialog telling us to handle the impending
// deletion of a view.
//
{
    Initialize(viewerShell::detach_hook);
    viewerShell* vsh = (viewerShell*)vsh_data;
 
    // If one of the viewers is looking at this view,
    // find a different view to put in this viewer, or close it.
    viewPtr view_header = vsh->dlg->get_view_ptr(view_num);
//    viewer* vwr = vsh->viewer_of_view(view_header);
    // Code changed to support the same view in the Multiple viewers
    // close this view in all viewers.
    for(int v_index = vsh->num_viewers - 1; v_index >= 0; v_index--){
        viewer *vwr = vsh->viewers[v_index];
        if(vwr && vwr->get_view_ptr() == view_header) {
	    // Preserv old target_viewer and preferred viewer
	    viewer *old_vwr = view_set_target_viewer (vwr);
	    viewer *prf_vwr = viewer::preferred_viewer ();
	    set_preferred_viewer ((viewer *)NULL);
	    
            vwr->change_view_from_history (-1, 1);
            viewPtr switch_view = vwr->get_view_ptr();
 
            if (!switch_view || switch_view == view_header) {
                switch_view = NULL;
                // Find the first un-used view.
                dialogPtr dlg = vsh->dlg;
                int i;
 
                for(i = dlg->first_view(); i >= 0; i = dlg->next_view(i)) {

                    if(i != view_num) {
                        viewPtr viewp = dlg->get_view_ptr(i);
                        if(!vsh->view_in_use(viewp)) {
                            switch_view = viewp;
                            break;
                        }
                    }
                }
                vwr->change_view(switch_view, 1);
            }
	    // Restore target_viewer and prefered viewer
	    (void)view_set_target_viewer (old_vwr);
	    set_preferred_viewer (prf_vwr);
        }
    }
}

void viewerShell::remove_view_from_menu(viewPtr view_head)
//
// Remove a view from the list-of-views RTL,
// replacing it with a duplicate if possible.
//
{
    Initialize(viewerShell::remove_view_from_menu);

    RTLNodePtr rtl_root = checked_cast(RTLNode, rtl->get_root());
    if(rtl_root->rtl_includes(view_head))
    {
	// If duplicate view is visible, add that into rtl.
	viewPtr dup_view_head = find_duplicate_view(view_head);
	if(dup_view_head)
	{
	    dup_view_head->set_name(view_head->get_name());
	    rtl_add_obj(rtl_root, dup_view_head);
	}
	rtl_delete_obj(rtl_root, view_head);
    }
}


void viewerShell::add_view_to_menu(viewPtr viewp)
//
// Add a view to the list-of-views RTL,
// but only if the RTL does not already contain a duplicate of the view.
//
{
    Initialize (viewerShell::add_view_to_menu);

    if(viewp == NULL  ||  viewp != find_view(viewp))
	return;

    // Make the name unique
    RTLNodePtr rtlnode = checked_cast(RTLNode, rtl->get_root());
    symbolArr& view_array = rtlnode->rtl_contents();

    if(!view_array.includes(viewp))
    {
	const char* view_name = viewp->get_name();
	int new_num = 0;
	genString new_name = view_name;
	int found = (view_array.size() > 0);

	while(found)
	{
	    ObjPtr el;
	    ForEachS(el, view_array)
	    {
		viewPtr v = checked_cast(view, el);
		if(found = (new_name == v->get_name()))
		{
		    new_name.printf("%s<%d>", view_name, ++new_num);
		    break;
		}
	    }
	}
	add_view_type_to_name(viewp, new_name);
	viewp->set_name(new_name);
	rtl_add_obj(rtlnode, viewp);
    }
    view_create_flush();
}

void add_view_type_to_name(view *v, genString& name)
{
    Initialize(add_view_type_to_name);
    
    if(cmd_execute_journal_name){
	if(!cmd_available_p(cmdViewName))
	    return;
    }
    
    genString tmp_name;
    switch(v->get_type()){
      case Rep_RawText:
	tmp_name += "File: ";
	break;
      case Rep_SmtText:
	tmp_name += "Text: ";
	break;
      case Rep_VertHierarchy:
	tmp_name += "Outline: ";
	break;
      case Rep_FlowChart:
	tmp_name += "Flowchart: ";
	break;
      case Rep_Tree:
	tmp_name += "Tree: ";
	break;
    }
    tmp_name += name;
    name      = tmp_name;
}

viewPtr viewerShell::open_view(
    appPtr app_head, repType type, appTreePtr app_node)
//
// Open a new view in the current viewer.
//
{
    Initialize (viewerShell::open_view);

    viewPtr view_head = NULL;

    if(app_node == NULL  &&  app_head != NULL)
	app_node = checked_cast(appTree, app_head->get_root());

    if(app_node)
    {
	view_set_target_viewer(available_viewer());
	view_head = view_create(app_node, type);
	view_create_flush();
    }
    return view_head;
}


viewPtr viewerShell::open_view(repType type, symbolPtr symbol)
//
// Open a new view in the current viewer.
//
{
    Initialize (viewerShell::open_view);

    view_set_target_viewer(available_viewer());
    viewPtr view_head = view_create(symbol, type);
    view_create_flush();

    return view_head;
}

viewPtr viewerShell::find_view(int i)
{
    if (i < 0 || i >= num_viewers)
        return NULL;
    return viewers[i]->get_view_ptr();
}

void viewerShell::bring_to_top()
//
// Bring the viewerShell to the top of the window heap.
//
{
    Initialize(viewerShell::bring_to_top);

    unsigned int mask = CWStackMode;

    XWindowChanges changes;
    changes.stack_mode = TopIf;

    XReconfigureWMWindow(XtDisplay(wij), XtWindow(wij),
			 DefaultScreen(XtDisplay(wij)),
			 mask, &changes);

    gtShell_set_focus(wij);
}


//------------------------------------------

dialogPtr viewerShell::getDialog()
{
    Initialize(viewerShell::getDialog);

    return dlg;
}

//------------------------------------------

viewPtr viewerShell::request_view(viewPtr test_view)
//
// Check to see if a requested view is already in use. If not, return it.
// Otherwise, open a new view of the same thing, and return the new view.
//
{
    Initialize(viewerShell::request_view);

    return view_in_use(test_view) ? duplicate_view(test_view) : test_view;
}


extern "C" viewPtr viewerShell_get_current_view(viewerShell* vs)
{ return vs->get_current_view(); }


viewPtr viewerShell::get_current_view()
//
// Return current viewHeader of this viewerShell, or NULL.
//
{
    Initialize(viewerShell::get_current_view);

    if(current_viewer >= 0)
	if(viewers[current_viewer])
	    return viewers[current_viewer]->get_view_ptr();

    return  NULL;
}

extern "C" int can_collapse_explode(viewerShell* vsh)
//
// Return true if vsh permits collapse/explode on the current view
//
{
    Initialize(can_collapse_explode);
    viewPtr v = (vsh) ? vsh->get_current_view() : NULL;

    //boris: disable collapse/explode on all ste views
    return (v == 0 || is_steView(v)) ? 0 : 1;
}

extern "C" int is_flowchart_view(viewerShell* vsh)
//
// Return true if vsh will permit collapse/explode.
//
{
    Initialize(is_flowchart_view);

    viewPtr v = (vsh) ? vsh->get_current_view() : NULL;
    if (v && is_viewFlowchartHeader(v))
	return 1;

    return 0;
}

extern "C" int can_refresh_view(viewerShell* /* vsh */ )
//
// Return true if view is ERD, Data Chart, or Subsystem
//
{
    Initialize(can_refresh_view);

    int vt = gra_type();
    if (vt == Rep_ERD || vt == Rep_DataChart || vt == Rep_SubsystemMap)
        return 1;

    return 0;
}


extern "C" int can_manage_file( viewerShell* )
// Return true if this file is in project
//
// vsh is the object for the viewerShell where the manage button is pressed
// always returns true for list views
{
    Initialize(can_manage_file);

    if (customize::no_cm())
        return 0;
    return 1;
}
 
extern "C" int edit_menu_can_edit(viewerShell* vsh, int *vi_mode_sensitive, int *vi_mode_state, int *read_only)
/*
 * Return true if cView is not zero
 * vi_mode_sensitive and vi_mode_state are returned for
 * steViews depended on steView smod_vision flag.
 * if the view is structured text, disable VI mode
 * read_only flag = 0   if view is writable
 *                  1   if app is read_only
 *                  2   if view is a graphic view (not steView)
 *                  3   if project is not writable
 *                  4   if view is read_only by some wierd reason
 *                  -1  if view is RAW view
 */
{
    Initialize(edit_menu_can_edit);
    viewPtr v = (vsh) ? vsh->get_current_view() : NULL;
    if (v == NULL || !view_get_ldr(v))
        return 0;
 
    if (is_steView(v)) {
	if(v->get_type() == Rep_TextText) { // structured text: no VI mode allowed
	    *vi_mode_sensitive = False;
	    *vi_mode_state = False;
	}
	else {
	    *vi_mode_sensitive = True;
	    *vi_mode_state = v->smod_vision;
	}
    }
 
    appTreePtr sub_rt = NULL;
    appPtr ah = NULL;
   
    ldrPtr lh = view_get_ldr (v);
    if (lh) {
        ldrTreePtr lroot = checked_cast(ldrTree,lh->get_root());
        sub_rt = (lroot) ? lroot->get_appTree() : NULL;
        ah = (lh) ? ldr_get_app (lh) : NULL;
    }
 
    *read_only = 0;
 
    if (ah && ah->get_type() == App_RAW)
        *read_only = -1;
    if (ah && ah->read_only)
        *read_only = 1;
    else if ( !is_steView(v) )
        *read_only = 2;
    else if (ah) {
        projNode *prj = appHeader_get_projectNode(ah);
        if (prj && prj->get_xref() && !prj->get_xref()->is_writable())
            *read_only = 3;
    } else if (v->read_only)
        *read_only = 4;
 
    return 1;
}
 
//
//Guy: I added this function so that when opening a null view, you can
//set the sensitivity of the menu options accordingly.
//
extern "C" int viewerShell_Is_viewer_null(viewerShell* vsh)
//
// Return true if cView is not zero
// This function is needed to set the sensitivity of the file menu options
// correctly when a new window (viewer) is created
{
    Initialize(viewerShell_Is_viewer_null);
    viewPtr v = (vsh) ? vsh->get_current_view() : NULL;
    if (v == NULL || !view_get_ldr(v))
      return 0;
    else
      return 1;
 
}
 
extern "C" int can_add_viewer(viewerShell* vsh)
//
// Return true if vsh will permit another viewer to be added to it.
//
{
    Initialize(can_add_viewer);

    return vsh->get_num_viewers() < viewerShell::MAX_VIEWERS;
}

extern "C" int can_remove_viewer(viewerShell* vsh)
//
// Return true if vsh will permit a viewer to be removed from it.
//
{
    Initialize(can_remove_viewer);

    return vsh->get_num_viewers() > 1;
}

extern "C" void vsAddViewer(viewerShell* vsh)
{
    Initialize(vsAddViewer);

    vsh->split();
}

extern "C" void vsRemoveViewer(viewerShell* vsh)
{
    Initialize(vsRemoveViewer);

    vsh->unsplit();
}

//------------------------------------------
// open the top item in the selection stack

extern "C" void viewerShell_openSelected(viewerShell* vsh)
{
   Initialize(viewerShell_openSelected);

   viewPtr vh = vsh->get_current_view();
   if(vh)
   {
       driver_instance->update_selection(vh, 1);
       symbolArr array;
       driver_instance->fill_array_with_selected_appnodes(NULL, array, 1);
       
       const int selected = array.size();
       if(selected)
       {
	   symbolPtr sym = array[0];
	   if(sym.relationalp()){
	       Relational *ob = sym;
	       if(is_appTree(ob)){
		   appTree * node = (appTree *)ob;
		   appTree * prev = node;
		   while(node->get_parent()){
		       prev = (appTree *)node;
		       node = (appTree *)node->get_parent();
		   }
		   sym = prev;
		   create_view_of_def(sym);
	       }
	   }
       }
       else
       {
	   OperPoint* pt = vh->insertion_point();
	   if(pt  &&  !pt->node.isnull()){
	       symbolPtr sym = pt->node;
//	       create_view_of_def(sym);
	   }
	   delete pt;
       }
       view_create_flush();
   }
}

//------------------------------------------
// Expand view to whole file.

extern "C" void viewerShell_ExpandView(viewerShell* vsh)
{
    Initialize(viewerShell_ExpandView);
    
    viewPtr vh = vsh->get_current_view();
    if(vh)
    {
	symbolArr array;
	switch(repType(vh->get_type()))
	{
	  case Rep_VertHierarchy:		// Operate on app of view.
	  case Rep_FlowChart:
	  case Rep_TextDiagram:
	  case Rep_TextText:
	  case Rep_TextFullDiagram:
	  case Rep_SmtText:
	  case Rep_RawText:
	  {
	      ldrPtr ldr_head = view_get_ldr(vh);
	      if(ldr_head){
		  appPtr app_head = ldr_get_app(ldr_head);
		  if(app_head)
		      array.insert_last(app_head);
	      }
	  }
	  break;
	  default:
	  break;
        }
	if(array.size() != 0){
	    openFromBrowser create(NULL, array, repType(vh->get_type()));
	    create.start();
	} else 
	    msg("This view could not be expanded.", warning_sev) << eom;
    }
}

extern "C" void viewerShell_openDefinition(viewerShell* vsh)
{
    Initialize(viewerShell_openDefinition);

    int found = 0;
    int selected = 0;

    start_transaction() {
	viewPtr vh = vsh->get_current_view();
	if(vh)
	    driver_instance->update_selection(vh, 0);
	
	symbolArr array;
	driver_instance->fill_array_with_selected_appnodes(NULL, array, 1);
	
	selected = array.size();

	if (selected) {

	    for(int i = 0; i < selected; i++)
		found += create_view_of_def((symbolPtr)array[i]);

	} else if (vh) {
	    OperPoint* pt = vh->insertion_point();
	    if(pt  &&  !pt->node.isnull()) {
		found += create_view_of_def(pt->node);
		delete pt;
	    }
	}

	if (found)
	    view_create_flush();

    } end_transaction();

    gtPushButton::next_help_context ("Viewer.Help.OpenDefinition");

    if (found == 0 && selected == 0)
	msg("Could not open definition for the current cursor position.", error_sev) << eom;
    else if (found == 0)
	msg("Could not open definition for the current selection(s).", error_sev) << eom;
    else if (found < selected)
	msg("Could not open definition for some of the selections.", warning_sev) << eom;
}

projModule *get_module_from_view(viewPtr view)
{
    Initialize(get_module_from_view);
    
    if(!view)
	return NULL;
    switch(repType(view->get_type()))
    {
      case Rep_VertHierarchy:		// Operate on app of view.
      case Rep_FlowChart:
      case Rep_TextDiagram:
      case Rep_TextText:
      case Rep_TextFullDiagram:
      case Rep_SmtText:
      case Rep_RawText:
      {
	  ldrPtr ldr_head = view_get_ldr(view);
	  if(ldr_head){
	      appPtr app_head = ldr_get_app(ldr_head);
	      if(app_head)
		  return app_head->get_module();
	  }
      }
         break;
      default:
      break;
    }
    return NULL;
}

extern void unload_selection(symbolArr&); // Defined in browser_callbacks.C

extern "C" void viewerShell_file_unload(viewerShell *vsh)
{
    viewPtr view = vsh->get_current_view();
    if(view)
	driver_instance->update_selection(view, 0);
    symbolArr array;
    driver_instance->fill_array_with_selected_appnodes(NULL, array, 0);
    if(array.size() == 0 && view){
	projModule *m = get_module_from_view(view);
	if(m)
	    array.insert_last(m);
    }
    unload_selection(array);
    driver_instance->unselect((viewPtr)NULL);
}

extern void update_selection(symbolArr&); // Defined in browser_callbacks.C

extern "C" void viewerShell_file_update(viewerShell *vsh)
{
    Initialize(viewerShell_file_update);
    
    viewPtr view = vsh->get_current_view();
    if(view)
	driver_instance->update_selection(view, 0);
    symbolArr array;
    driver_instance->fill_array_with_selected_appnodes(NULL, array, 0);
    update_selection(array);
    driver_instance->unselect((viewPtr)NULL);
}

extern void preload_selection(symbolArr&); // Defined in browser_callbacks.C

extern "C" void viewerShell_file_load(viewerShell *vsh)
{
    Initialize(viewerShell_file_load);
    
    viewPtr view = vsh->get_current_view();
    if(view)
	driver_instance->update_selection(view, 0);
    symbolArr array;
    driver_instance->fill_array_with_selected_appnodes(NULL, array, 0);
    if(array.size() == 0 && view){
	projModule *m = get_module_from_view(view);
	if(m)
	    array.insert_last(m);
    }
    preload_selection(array);
}

extern void language_selection(symbolArr&); // Defined in browser_callbacks.C

extern "C" void viewerShell_file_language(viewerShell *vsh)
{
    Initialize(viewerShell_file_language);
    
    viewPtr view = vsh->get_current_view();
    if(view)
	driver_instance->update_selection(view, 0);
    symbolArr array;
    driver_instance->fill_array_with_selected_appnodes(NULL, array, 0);
    if(array.size() == 0 && view){
	projModule *m = get_module_from_view(view);
	if(m)
	    array.insert_last(m);
    }
    language_selection(array);
}

extern void setup_home_from_selection(symbolArr&); // Defined in util/src/set_homeproj.C
void regenerate_rtl_and_refresh_views(RTL *rtl); // defined in view/rtl

extern "C" void viewerShell_manage_home_project(viewerShell *vsh)
{
    Initialize(viewerShell_manage_home_project);
    
    viewPtr view = vsh->get_current_view();
    if(view)
	driver_instance->update_selection(view, 0);
    symbolArr array;
    driver_instance->fill_array_with_selected_appnodes(NULL, array, 0);
    setup_home_from_selection(array);
    RTLPtr rtl = projNode::get_project_rtl();
    regenerate_rtl_and_refresh_views(rtl);
}

//------------------------------------------

symbolArr *viewerShell_to_unload = NULL;

static void viewerShell_do_put(void* data)
{
    Initialize(viewerShell_do_put);

    driver_instance->unselect((viewPtr)NULL);
    symbolArr& array = *(symbolArr*)data;
    putUI* dialog = putUI::load_putUI(array);
}

extern projModule *projModule_of_symbol(symbolPtr&);
extern void unlock_module_arr(symbolArr &modules);

void viewerShell_manage_internal(viewerShell* vsh, viewerShell_MANAGE op,
				 char *make_target)
// the routine to handle the pull down menu for manage in viewerShell
// this general-purpose routine is called by all the callback functions
{
    Initialize(viewerShell_manage_internal);

    viewPtr view_head = vsh->get_current_view();

    // Collect app-header(s) for the current view, or the current selection
    // (depending on the type of the view), into "array".

    symbolArr array;
    symbolArr proj_array; // used for make/update makefile options
    int ask_flag ;

    int selected_from_miniBrowser = 0;
    {
	symbolArr sym_array;
	
	driver_instance->fill_array_with_selected_appnodes(
	    NULL, sym_array, 0);
	symbolPtr el;
	ForEachS(el,sym_array) {
	    symbolPtr  xsym = el.get_xrefSymbol();
	    if(xsym.xrisnull()) {
		if(is_projModule(el))
		    array.insert_last(el);
		else if(is_RTL(el))
		    proj_array.insert_last(el);
	    }
	    else if(xsym.get_kind() == DD_MODULE){
		projModule *mod = projModule_of_symbol(xsym);
		if(mod)
		    array.insert_last(mod);
	    }
	    else if(xsym.get_kind() == DD_PROJECT)
		proj_array.insert_last(el);
	}
	if(array.size() || proj_array.size())
	    selected_from_miniBrowser = 1;
    }   

    if(!selected_from_miniBrowser){
	if(view_head == 0)
	    return;
	switch(repType(view_head->get_type()))
	{
	  case Rep_VertHierarchy:		// Operate on app of view.
	  case Rep_FlowChart:
	  case Rep_TextDiagram:
	  case Rep_TextText:
	  case Rep_TextFullDiagram:
	  case Rep_SmtText:
	  case Rep_RawText:
	      {
		  ldrPtr ldr_head = view_get_ldr(view_head);
		  if(ldr_head){
		      appPtr app_head = ldr_get_app(ldr_head);
		      if(app_head && app_head->get_module())
			  array.insert_last(app_head->get_module());
		  }
	      }
	      break;

	  case Rep_MultiConnection:		// Operate on app of selection.
	  case Rep_Tree:
	  case Rep_ERD:
	  case Rep_OODT_Inheritance:
	  case Rep_OODT_Relations:
	  case Rep_OODT_Scope:
	  case Rep_DataChart:
	  case Rep_SubsystemMap:
	  {
	      driver_instance->update_selection(view_head, 0);

	      objArr app_tree_array;
	      driver_instance->fill_array_with_selected_appnodes(
		  NULL, app_tree_array, 1);
	      const int num = app_tree_array.size();
	      if(num == 0){
		  OperPoint* pt = view_head->insertion_point();
		  if(pt  &&  !pt->node.isnull())
		      app_tree_array.insert_last(checked_cast(appTree,pt->node));
		  delete pt;
	      }
	      for(int i = 0; i < num; i++){
		  appTreePtr app_tree = checked_cast(appTree,app_tree_array[i]);
		  if(app_tree  &&  is_smtTree(app_tree))
		      app_tree = dd_get_token_def(smtTreePtr(app_tree));
		  if(app_tree){
		      appPtr app_head = app_tree->get_header();
		      projModule *mod = checked_cast(projModule, app_head->get_module());
		      if(mod && !array.includes(mod)){
			  array.insert_last(mod);
		      }
		  }
	      }
	  }
	      break;

	    default:			// Ignore following repTypes.
	      break;
	  }
    }  // selected_from_miniBrowser

    genString fn;
    projModulePtr dst_mod = NULL;
    
    if(array.size() == 0 && op != MANAGE_UPDATE_MAKEFILE && op != MANAGE_MAKE){
	msg("No objects found to perform operation.", warning_sev) << eom;
	return;
    }
    
    OperPoint *o_point = NULL;
    if ( !selected_from_miniBrowser && is_steView( view_head ) )
	// Save the focus point for ste view
	o_point = checked_cast(steView, view_head)->raw_insertion_point();

    bool yes_to_all = false;
    if (op == MANAGE_PUT){
      if(viewerShell_to_unload == NULL)
          viewerShell_to_unload = new symbolArr;
      else
          viewerShell_to_unload->removeAll();
      objArr    app_headers;
      for(int i = array.size() - 1; i >= 0; i--){
          projModule *mod = checked_cast(projModule,array[i]);
          viewerShell_to_unload->insert_last(mod);
          if(mod->get_app())
              app_headers.insert_last(mod->get_app());
      }
      offer_to_save_list(NULL, Flavor_Putting, &app_headers, viewerShell_do_put,
                         viewerShell_to_unload);
    }
    else for(int i = 0; i < array.size(); ++i)
    {
	// on the app-headers gathered above.
	projModulePtr module = checked_cast(projModule,array[i]);
	appPtr old_app_head  = module->find_app();
	appPtr app_head      = NULL;
	if(module) switch(op)
	{
	  case MANAGE_GET:
          case MANAGE_GET2:
	    if (op == MANAGE_GET2) {
		Prompt prompt(NULL, TXT("Get Options"),
		  TXT("Enter options for Get command:"));

            	genString name(customize::configurator_get());
            	if(prompt.ask(name) <= 0 ) 
			break;
	    	else
                {
		    if (strcmp((char *)name, customize::configurator_get()) != 0)
            	        customize::configurator_get((char *) name);
		    customize::save_configurator(NULL);
            	} 
            }
	    dst_mod = NULL;
	    {
		symbolPtr sm = module;
		if (op == MANAGE_GET &&
		    customize::configurator_get_comments() && 
		    (!strcmp(customize::configurator_system(), "clearcase") || 
		     !strcmp(customize::configurator_system(), "continuus") || 
		     !strcmp(customize::configurator_system(), "custom"))) {
		    
		    getUI dlg;
		    genString comm;
		    if (dlg.ask(comm) <= 0) break;
		    genString tnm_for_comment = create_one_tmp_file(comm);
		    dst_mod = projModule::get_module_with_data(sm, (void*)tnm_for_comment.str());
		    OSapi_unlink(tnm_for_comment);
		} else
		  dst_mod = projModule::get_module(sm);
		if (dst_mod) { // got it successfully
		    dst_mod->get_phys_filename (fn);
		    app_head = get_create_app_by_name (fn);
		    if (app_head) {
			viewPtr new_view = view_create (app_head);
			if ( is_steView( new_view ) && o_point )
			    checked_cast(steView,new_view)->insertion_point(o_point);
			view_create_flush();
		    }
		}
		break;
	    }
	  case MANAGE_PUT:
	    break;

	  case MANAGE_COPY:  
{
		symbolPtr sm = module;
		if (dst_mod = projModule::copy_module(sm)) {
		    dst_mod->get_phys_filename (fn);
		    app_head = get_create_app_by_name (fn);
		    if (app_head) {
			viewPtr new_view = view_create (app_head);
			if ( is_steView( new_view ) && o_point )
			    checked_cast(steView,new_view)->insertion_point(o_point);
			view_create_flush();
		    }
		}
		break;
            }
          case MANAGE_DIFF:
            module->get_phys_filename (fn);
	    if((old_app_head && !view_head) || (old_app_head && view_head )) {
		ask_flag = old_app_head->is_src_modified();
		if (!ask_flag) {
		    appPtr raw_app;
		    if (raw_app = app::get_header(App_RAW, fn))
			ask_flag = raw_app->is_modified();
		}
		if (ask_flag){
		    genString buffer;
		    buffer.printf(TXT("This file (%s) is modified. Do you want to save it before doing the diff?"),fn.str());
		    int diff_flag = 1;
		    gtPushButton::next_help_context("Browser.Manage.Diff.Help");
		    diff_flag = popup_Question(TXT("Diff Module"), (char *)buffer,
					       TXT("Save then Diff"), TXT("Diff without Save"), TXT("Cancel"));
		    if (diff_flag != 1 && diff_flag != 2)
			break;
		    if (diff_flag == 1)
			::db_save(old_app_head, 0);
		}
	    }
            module->diff_module(module->get_project());
            break;
	    
	  case MANAGE_LOCK:
	    module->lock_module();
	    break;

	  case MANAGE_UNLOCK:
            unlock_module_arr(array); // this routine will be for all modules in the array
            i = array.size(); // update i to the last index in the array to force it out of loop
	    break;

          case MANAGE_UNGET:
            yes_to_all = browser_unget_module(0, module, yes_to_all);
            break;

	  case MANAGE_UPDATE_MAKEFILE:
	  case MANAGE_MAKE:
	    break; // these cases handled above
	}
    }
    configurator_to_icon();
}

void viewerShell_manage(viewerShell* vsh, viewerShell_MANAGE op)
// callback for entries of the manage menu other than manage->make 
{
    viewerShell_manage_internal(vsh, op, NULL);
}


//------------------------------------------
// BASIC EDITING FUNCTIONS
//------------------------------------------


extern "C" void viewerShell_cut(viewerShell* vs)
{ vs->cut(); }

void viewerShell::cut()
{
    Initialize(viewerShell::cut);

    if (current_viewer >= 0)
	viewers[current_viewer]->cut();
}

//------------------------------------------

extern "C" void viewerShell_paste(viewerShell* vs)
{ vs->_paste(); }

void viewerShell::_paste()
{
    Initialize(viewerShell::_paste);

    if (current_viewer >= 0)
	viewers[current_viewer]->_paste();
}

//------------------------------------------

extern "C" void viewerShell_copy(viewerShell* vs)
{ vs->copy(); }

void viewerShell::copy()
{
    Initialize(viewerShell::copy);

    if (current_viewer >= 0)
	viewers[current_viewer]->copy();
}

//------------------------------------------

extern "C" void viewerShell_reference(viewerShell* vs)
{ vs->reference(); }

void viewerShell::reference()
{
    Initialize(viewerShell::reference);

    if (current_viewer >= 0)
	viewers[current_viewer]->reference();
}

//------------------------------------------

extern "C" void viewerShell_manipulate_perspective(
    viewerShell* vs, int op, int i1, int i2, int i3, int i4)
{ vs->manipulate_perspective(op, i1, i2, i3, i4); }

void viewerShell::manipulate_perspective(
    int op, int i1, int i2, int i3, int i4 )
{
    Initialize (viewerShell::manipulate_perspective);

    viewer* v = (current_viewer < 0 ? NULL : viewers[current_viewer]);
    if(v)
	v->manipulate_perspective(op, i1, i2, i3, i4);
}

//------------------------------------------

extern "C" void viewerShell_panner_callback(
    void*, viewerShell* vs, int op, int horz, int vert)
{ vs->panner_callback(op, horz, vert); }

void viewerShell::panner_callback(int op, int curx, int cury)
{
    Initialize(viewerShell::panner_callback);

    viewer* v = (current_viewer < 0 ? NULL : viewers[current_viewer]);
    if(v)
    {
	int x0, y0, w, h, cx, cy, cw, ch;
	Panner_values(panner, &x0, &y0, &w, &h, &cx, &cy, &cw, &ch);

	switch(op)
	{
	  case VIEW_PAN_XY:
	    if(curx < x0) curx = x0;
	    else if(curx > x0 + w - cw) curx = x0 + w - cw;

	    if(cury < y0) cury = y0;
	    else if(cury > y0 + h - ch) cury = y0 + h - ch;

	    v->manipulate_perspective(
		VIEW_PAN_XY, curx, y0 + h - cury - ch, 0, 0);
	    break;
	}
    }
}

//------------------------------------------

extern "C" void viewerShell_explode_current(viewerShell* vs, int level)
{ vs->explode_current(level); }

void viewerShell::explode_current(int level)
{
    Initialize(viewerShell::explode_current);

    if(current_viewer >= 0)
    {	
	viewPtr view_head = viewers[current_viewer]->get_view_ptr();
	if(is_viewGraHeader(view_head))
	    driver_instance->expand(viewGraHeaderPtr(view_head), level);
        else if ( is_steView( view_head ) )
            ste_explode( checked_cast(steView, view_head), level );
    }
}

//------------------------------------------

extern "C" void viewerShell_collapse_current(viewerShell* vs, int level)
{ vs->collapse_current(level); }

void viewerShell::collapse_current(int level)
{
    Initialize(viewerShell::collapse_current);

    if (current_viewer >= 0)
    {
	viewPtr view_head = viewers[current_viewer]->get_view_ptr();
	if(is_viewGraHeader(view_head))
	    driver_instance->collapse(view_head, level);
        else if ( is_steView( view_head ) )
            ste_collapse( checked_cast(steView, view_head), level );
    }
}

//------------------------------------------

void view_refresh_window(viewGraHeaderPtr v)
{
    v->get_gra_interface()->RefreshWindow();
}

extern "C" void viewerShell_refresh_gra_view(viewerShell* vh)
{
    Initialize(viewerShell_refresh_gra_view);

    int vt = gra_type();
    if (vt != Rep_ERD && vt != Rep_DataChart && vt != Rep_SubsystemMap)
        return;

    int vidx   = vh->get_current_viewer();
    viewer *vr = vh->get_viewer(vidx);
    view *v    = vr->get_view_ptr();

    view_refresh_window(viewGraHeaderPtr(v));
}

//------------------------------------------

extern "C" void viewerShell_view_history(viewerShell* vs, int delta)
{ vs->view_history(delta); }

void viewerShell::view_history(int delta)
{
    Initialize(viewerShell::view_history);

    if(current_viewer >= 0)
    {
	viewer* vwr = viewers[current_viewer];
	vwr->change_view_from_history(delta);
    }
}

//------------------------------------------

viewer* viewerShell::viewer_of_rtlview(viewListHeaderPtr vw) // class static
{
    Initialize (viewerShell::viewer_of_rtlview);

    for(int i = 0; i < vsh_array.size(); ++i)
    {
	viewerShell* vs = (viewerShell*)vsh_array[i];
	for (int i=0; i < vs->num_viewers; i++)
	    if (vw == vs->viewers[i]->get_rtl_view_ptr())
		return vs->viewers[i];
    }

    return NULL;
}

//------------------------------------------
// cleaned:
viewerShell* viewerShell::viewerShell_of_view(viewPtr vw) // class static
{
    Initialize (viewerShell::viewerShell_of_view);

    for(int i = 0; i < vsh_array.size(); ++i)
    {
	viewerShell* vs = (viewerShell*)vsh_array[i];
	if(vs->dlg && vs->dlg->contains_view(vw))
	    return vs;
    }

    return NULL;
}

// cleaned:
viewerShell* viewerShell::viewerShell_of_screen(steScreenPtr scr) // class static
{
    Initialize (viewerShell::viewerShell_of_screen);

    if (!scr)
	return NULL;

    viewer *vwr = checked_cast(viewer, get_relation(viewer_of_screen, scr));
    viewerShell *vsh = (vwr) ? vwr->get_shell() : NULL;
    return vsh;
}

//------------------------------------------

viewer* viewerShell::viewer_of_view(viewPtr v)
{
    Initialize (viewerShell::viewer_of_view);

    viewer* vwr = NULL;
    for(int i = 0; i < num_viewers; i++)
	if(viewers[i]  &&  viewers[i]->get_view_ptr() == v)
	{
	    vwr = viewers[i];
	    break;
	}
    return vwr;
}

//------------------------------------------

extern "C" void viewerShell_set_current_view (void* ptr)
{
    Initialize (viewerShell_set_current_view);

    viewPtr view_header = viewPtr(ptr);
    viewerShell* vsh = viewerShell::viewerShell_of_view(view_header);
    if(vsh)
    {
	viewer* vwr = vsh->viewer_of_view(view_header);
	if(vwr) {
	    // ste_finalize() on swithcing focus from steView to not steView
	    viewerShell *old_vsh = viewerShell::get_current_vs(1);   // no create
	    viewPtr old_v = (old_vsh) ? old_vsh->get_current_view() : NULL;
	    if (old_v && old_v != view_header && !is_steView(view_header)) {
		int tp = old_v->get_type ();
		if (tp == Rep_TextText || tp == Rep_SmtText)
		{
		    ste_finalize (old_v);
		}
	    }

	    viewerShell::focused_viewer (vwr);
	}
    }
}

//------------------------------------------

extern "C" viewerEditMode viewerShell_get_mode(viewerShell* vs)
{ return vs->get_mode(); }

//------------------------------------------

viewPtr viewerShell::find_view(ldrPtr ldr_head)
{
    Initialize (viewerShell::find_view);

    if(ldr_head)
    {
	RTLNodePtr rtl_node = RTLNodePtr(rtl->get_root());
	ObjPtr el;
	ForEachS(el, rtl_node->rtl_contents())
	{
	    viewPtr view_head = checked_cast(view, el);
	    if(ldr_head == view_head->get_ldrHeader())
		ReturnValue (view_head);
	}
    }
    return NULL;
}


viewPtr viewerShell::find_view(viewPtr view_head)
{
    Initialize (viewerShell::find_view);

    RTLNodePtr rtl_node = RTLNodePtr(rtl->get_root());
    if(rtl_node->rtl_includes(view_head))
	return view_head;

    viewPtr eq_view_head = find_view(view_head->get_ldrHeader());

    return eq_view_head ? eq_view_head : view_head;
}


viewPtr viewerShell::find_view_for_viewer(ldrPtr ldr_head, viewer *the_viewer)
{
    Initialize (viewerShell::find_view_for_viewer);

    if(ldr_head)
    {
	viewPtr open_view = the_viewer->get_view_ptr();
	ldrPtr open_ldr = open_view ? open_view->get_ldrHeader() : NULL;
	if (ldr_head == open_ldr)
	    return open_view;

	RTLNodePtr rtl_node = RTLNodePtr(rtl->get_root());
	ObjPtr el;
	ForEachS(el, rtl_node->rtl_contents())
	{
	    viewPtr view_head = checked_cast(view, el);
	    if(ldr_head == view_head->get_ldrHeader()
	       && !viewer_of_view(view_head))
		return (view_head);
	}
    }
    return NULL;
}

//------------------------------------------

static int focus_node_of_category(
    viewPtr current_view, char* current_marker_categ,
    int direction, int from_beginning)
//
// Attempt to move along a marker stream, return true on success.
//
// This is a KLUGE because focus_node_of_category should be a virtual
// function on class view, but it is not.
//
{
    Initialize(focus_node_of_category);

    if(is_steView(current_view))
    {
	if(steViewPtr(current_view)->focus_node_of_category(
	    current_marker_categ, direction, from_beginning))
	    return 1;
    }
    else if(is_viewGraHeader(current_view))
    {
	if (viewGraHeaderPtr(current_view)->focus_node_of_category(
	    current_marker_categ, direction, from_beginning))
	    return 1;
    }
    return 0;
}

//------------------------------------------

static int wrapinc(int index, int direction, int size)
//
// Move index up or down depending on direction, wrapping at zero and size.
//
{
    Initialize(wrapinc);

    if(direction < 0)
    {
	if(--index < 0)
	    index = size - 1;
    }
    else
    {
	if(++index >= size)
	    index = 0;
    }
    return index;
}

//------------------------------------------


extern "C" void viewerShell_app_save(viewerShell* vs)
{ vs->app_save(); }

void viewerShell::app_save()
{
    Initialize(viewerShell::app_save);

    int success = 0;
    objArr app_array;
    driver_instance->fill_array_with_selected_appnodes(NULL, app_array, 1);
    viewPtr vp = get_current_view();
    if(vp){
	ldrPtr lp = view_get_ldr(vp);
	if(lp){
	    appPtr ap = ldr_get_app(lp);
	    if(ap)
		app_array.insert_last(ap);
	}
    }
    for(int i = 0; i < app_array.size(); i++){
	appPtr     ap  = NULL;
	Relational *ob = (Relational *)app_array[i];
	if(!ob)
	    continue;
	if(is_projModule(ob))
	    ap = projectModule_get_appHeader((projModule *)ob);
	else if(is_app(ob))
	    ap = (appPtr)ob;
	if(ap){
	    if(is_smtHeader(ap) || is_steDocument(ap) || is_objRawApp(ap)){
		if (ap->is_modified()) {
		    if(success = db_save_or_cancel(ap, 0, 1)){
			if (success != -1)
			    ap->clear_modified();
		    } else {
			msg("Save_failed:viewerShell.h.C", error_sev) << ap->get_name() << eom;
		    }
		} 
	    }
	}
    }
}

extern "C" void viewerShell_app_save_as(viewerShell* vs)
{ vs->app_save_as(); }

void viewerShell::app_save_as()
{
// NOT IMPLEMENTED //
}


extern "C" void viewerShell_is_customizing(
    viewerShell* vsh, viewerCustMode new_mode)
{ vsh->is_customizing(new_mode); }


void viewerShell::is_customizing(viewerCustMode new_mode)
{
    Initialize(viewerShell::is_customizing);

    cust_mode = new_mode;
}


extern "C" int viewerShell_finalize(viewerShell* shell, Widget widget)
//
// Called before any command is executed in the viewerShell interface.  If
// it returns 0, the interface is in "customize" mode and the command
// should not be executed (usually), otherwise this function finalizes any
// pending changes in the views.
//
{
    Initialize(viewerShell_finalize);

    if(shell  &&  widget  &&  shell->is_customizing())
    {
	if (shell->is_customizing() == VC_customize)
	    shell->add_buttonbar_item(widget);
	return 0;
    }
    else
    {
	viewerShell::pre_hook(shell);
	return 1;
    }
}


void viewerShell::pre_hook(viewerShell* shell)
{
    Initialize(viewerShell::pre_hook);

    push_busy_cursor();

    if(hook_shell = shell) {
	viewPtr view_header = shell->get_current_view();
	// Always call ste_finalize to freeze epoch.
	ste_finalize(view_header);
	if (view_header &&  !is_steView(view_header))
	    gra_finalize();
    }
}

extern "C" void viewerShell_post_hook(Widget)
{ viewerShell::post_hook(); }


void viewerShell::post_hook()
//
// Called after command has returned, assumes that pre_hook was called.
//
{
    Initialize(viewerShell::post_hook);

    reset_cursor();

    if(hook_shell)
    {
	hook_shell->sensitize_buttonbar();
	hook_shell = NULL;
    }
}

extern "C" int viewer_finalize(viewer* vwr, Widget widget)
//
// Convenience function for calling "finalize" from a viewer.
//
{
    Initialize(viewer_finalize);

    return viewerShell_finalize(vwr->get_shell(), widget);
}


extern "C" void viewer_post_hook(viewer*, Widget)
//
// Matching post_hook function for viewer_finalize.
//
{
    Initialize(viewer_post_hook);

    viewerShell::post_hook();
}


//------------------------------------------

extern "C" void viewerShell_decorate_source(viewerShell* vs, int flag)
{ vs->decorate_source(flag); }

void viewerShell::decorate_source(int flag)
{
    Initialize(viewerShell::decorate_source);

    if(current_viewer >= 0)
    {
	viewPtr view_head = viewers[current_viewer]->get_view_ptr();
	if(is_steView(view_head))
	    ste_interface_decorate_source(steViewPtr(view_head), flag);
    }	
}

//------------------------------------------

extern "C" void viewerShell_enable_decorate_source(viewerShell* vs)
{ vs->enable_decorate_source(); }


void viewerShell::enable_decorate_source()
{
    Initialize(viewerShell::enable_decorate_source);

    if(current_viewer >= 0)
    {
	viewPtr view_head = viewers[current_viewer]->get_view_ptr();
	if(view_head  &&  is_steView(view_head))
	{
	    const int decorated = steViewPtr(view_head)->decorate_source();
	    vs_enable_decorate_source(wij, TRUE, decorated);
	    return;
	}
    }
    vs_enable_decorate_source(wij, FALSE, FALSE);
}

//--------------------------------------


extern "C" void viewerShell_initial_shell() 
// called on startup.
// pops up a viewerShell with a list of top level projects
{
    viewerShell* vsh = new viewerShell(CREATE_MINIBROWSER);
    view_set_target_viewer(vsh->create_viewer());
    viewerShell::set_current_vs(vsh);
    
    RTLPtr rtl = projNode::get_project_rtl();
    rtl->set_name("Top Level Projects");
    
    vsh->open_view(rtl, Rep_RawText, NULL);
    
    vsh->bring_to_top();
    vsh->map();
}

//
// enable manage menu items
//
void viewerShell::enable_manager(int on)
{
    if (customize::no_cm())
        on = 0;
    vs_enable_manager (wij, on);
}

//
// static member to enable manage menu items on all shells
//
void viewerShell::enable_managers()
{
    Initialize(viewerShell::enable_managers);

    ObjPtr el;
    objArr tmp = viewerShell::vsh_array;
    int writeable = projNode::home_is_writeable();
    ForEach(el,tmp) {
	viewerShellPtr(el)->enable_manager(writeable);
    }
}

// ------------------------------------------------
// viewerShell method to configure debugger session
// ------------------------------------------------
// get viewer + view for the session
// returns: -1: error, 0: OK
int viewerShell::configure(app * sptr, viewer * & vwr, view * & vw)
{
    return -1;
}

//------------------------------------------
// viewerShell::set_closure_state [static]
//------------------------------------------

void viewerShell::set_closure_state(bool closure_enabled) {
   Initialize(viewerShell::set_closure_state);

   for (size_t i = 0; i < vsh_array.size(); i++) {
      viewerShell* vs = (viewerShell*) vsh_array[i];
      if (vs && vs->mini_browser) {
	 vs->mini_browser->closure(closure_enabled);
      }
   }
}

