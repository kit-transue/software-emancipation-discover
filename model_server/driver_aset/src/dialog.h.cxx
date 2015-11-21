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
// dialog
//------------------------------------------
// synopsis:
// Abstract base class of all dialogs.
//
// description:
// A dialog contains one or more views with a
// common insertion point.  It also contains
// some other bookkeeping information.
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------

// include files
#include "cLibraryFunctions.h"
#include "msg.h"
#include "representation.h"
#include "OperPoint.h"
#include "ldrBrowserHierarchy.h"
#include "ldrSubsysHierarchy.h"
#include "ldrScrapbook.h"
#include "ldrRTLDisplayType.h"
#include "viewHierarchyHeader.h"
#include "viewSelection.h"
#include "viewFn.h"
#include "viewList.h"
#include "viewGridHeader.h"
#include "steView.h"
#include "steRawView.h"
#include "ste.h"
#include "astTree.h"
#include "dialog.h"
#include "ldrDGraph.h"
#include "ui.h"
#include "ldrNode.h"
#include "driver.h"
#include "ldrSmtHierarchy.h"
#include "ldrSmtFlowchart.h"
#include "symbolTreeHeader.h"
#include "symbolLdrHeader.h"
#include "ldrSmodShort.h"
#include "ldrERDHierarchy.h"
#include "ldrDCHierarchy.h"
#include "ste_interface.h"
#include "RTL_externs.h"
#include "messages.h"
#include "dd_or_xref_node.h"
#include "appMarker.h"


RelClass(viewNode);
RelClass(viewSymbolNode);

init_relational(dialog, Relational);

extern int gala_current_window;

// External functions

extern "C" bool isServer();
ddElement* smt_get_dd(smtTreePtr);
extern viewPtr ct_viewGraHeader(ldrPtr, repType);

extern "C" void dialog_set_current_window(void*);
extern int ste_is_short_smt();

// Delete a viewHeader and possibly its ldrHeader.

static void delete_view_header(view* viewp)
{
    if (viewp == NULL)
	return;

    appPtr  appp = NULL;
    ldrPtr  ldrp = viewp->get_ldrHeader();
    if (ldrp) {
	if (ldrp->get_type() == Rep_DGraph) {
	    obj_delete(viewp);
	    return;
	}
	appp = ldrp->get_appHeader();
	viewp->rem_rel(ldr_of_view);
    }

    obj_delete(viewp);
	
    if (ldrp) {
	Obj* views = get_relation(view_of_ldr, ldrp);
	if (!views || views->size() == 0) {
    	    // If this was the ldrs only view, delete the ldr, too.
	    // Disconnect the ldr from the app immediately
	    // to avoid unnecessary (and possibly fatal) propagation.
	    ldrp->rem_rel(app_of_ldr);
	    obj_delete(ldrp);
	    
	    if (appp && appp->get_type() == App_RAW) {
		Obj *ldrs = get_relation(ldr_of_app, appp);
		if (!ldrs || ldrs->size() == 0)
		    obj_delete(appp);
	    }
	}
    }
}

// Return the name of whatever the view is looking at

char* get_name_o_view(view* viewp)
{
    Initialize(get_name_o_view);

    if (!viewp)
	return(NULL);
    ldrPtr ldrp = viewp ? viewp->get_ldrHeader() : NULL;
    ldrTreePtr ldrt = ldrp ? checked_cast(ldrTree,ldrp->get_root()) : NULL;
    appTreePtr subroot = ldrt ? ldrt->get_appTree() : NULL;
    appPtr apphead = subroot ? subroot->get_header() : NULL;
    repType rep_type = repType(viewp->get_type());
    if (apphead)
    {
	switch(apphead->get_type())
	{
	  case App_DD:
	    if (subroot)
		return(checked_cast(ddElement,subroot)->get_name());
	    else 
		return("OODT Class Browser");

	  case App_SMT:
	    {
		appTreePtr root = checked_cast(appTree,apphead->get_root());
		appTreePtr temp_subroot = subroot ? subroot : root;

		// Get name of the dialog
		while(temp_subroot != root)
		{
		    ddElementPtr ep = smt_get_dd(checked_cast(smtTree,temp_subroot));
		    if(ep)	// dd element must have some name
			return(ep->get_name());
		    else
			temp_subroot = checked_cast(appTree,temp_subroot->get_parent());
		}
		// if subroot is same as root, get name from header
		return(apphead->get_name());
	    }

	  case App_RTL:
	    if (rep_type == Rep_MultiConnection || rep_type ==
                  Rep_OODT_Inheritance || rep_type == Rep_OODT_Relations ||
                  rep_type == Rep_OODT_Scope)
	       if (subroot)
		  return checked_cast(dd_or_xref_node, subroot)->get_name();
	       else return "OODT Class Browser";
	    else return apphead->get_name();

	  case App_STE:
	  case App_SUBSYS:
	  default:
	    return(apphead->get_name());

	}
    }
    return(NULL);
}

// Opens up a view from a ldr
viewPtr dialog::open_view(ldrPtr ldr_head, repType type, int sorted)
{
    Initialize (dialog::open_view);

    if(!ldr_head)
	return(NULL);

    // Create view of the proper type.
    viewPtr viewp = NULL;

    if (!ste_interface_get_epoch_flag()) {
	if (type == Rep_SmtText)
	    type = Rep_VertHierarchy;
	else if (type == Rep_TextText)
	    type = Rep_TextFullDiagram;
        if ( type == Rep_RawText ){
           msg("ERROR: Emacs is not loaded. Cannot view ascii files. To load Emacs, restart DISCOVER with the discover script.") << eom;
           return 0;
        }
    }

    switch(type)
    {
      case Rep_List:
	{
	viewp = db_new(viewListHeader, (ldr_head, type, 0, sorted));
	break; }

      case Rep_Menu:
	{
	viewp = db_new(viewListHeader, (ldr_head, type, 1, 1));
	break;}
	
      case Rep_TextText:
      case Rep_SmtText:
	{
	viewp = db_new(steView, (checked_cast(steDisplayHeader, ldr_head)));
	viewp->set_type(type);
	break; }

      case Rep_RawText:
	{
	viewp = db_new(steRawView, (checked_cast(objRawLdr,ldr_head)));
	viewp->set_type(type);
        break; }

      case Rep_DGraph:
      default:
	viewp = ct_viewGraHeader(ldr_head, type);
	break;
    }
    if (viewp && is_steView(viewp))
    {
	if ((checked_cast(steView,viewp))->is_valid() == NULL )
	{

            msg("ERROR: failed open view.") << eom;
	    obj_delete (viewp);
	    viewp = NULL;
	}
    }
    if(viewp)
    {
	put_relation(view_of_ldr, ldr_head, viewp);
	char* view_name = get_name_o_view(viewp);
	if(view_name)
	    viewp->set_name(view_name);
	else{ 
	    //bug fix 7247. 7/7/94.
	    //If the view is graphical and has no name, then it is not
	    //created because the names of all graphical views are subsequently
	    //inserted into the view history list. Therefore a graphical view with
	    //no name( which indicates that it has not been created correctly)
	    //will produce a big time crash.
	    if(is_viewGraHeader(viewp) && (viewp->get_name() != NULL))
	    { 
               msg("ERROR: failed open view.") << eom;
	       obj_delete (viewp);
	       viewp = NULL;
	   }
	}
		
		
       
    }

    return(viewp);
}

//
// Opens a view of specified type for an apptree. level specifies 
// The depth of view tree to be created, depth of 0 means create
// entire view tree.
//
viewPtr dialog::open_view(
    appTreePtr subroot, repType type, int level,
    int sort_order, ldrRTLDisplayType  fields)
{
    Initialize(dialog::open_view);

    appPtr approot = (subroot ? subroot->get_header() : NULL);

    if(approot == NULL && type != Rep_ERD && type != Rep_DataChart)
	return(NULL);

    ldr* ldr_head = layout(approot, subroot, type, level, sort_order, fields);

    // Create view of the proper type.
    if(! ldr_head)
      return NULL;
    if(type == Rep_List)
      return open_view(ldr_head, type, sort_order);
    else
      return open_view(ldr_head, type, level);
}

//------------------------------------------
// open_view
//
// Open a view for the dialog of the appropriate type.
//
// parameters:
//	view_num - index of the view to open
//	approot  - appheader of the ldr of the view.
//	subroot  - node of the apptree of the ldr root
//	type	 - One of the several types of representation
//	sort-order - How to sort the ldr (relevent to RTL only)
//	fields   - Fields to display (relevent to RTL only)
//------------------------------------------

void dialog::open_view(int view_num, appPtr approot, appTreePtr subroot,
		       repType type, int sort_order, ldrRTLDisplayType fields)
{
    Initialize(dialog::open_view);

    const int level = 0;

    if(!subroot && approot)
	subroot = checked_cast(appTree,approot->get_root());

    view* vvv = open_view(subroot, type, level, sort_order, fields);

    if(vvv)
      set_view(view_num, vvv);

}

//------------------------------------------
// set_view
//
// Store a view-header at the specified view slot.
//
// parameters:
//	view_num  - index into view array.
//	viewp     - pointer to the view-header.
//------------------------------------------

void dialog::set_view(int view_num, viewPtr viewp)
{
    Initialize(dialog::set_view);

    if(view_num < 0)
	return;

    if(view_num < max_view_num())
    {
	close_view(view_num, false);
    }
    else
    {
	const int diff = view_num - max_view_num() + 1;
	if(diff > 10)	// A (somewhat arbitrary) sanity check.
	{
	    msg("Error: view_num too big for its britches.\n") << eom;
	    return;
	}

	// Make room for the new view.
	view_array.grow(diff);
	for(int i = max_view_num() - diff; i < max_view_num(); ++i)
	    view_array[i]->zero();
    }

    get_view(view_num) = viewp;
    current_view = viewp;
    current_text = NULL;
    oper_pt      = NULL;

    return;
}

// Close the view and delete it if delete_flag is true.

void dialog::close_view(int view_num, int delete_view)
{
   Initialize(dialog::close_view);

   viewPtr viewp = get_view_ptr(view_num);
   if(viewp)
   {
      driver_instance->unselect(viewp);
      close_window(view_num);

      if(is_steView(viewp))
         checked_cast(steView,viewp)->close_window();

      // view array should be zeroed out before deleting to
      // prevent recursive call
      view_array[view_num]->zero();

      if(delete_view)
	  delete_view_header(viewp);
   }

   return;
}

// Convenience function for previous close_view.

void dialog::close_view(viewPtr viewp, int delete_view)
{
    Initialize(dialog::close_view);

    int view_num = get_view_num(viewp);
    if(view_num >= 0)
	close_view(view_num, delete_view);

    return;
}

//------------------------------------------
// Constructor dialog
//------------------------------------------

dialog::dialog(char* ttl)
: diag_type(dialogType_unknown),
  title(ttl),
  dialog_swidget(NULL),
  detach_hook(NULL),
  detach_data(NULL),
  current_view(NULL),
  current_text(NULL),
  oper_pt(NULL),
  num_views(0),
  view_array(1)
{
    view_array.grow(1)->zero();
}

//------------------------------------------
// Copy constructor dialog
//------------------------------------------

dialog::dialog(const dialog& dlg)
: diag_type(dlg.diag_type),
  title(dlg.title),
  dialog_swidget(NULL),
  detach_hook(NULL),
  detach_data(NULL),
  current_view(NULL),
  current_text(NULL),
  oper_pt(NULL),
  num_views(0),
  view_array(1)
{
    view_array.grow(1)->zero();
}

//------------------------------------------
// Destructor dialog
//------------------------------------------
dialog::~dialog()
{}

//------------------------------------------ 
// next_view
//
// Return the index of the next non-empty view slot following the given slot.
// Returns -1 if there are no remaining non-empty view slots.
//------------------------------------------

int dialog::next_view(int view_num)
{
    Initialize(dialog::next_view);

    while(++view_num < max_view_num())
	if(get_view(view_num))
	    return(view_num);

    return(-1);
}

//------------------------------------------ 
// empty_view
//
// Return the index of an empty view slot.  If no empty view slots exist,
// extend the array to provide one, so this always succeeds.
//------------------------------------------

int dialog::empty_view()
{
    Initialize(dialog::empty_view);

    for(int view_num = 0; view_num < max_view_num(); ++view_num)
	if(!get_view(view_num))
	    return(view_num);

    view_array.grow(1)->zero();

    return(max_view_num() - 1);
}

//-----------------------------------------
// Dialog set title
//-----------------------------------------
void dialog::set_title(char* t)
{
    Initialize(dialog::set_title);

    title = t;

    return;
}

//------------------------------------------
// send_string
//
// Get the current command string for the dialog.
//------------------------------------------
void dialog::send_string(ostream& strm) const
{
   Initialize(dialog::send_string);

   if(current_text)
      strm << current_text;
   strm << ends;

   return;
}

//------------------------------------------
// contains_app
//
// Function to see if the dialog has a view of the specified app
//------------------------------------------

boolean dialog::contains_app(appPtr app_header)
{
   Initialize(dialog::contains_app);

   for(int i = 0; i < max_view_num(); i++)
   {
      viewPtr viewp = get_view(i);
      if(viewp)
      {
	   ldrPtr ldrp = viewp->get_ldrHeader();
           if(ldrp && (app_header == ldrp->get_appHeader()))
	       return(true);
      }
   }

   return(false);
}

//------------------------------------------
// contains_view
//
// Function to see if the dialog owns the specified view
//------------------------------------------

boolean dialog::contains_view(viewPtr viewp)
{
   Initialize(dialog::contains_view);

   for(int i = 0; i < max_view_num(); i++)
       if(viewp == get_view(i))
	   return(true);

   return(false);
}

//------------------------------------------
// regenerate_view
//
// Function to re-extract and re-layout a view
//------------------------------------------

void dialog::regenerate_view(viewPtr viewp)
{
    Initialize(dialog::regenerate_view);

    if(!is_viewGraHeader(viewp))
	return;

    viewGraHeaderPtr view_gra = checked_cast(viewGraHeader,viewp);

    ldrPtr ldr_head = view_gra->get_ldrHeader();
    if (!ldr_head)
       return;		// view or ldr already deleted -- can't regenerate!

    viewNodePtr old_view_root = checked_cast(viewNode,view_gra->get_root());
    viewNodePtr new_view_root = view_extractor(
	view_gra, checked_cast(ldrNode,ldr_head->get_root()));
    view_gra->copy_state(old_view_root);
    view_gra->route_view();

// view_tree_deleter won't work without a header, so we have to restore
// the old tree temporarily to delete it.

    put_relation(header_of_tree, old_view_root, view_gra);
    view_gra->view_tree_deleter(old_view_root);
    put_relation (header_of_tree, new_view_root, view_gra);
    view_gra->repaint_view();

    return;
}

//------------------------------------------
// create_window
//
// Function to open a window for the specified view 
//------------------------------------------
void dialog::create_window(int view_num, void* win)
{
    Initialize(dialog::create_window);

    if(view_num < 0 || view_num >= max_view_num())
	return;

    if(win)
	get_window(view_num) = win;

    viewPtr viewp = get_view(view_num);
    if(viewp)
    {
	if (!win && isServer()) {
	       get_window(view_num) = (void *)gala_current_window;
	       dialog_set_current_window((void*)gala_current_window);
        } else
	    dialog_set_current_window(win);
	viewp->open_window((long)get_window(view_num));
	dialog_set_current_window(NULL);
    }

    return;
}

//-------------------------------------------
// send_view_to_ui
//
//   Send view to a structure understood by ui
//------------------------------------------
void dialog::send_view_to_ui(int view_num, void* /* data */)
{
   Initialize(dialog::send_view_to_ui);

   viewPtr viewp = get_view_ptr(view_num);
   if(viewp)
   {
      repType rep = repType(viewp->get_type());
      if((rep == Rep_List))
      {
	  viewListHeaderPtr view_ui = checked_cast(viewListHeader,viewp);
      }
   }

   return;
}

//--------------------------------------------
// close_window
//
// closes current window 
//
//-------------------------------------------
void dialog::close_window(int view_num)
{
    Initialize(dialog::close_window);

    viewPtr viewp = get_view_ptr(view_num);
    if(viewp) {
	viewp->close_window();
	unregister_scrollbars(view_num);
	unregister_panner (view_num);
    }

    return;
}

//------------------------------------------
// resize_window
//
// Performs the window resize function
//------------------------------------------

void dialog::resize_window(void* win)
{
    Initialize(dialog::resize_window);

    ui_resize_window((Window)win);

    return;
}

void dialog::change_view(
    void* win, int view_num, repType new_type, appTreePtr subroot)
{
    Initialize(dialog::change_view);

    viewPtr viewp = get_view_ptr(view_num);
    appMarker *marker = NULL;

    if(viewp)
    {
	current_view = NULL;		// Avoid crashes in called functions.

	repType old_type = repType(viewp->get_type());
        ldrPtr ldrp = view_get_ldr(viewp);
	ldrTreePtr ldrt = ldrp ? checked_cast(ldrTree,ldrp->get_root()) : NULL;
	appTreePtr old_subroot = ldrt ? ldrt->get_appTree() : NULL;

	if(subroot == NULL)		// Get subroot if necessary.
	    subroot = old_subroot;

	// if not necessary to regenerate view, return back
	if(new_type == old_type && subroot == old_subroot)
	    return;

        if ( is_steView(viewp) ){
           OperPoint *pt = viewp->insertion_point();
           if ( pt ){
              steViewPtr(viewp)->
                  save_cursor( checked_cast(appTree,pt->node), pt->offset_of );
              delete pt;
           }
        }
        marker = (appMarker *)get_relation(cursor_of_view,viewp);
        if ( marker )
           rem_relation(cursor_of_view, viewp, marker);

	viewp->close_window();		// Close window.

	get_view(view_num) = NULL;	// Reset viewHeader *first* to suppress
					// propagation resulting from deletion
					// of view.

	delete_view_header(viewp);	// Delete old view.
    }
    if(subroot)				// Open new view.
    {
        appPtr h = checked_cast(app,subroot->get_header());
	dialog_set_current_window(get_window(view_num));
	open_view(view_num, h, subroot, new_type);
        viewPtr v = get_view_ptr(view_num);
        if ( marker )
           put_relation(cursor_of_view, v, marker);        

        if ( v && marker && is_steView(v) ){
           int offset;
           appTreePtr nd = marker->get( offset );
           OperPoint pt(nd,(objOperType)0);
           pt.offset_of = offset;
           steViewPtr( v )->insertion_point( &pt );
        }
	create_window(view_num, win ? win : get_window(view_num));
	dialog_set_current_window(NULL);
    }

    return;
}

//------------------------------------------
// change_view
//
// Function to change specified view to new representation
//------------------------------------------

void dialog::change_view(void* win, int view_num, repType new_type)
{
    change_view(win, view_num, new_type, NULL);
}

//--------------------------------------
//
// Function to change view to new app and representation
//
//--------------------------------------
void dialog::change_view(
    int view_num, appPtr, appTreePtr subroot, repType new_type)
{
    change_view(NULL, view_num, new_type, subroot);
}

//------------------------------------------
// set_detach_hook
//
// Sets function for detach_view to call before it detaches the view,
// thus allowing the dialog's owner to do some cleanup before a view
// goes away.
//------------------------------------------

void dialog::set_detach_hook(void* data, void (*hook)(void*, int))
{
    Initialize(dialog::set_detach_hook);

    detach_hook = hook;
    detach_data = data;

    return;
}

//------------------------------------------
// detach_view
//
// Detaches view from dialog when it is being deleted.
//------------------------------------------

void dialog::detach_view(viewPtr viewp)
{
    Initialize(dialog::detach_view);

    int view_num = get_view_num(viewp);
    if(view_num >= 0)
    {
	if(detach_hook)
	    (*detach_hook)(detach_data, view_num);
	view_array[view_num]->zero();
    }

    return;
}

//------------------------------------------
// get_view_name
//------------------------------------------

char* dialog::get_view_name(int view_num)
{
    Initialize(dialog::get_view_name);

    viewPtr viewp = get_view_ptr(view_num);
    if(viewp)
    {
	ldrPtr ldrp = viewp->get_ldrHeader();
	appPtr appp = checked_cast(app,ldrp->get_appHeader());
	return(appp->get_name());
    }
    else
	return(NULL);
}

//------------------------------------------
// refresh
// Currently unimplimented
//------------------------------------------

void dialog::refresh()
{
    Initialize(dialog::refresh);
    return;
}

//------------------------------------------
// put_selection
//
//------------------------------------------

OperPoint* dialog::put_selection(viewSelection& view_sel)
{
    Initialize(dialog::put_selection);

    OperPoint* pt = new OperPoint((Obj *)checked_cast(appTree,NULL), objOperType(FIRST));

    viewTree* view_node = view_sel.view_node;
    viewTree* view_conn = view_sel.connector;


    ldrTree* ldr_node;
    ldrTree* ldr_conn;
    if (view_node)
        ldr_node = view_node->get_ldrTree();
   if( view_conn )
        ldr_conn = view_conn->get_ldrTree();
    return(pt);
}
#ifdef XXX_dialog_layout
ldrHeader* dialog_layout(symbolPtr tree, repType tp)
{
    Initialize(dialog::layout);

    symbolTreeHeader* appl = find_symbolTreeHeader (tree);
    ldrHeader* root_head = appl ?
	checked_cast (ldrHeader, ldr::find_ldrHeader (appl, tp)) : NULL;
    if (root_head)
	return root_head;
    switch (tp) {
      case Rep_Tree:
	{
	  symbolPtr tree_sym = tree.get_xrefSymbol();
	if (tree_sym->xrisnull()) {
            msg("ERROR: Cannot open call tree on selected object.") << eom;
	    break;
        }
	if (!appl) {
	    appl = db_new (symbolTreeHeader, (0));
	    RTLNode* rtl_node = checked_cast (RTLNode, appl->get_root ());
	    rtl_node->rtl_insert (tree);
	}
	root_head = db_new(symbolLdrHeader, (appl));
	break;
	}

      case Rep_MultiConnection:
      case Rep_OODT_Inheritance:
      case Rep_OODT_Relations:
      case Rep_OODT_Scope:
	root_head = db_new(ldrBrowserHierarchy, (tree));
	break;

      case Rep_DataChart:
	{
	root_head = db_new(ldrBrowserHierarchy, (tree));
	break;}

      default:
	{
	break;}
    } 
    if(root_head)
	root_head->set_type(tp);
    return root_head;
}
#endif
ldrHeader* dialog::layout(appPtr appl, appTreePtr tree, repType tp,
			  int, int /* sort_order */, ldrRTLDisplayType /* fields */)
{
    Initialize(dialog::layout);

    if (!ste_interface_get_epoch_flag()) {
	if (tp == Rep_SmtText)
	    tp = Rep_VertHierarchy;
	else if (tp == Rep_TextText)
	    tp = Rep_TextFullDiagram;
        if ( tp == Rep_RawText ) {
           msg("ERROR: Emacs is not loaded. Cannot view ascii files. To load Emacs, restart DISCOVER with the discover script.") << eom;
           return 0;
        }
    }

    ldrHeader* root_head = appl 
       ? checked_cast(ldrHeader,ldr::find_ldrHeader(appl, tp, tree))
       : NULL;

    if (root_head) 
       return root_head;

    if(appl)
    {
	switch(appl->get_type())
	{
	  case App_SMT:
	    {
            smtHeaderPtr h = checked_cast(smtHeader,appl);
 	    if(!h->parsed){	// Make tree for header
 	      h->make_tree(); // (import/restore)
 	      tree = checked_cast(appTree,appl->get_root());
 	    }

	    if ( (tp == Rep_SmtText || tp == Rep_TextText) &&
		 (!ste_interface_get_epoch_flag()) )
		tp = Rep_TextDiagram;

            smtTreePtr smt_tree = checked_cast(smtTree,tree);
	    switch(tp)
	    {
	      case Rep_SmtText:
// next line always creates SmodShort source view 
		{
                root_head = db_new(ldrSmodShort,(h, smt_tree));

		put_relation(ste_opened_document, ste_main, appl);
		break; }
		
	      case Rep_TextText:
		{
		msg("shouldn\'t have gotten here - Fix me\n") << eom;
		root_head = NULL;
		break; }

	      case Rep_MultiConnection:
	      case Rep_OODT_Inheritance:
	      case Rep_OODT_Relations:
	      case Rep_OODT_Scope:
		root_head = db_new(ldrBrowserHierarchy, (tree));
		break;

	      case Rep_FlowChart:
		{
		root_head = db_new(ldrSmtFlowchart,(smt_tree));
		objTree*root = root_head->get_root();
		if(!(root && root->get_first())){
                  msg("WARN: $1: empty file. Cannot build Flowchart view.") << appl->get_name() << eom;
		  obj_delete (root_head);
		  root_head = NULL;
		} else {
		    put_relation(ldr_of_app, appl, root_head);
		}
		break;}

             default:
		{
		root_head = db_new(ldrSmtHierarchy,(h, smt_tree));
		put_relation(ldr_of_app, appl, root_head);
		break; }
	    }
	    break;
	    }  

	  case App_STE:
	    {
            Assert(0);
	    break;
	    }
          case App_RAW:
	    {
            objRawApp *hd = checked_cast(objRawApp,appl);
            objRawLdr *ld_hd = new objRawLdr(hd,(void *)NULL);
            root_head = checked_cast(ldrHeader,ld_hd);
            break;}

	  case App_DD:
	   {
	      ddElementPtr dd = checked_cast(ddElement, tree);
	      if (dd->get_kind() == DD_CLASS) {
		 root_head = db_new(ldrBrowserHierarchy, (tree));
	      }
	      else msg("ERROR: $1 is not a class and cannot be opened into an inheritance view") << dd->get_name() << eom;
	      break;
	   }

	  case App_RTL:
	    {
	    switch(tp)
	    {
	      case Rep_DGraph:
		{ root_head = new ldrDGraph(appl); }
		break;

	      case Rep_List:
	      default:
		{
		root_head = checked_cast(ldrHeader,db_new(
		    ldrList, (checked_cast(RTL,appl))));
		put_relation(ldr_of_app, appl, root_head);
		break; }
	    }
	    break;}

	  case App_SCRAPBOOK:
	    {
	    root_head = db_new(
		ldrScrapbook, (checked_cast(scrapbook,appl), checked_cast(clipboard,tree)));
	    put_relation(ldr_of_app, appl, root_head);
	    break;}

	  default:
	    break;
	}
    }
    else if(tp == Rep_ERD) {
        symbolArr arr;
	root_head = db_new(ldrERDHierarchy, (arr));
    }
    else if (tp == Rep_DataChart) {
        symbolArr arr;
	root_head = db_new(ldrDCHierarchy, (arr));
    }

    if(root_head)
	root_head->set_type(tp);

    return(root_head);
}

// Returns view number for a view header;
// Negative return value is an error
int dialog::get_view_num(viewPtr viewp)
{
    Initialize(dialog::get_view_num);

    if(viewp)
    {
	for(int i = 0; i < max_view_num(); i++)
	{
	    if(viewp == get_view(i))
		return(i);
	}
    }
    return(-1);
}

// Returns viewPtr for a view num.
viewPtr dialog::get_view_ptr(int view_num)
{
    Initialize(dialog::get_view_ptr);

    if(view_num < 0 || view_num >= max_view_num())
	return(NULL);
    else
	return(get_view(view_num));
}

// Return parent window of a view.
void* dialog::window_of_view(viewPtr viewp)
{
    Initialize(dialog::window_of_view);

    int view_num = get_view_num(viewp);
    if(view_num >= 0)
	return(get_window(view_num));
    else
	return(NULL);
}

/*
   START-LOG-------------------------------------------

   $Log: dialog.h.cxx  $
   Revision 1.41 2000/12/01 14:06:47EST ktrans 
   More conversions of old messages to msg()


   END-LOG---------------------------------------------

*/



