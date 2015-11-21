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
// gtRTL.h.C
//------------------------------------------
// synopsis:
// Standard interface to run-time lists
//------------------------------------------

// INCLUDE FILES
#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <representation.h>
#include <systemMessages.h>
#include <Notifier.h>
#include <Relational.h>
#include <driver.h>
#include <dialog.h>
#include <RTL.h>
#include <RTL_externs.h>
#include <viewList.h>
#include <top_widgets.h>
#include <instanceBrowser.h>

#include <gtRTL_util.h>
#include <gtRTL.h>
#include <gtPushButton.h>
#include <gtBaseXm.h>

#include <X11/StringDefs.h>
#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
#endif /* ISO_CPP_HEADERS */
#include <machdep.h>

struct selContext {
    gtRTL*      rtl;
    symbolArr   arr;
    bool        notify;
    selContext  (gtRTL* r, const symbolArr& a, bool n) :
                                rtl (r), arr (a), notify (n) { }
};

// Resources for color definition (filtered/unfiltered state)

typedef struct{
  Pixel filtered_color;
} RTLResources, *RTLResourcesPtr;

static int        RTL_resource_loaded = 0;
static XtResource resources[] = { { "rtlFilteredColor", "RTLFilteredColor", XtRPixel, sizeof(Pixel),
                                    XtOffset(RTLResourcesPtr, filtered_color), XtRImmediate, (caddr_t) 1} };

RTLResources  RTL_resources;
extern Widget UxTopLevel;

static void load_RTL_resources(void)
{
    if(RTL_resource_loaded)
	return;
    XtGetApplicationResources(UxTopLevel, (XtPointer)&RTL_resources, resources, XtNumber(resources), 
			      NULL, 0);
    RTL_resource_loaded = 1;
}


// FUNCTION DEFINITIONS

static void gtList_of_gtRTL_destroyed(gtBase*, void* data)
{
    delete (gtRTL*)data;
}

int gtRTL::get_filtered_color()
{
    return RTL_resources.filtered_color;
}

gtRTL::gtRTL(gtBase* parent, const char* name, const char* title,
	     gtListStyle ltype, const char** entries, int num_entries)
: dlg(new dialog),
  //ui_list_data(ui_make_empty_list()),
  rtl(NULL)
{
    delete r;
    r              = NULL;
    old_background = 1;
    handle_buttons = 0;
    
    load_RTL_resources();
    driver_instance->add_dialog(dlg);
    list = gtList::create(parent, name, title, ltype, entries, num_entries,
	  gtRTL::util_CB, this, update_CB, this, gtRTL::reset_CB, this, this);
    list->user_destroy(gtList_of_gtRTL_destroyed, this);
    this->ltype = ltype;
}


gtRTL::~gtRTL()
{
    list = NULL;
    set_rtl(NULL);
    driver_instance->close_dialog(dlg);
}

void gtRTL::handle_special_buttons(int flag)
{
    handle_buttons = flag;
}

void gtRTL::set_rtl(symbolArr & sa, int sort_order)
{
  Initialize(gtRTL::set_rtl);
  
  RTLPtr rtl = RTL();
  char* oldname=0;
  if (rtl)
      oldname = rtl->get_name();
  class RTL *tmp_rtl = new class RTL(oldname);
  RTLNodePtr nrtl_root = checked_cast(RTLNode, tmp_rtl->get_root());
  
  symbolPtr el2;
  ForEachS (el2, sa)
  {
    rtl_add_obj (nrtl_root, el2);
  }
  
  set_rtl(tmp_rtl, sort_order);
}


void gtRTL::set_rtl(RTLPtr r, int sort_order)
{
    Initialize(gtRTL::set_rtl);

    if(rtl == r)
	return;

    regenerate_rtl (r, sort_order);
}

void gtRTL::set_rtl_reset_filters(RTLPtr r)
{
    Initialize(gtRTL::set_rtl_reset_filters);
    
    regenerate_rtl(r, 1, 1);
}
	

void gtRTL::set_format (char* fmt) {
  Initialize(gtRTL::set_format);
  if(rtl)
    {
      viewListHeader* vh = checked_cast(viewListHeader, dlg->get_view_ptr(0));      
      if (vh)
	{
	  vh->format_spec(fmt);
	}
    }
}

void gtRTL::set_sort (char* srt) {
  Initialize(gtRTL::set_sort);
  if(rtl)
    {
      viewListHeader* vh = checked_cast(viewListHeader, dlg->get_view_ptr(0));      
      if (vh)
	{
	  vh->sort_spec(srt);
	}
    }
}


// value 2 for reset_filters is a special case used now by minibrowser to just 
// update contnets of the list without resetting user filters	    
void gtRTL::regenerate_rtl(RTLPtr r, int sort_order, int reset_filters)
{
    Initialize(gtRTL::regenerate_rtl);
    genString sort_spec;
    genString format_spec;
    genString show_spec;
    genString hide_spec;
    int save_specs = 0;
    
    if(rtl) {
	viewListHeader* old_vh = checked_cast(viewListHeader, dlg->get_view_ptr(0));

	if(old_vh) {
            sort_spec   = old_vh->sort_spec();
            format_spec = old_vh->format_spec();
	    
	    if(rtl_get_APIhandler(rtl)) {
		if(reset_filters == 2) {
		    show_filter = old_vh->show_filter();
		    hide_filter = old_vh->hide_filter();
		} else {
		    show_filter = "";
		    hide_filter = "";
		}
	    } else {
		if(!reset_filters) {
		    show_filter = old_vh->show_filter();
		    hide_filter = old_vh->hide_filter();
		} else {
		    show_filter = "";
		    hide_filter = "";
		}
	    }
	    save_specs = 1;
 
	    dlg->close_window(0);
	    dlg->close_view(0, true);
	}
    }
    if(rtl = r) {
	if(handle_buttons)
	    list->set_special_buttons(0);
	miniBrowserRTL_API *api = rtl_get_APIhandler(rtl);
	if(api) {
	    if(reset_filters != 2) {
		char *new_sort = api->get_sort_spec(rtl);
		if(new_sort) {
		    sort_spec  = new_sort;
		    save_specs = 1;
		} else
		    sort_spec = "";
		format_spec = "";
		show_filter = "";
		hide_filter = "";
	    }
	    api->set_list(list);
	    if(handle_buttons) {
		if(api->need_vcr_control(rtl)) 
		    list->set_special_buttons(1);
		api->make_own_buttons(list->get_button_form(), this, rtl);
	    }
	}
	
	list->reset_pos();
	dlg->open_view(0, rtl, checked_cast(RTLNode, rtl->get_root()),
		       Rep_List, sort_order, rtlDisplayList);
	dlg->create_window(0, list->container()->rep()->widget());

        //set notifier for update
	viewListHeader* vh = checked_cast(viewListHeader, dlg->get_view_ptr(0));
	notifier.set_prop(PROP_SWIDGET, vh, this);

        if(save_specs) {
            vh->sort_spec((char*)sort_spec);
            vh->format_spec((char*)format_spec);
            vh->show_filter((char*)show_filter);
            vh->hide_filter((char*)hide_filter);
            save_specs = 0;
        }

    } else if(list) {		// list may be zeroed out by destructor.
	if(handle_buttons)
	    list->set_special_buttons(0);
	list->delete_all_items();
    }
}


symbolArr* gtRTL::app_nodes(symbolArr* array_ptr)
{
    Initialize(gtRTL::app_nodes);

    if(rtl == NULL)
	return NULL;

    if(array_ptr == NULL)
	array_ptr = new symbolArr;
    symbolArr& array =* array_ptr;

    int* positions = NULL;
    int  count = 0;
    list->get_selected_pos( &positions, &count);
    if(count)
    {
	viewListHeaderPtr view = checked_cast(viewListHeader, dlg->get_view_ptr(0));
	if(view)
	{
	    for(int i = 0; i < count; ++i)
	    {
		symbolPtr app_node = view->get_obj (positions[i] - 1);
		array.insert_last(app_node);
	    }
	}
	OSapi_free(positions);
    }

    return array_ptr;
}

void gtRTL::select_symbols (const symbolArr& array, int notify)
{
    Initialize (gtRTL::select_symbols);

    if (rtl == NULL)
	return;

    RTLNodePtr		node = checked_cast (RTLNode, rtl->get_root ());
    symbolArr& 		contents = node->rtl_contents ();
    int 		count = contents.size ();
    viewListHeaderPtr 	view;
    symbolPtr		sym;
    symbolArr           tmp = array;

    view = checked_cast (viewListHeader, dlg->get_view_ptr (0));
    if (view == NULL)
	return;
    
    list->sel_type(gtMultiple);
    for(int i = 0; i < count; i++){
	if(tmp.includes((sym = view->get_obj(i)))){
	    select_pos(i + 1, notify);
	    tmp.remove(sym);
	}
    }
    list->sel_type(ltype);
}

static void rtlSelectionCB (void* ctx)
{
    Initialize (rtlSelectionCB);

    if (ctx == NULL)
        return;

    selContext* context = (selContext*) ctx;
    if (context->rtl == NULL)
        return;

    context->rtl->select_symbols (context->arr, context->notify);

    delete context;
}

void gtRTL::post_selection (const symbolArr& syms, int notify)
{
    Initialize (gtRTL::post_selection);

    selContext* context = new selContext (this, syms, notify);

    pset_send_own_callback (rtlSelectionCB, context);
}

void gtRTL::post_selection (const symbolPtr& sym, int notify)
{
    Initialize (gtRTL::post_selection);

    symbolArr syms;

    syms.insert_last (sym);
    post_selection (syms, notify);
}

objArr* gtRTL::app_nodes(objArr* array_ptr)
{
    Initialize(gtRTL::app_nodes);

    if(rtl == NULL)
	return NULL;

    if(array_ptr == NULL)
	array_ptr = new objArr;
    objArr& array =* array_ptr;

    int* positions = NULL;
    int  count = 0;
    list->get_selected_pos( &positions, &count);
    if(count)
    {
	viewListHeaderPtr view = checked_cast(viewListHeader, dlg->get_view_ptr(0));
	if(view)
	{
	    for(int i = 0; i < count; ++i)
	    {
		ObjPtr app_node = view->get_obj (positions[i] - 1);
		array.insert_last(app_node);
	    }
	}
	OSapi_free(positions);
    }

    return array_ptr;
}


void gtRTL::select_appnode(appTree* , int )  // node, notify
{
    Initialize(gtRTL::select_appnode);

    void* dummy = NULL;

    //view_flush_rtls(dummy);  // bring the ui_list into sync with the RTL
    //int pos = view_index_of_appTree(checked_cast(viewListHeader, dlg->get_view_ptr(0)),
    //			    node);
    //if(pos >= 0)
    //list->select_pos(pos + 1, notify); // XmList indices start @ 1
}


void gtRTL::dialog_create_window(dialog* dlg, int n)
{
    dlg->create_window(n, list->container()->rep()->widget());
}


void gtRTL::dlg_create_window_instance(dialog* dlg, int n)
{
    dialog_create_window_instance(n, dlg, list->container()->rep()->widget());
}

extern void cmd_filter_record_count (int);

void gtRTL::update_CB(gtList* list, gtEvent*, void* data, gtReason)
//
// Called whenever the view is updated.
//
{
    Initialize(gtRTL::update_CB);

    gtPushButton* p = list->status_button();
    gtRTL*   gt_rtl = (gtRTL *) data;
    if(p)
    {
	char buffer[128];

	if (data) 
	{

	  viewListHeader* vh = (viewListHeader*) gt_rtl->dial()->get_view_ptr(0);

	  if (vh)
	  { // if there is a show or hide filter on the list


	    const char* show = vh->show_filter();
	    const char* hide = vh->hide_filter();

	    if ((show && *show) || (hide && *hide))
	    {
	      RTLPtr rtl = gt_rtl->RTL();
	      list->set_reset_sensitive(1);
	      if(gt_rtl->old_background == 1 && RTL_resources.filtered_color != 1){
		  gt_rtl->old_background = list->background();
		  list->background(RTL_resources.filtered_color);
	      }
	      if (rtl) 
	      {
		RTLNode* rtl_node = checked_cast(RTLNode, rtl->get_root());

		if (rtl_node) 
		{
		  sprintf(buffer, gettext(TXT("%d of %d from %d")),
			  list->num_selected(), list->num_items(), 
			  rtl_node->rtl_contents().size());

		  cmd_filter_record_count (list->num_items());

		  p->title(buffer);
		  return;
	} } } }	}
	
	list->set_reset_sensitive(0);
	if(gt_rtl->old_background != 1){
	    list->background(gt_rtl->old_background);
	    gt_rtl->old_background = 1;
	}
	sprintf(buffer, gettext(TXT("%d of %d")),
		list->num_selected(), list->num_items());

	cmd_filter_record_count (list->num_items());

	p->title(buffer);
    }
}


void gtRTL::update_status()
{
    update_CB(gt_list(), NULL, this, gtReason(0));
}


/*
   START-LOG-------------------------------------------

   $Log: gtRTL.h.C  $
   Revision 1.19 2000/07/10 23:05:21EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.19  1994/03/20  23:18:51  boris
Bug track: #6721
Fixed filter reset on Edit operations and "blinking" problem in viewerShell

Revision 1.18  1994/03/15  14:10:10  davea
Bug track: 6615, 4051
preserve rtl name, if any
do not grey out icon in scan mode
performance improvements

Revision 1.17  1994/01/25  23:53:20  kws
Bug track: 6112
Fix saving sort/format/hide/show soecs between browser rtl queries

Revision 1.16  1994/01/21  03:40:16  kws
*** empty log message ***

Revision 1.15  1994/01/21  00:23:11  kws
New rtl display

Revision 1.14  1993/12/31  19:38:19  boris
Bug track: Test Validation
Test Validation

Revision 1.13  1993/11/16  01:01:48  trung
Bug track: 0
propagation project

Revision 1.12  1993/06/21  16:43:49  swu
fix for bug #2889 so that filter list buttons read "n of n from n"

Revision 1.11  1993/04/30  18:50:40  glenn
Break gtRTL up into gtRTL.h.C, gtRTL_util.h.C, and gtRTL_cfg.h.C

Revision 1.10  1993/04/27  21:44:59  glenn
Add gtLabel.h
Delete gtRTLs gtRep in its ctor (not used).
Call ui_free_list in dtor.
Add TXT macros.
Plug leak in gtRTL_cfg ctor (bug #3469).
Rename "~/.RTL_filters" to "~/.psetfilt".

Revision 1.9  1993/04/22  21:19:01  davea
bug 3461 - add a return to get_entry_by_name()

Revision 1.8  1993/02/12  16:04:12  oak
Removed customize::shadow_root().

Revision 1.7  1993/02/12  16:00:58  oak
Moved RTL_filters to ~/.RTL_filters

Revision 1.6  1992/12/15  20:07:06  aharlap
changed for new xref

Revision 1.5  1992/11/23  18:20:32  wmm
typesafe casts.

Revision 1.4  1992/11/10  16:39:18  glenn
Replace call to system("rm -rf ...") with unlink().
Remove redundant typecasts of genString to (char*).

Revision 1.3  1992/10/20  15:58:25  oak
Deletes the file RTL_filters if it is of length
zero.  Fixes bug #1621.

Revision 1.2  92/10/13  14:07:12  kws
FIx crash if view as deleted before gtRTL

Revision 1.1  92/10/10  21:53:17  builder
Initial revision

   END-LOG---------------------------------------------
*/
