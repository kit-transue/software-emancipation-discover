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
// driver
//------------------------------------------
// synopsis:
// The main command driver for Alpha-set
//
// description:
// The driver contains the global selection stack
// and a pointer to all dialogs currently in
// progress.  Selections are processed by calling
// the put_selection method.  The commands themselves
// can be called directly:  they take no arguments, and
// work from the context of the selection stack and
// current dialog.
//

//------------------------------------------
// Restrictions:
// there is only one instance of this class instantiated at a time.
//------------------------------------------

#include <cLibraryFunctions.h>
#include <objOper.h>
#include <appMarker.h>
#include <steView.h>
#include <steBuffer.h>
#include <viewerShell.h>
#include <Object.h>
#include <genError.h>
#include <graSymbol.h>
#include <OperPoint.h>
#include <ldrNode.h>
#include <ldrSelection.h>
#include <viewNode.h>
#include <graWorld.h>
#include <dialog.h>
#include <driver.h>
#include <button.h>
#include <smt.h>
#include <SmtSte.h>
#include <ddict.h>
#include <steTextNode.h>
#include <ste_interface.h>
#include <ste_cut.h>
#include <ste_array_handler.h>
#include <ste_category_handler.h>
#include <ste_build_ref.h>
#include <reference.h>
#include <scrapbook.h>
#include <viewGraHeader.h>

#ifndef _browserShell_h
#ifdef _WIN32
#define POINTS ___POINTS
#define min    ___min
#define max    ___max
#endif
#include <browserShell.h>
#ifdef _WIN32
#undef POINTS
#undef min
#undef max
#endif
#endif

#include <messages.h>
#include <customize.h>

#ifndef _WIN32
#include <sys/param.h>
#endif

#include <style_hooks.h>
#include <ste_smod.h>
#include "cmd.h"
#include <machdep.h>
#include <steEpochList.h>
#include <miniBrowser.h>

#ifdef NEW_UI
#ifndef _mytimer_h
#include <mytimer.h>
#endif

#include <vport.h>
#include veventHEADER

#undef printf
#undef sprintf
#endif
#include "tcl.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <mpb.h>
int cmd_level();
#define cmd_dr_gen if(cmd_level()==1)cmd_gen
// local typedefs

typedef void (driver::* driver_command_ptr) ();

void enable_reparse_buttons(smtHeaderPtr smt_header, boolean enabled);

// local static variables

static driver_command_ptr dispatch_tab[eventType_size+1][eventModifier_size];

static appTreePtr opened_node;


// global variables
driverPtr driver_instance;  // The global driver instance


// Define relational stuff

init_relational(driver,Relational);

// function to work around non virtual get_header method.
inline viewPtr get_view_header(viewTreePtr vw_nd)
{
    Initialize(get_view_header);

    return is_steButton(vw_nd) ?
	checked_cast(view,(checked_cast(steButton,vw_nd))->get_header()) :
        checked_cast(view,vw_nd->get_header());
}

//------------------------------------------
// Constructor driver
//------------------------------------------

driver::driver () : graphics_world(0) { }

driver::driver( int argc, char* argv[] )
{
    Initialize(driver::driver);

    genError* err;

    genTry {
	graphics_world = new graWorld (argc, argv);
    } genCatch (err) {
        graphics_world = 0;
	cout << "Could not initialize graphics." << endl;
    }
    genEndtry;

    current_dialog = checked_cast(dialog, NULL);
    current_oper_pt = (OperPoint*) NULL;

    int ii, jj;
    for (ii=0; ii < eventType_size; ii++) {
	for (jj=0; jj < eventModifier_size; jj++) {
	    dispatch_tab[ii][jj] = NULL;
	}
    }

   // Left Mouse

    dispatch_tab[eventType_Left][eventMod_Down] =
	driver_command_ptr(&driver::select);
    dispatch_tab[eventType_Left][eventMod_Down|eventMod_Shift] =
	driver_command_ptr(&driver::addselect);

    // Right Mouse

    dispatch_tab[eventType_Right][eventMod_Down] =
	driver_command_ptr(&driver::addselect);
    dispatch_tab[eventType_Right][eventMod_Up] = 
	driver_command_ptr(&driver::addselect);

    char path[MAXPATHLEN];
    strcpy (path, customize::install_root());
    strcat (path, "/lib/global.xref");
}


//------------------------------------------
// Destructor driver
// //------------------------------------------

driver::~driver()
{}

//------------------------------------------
// get_file_xref 
//
// Gets the global file alias table
//------------------------------------------



//-----------------------------------------
// close_dialog
//
//  close already opened dialog
//-----------------------------------------

void driver::close_dialog(dialogPtr dlg)
{
    Initialize(driver::close_dialog);
    
    for(int i = dlg->first_view(); i >= 0; i = dlg->next_view(i))
	dlg->close_view(i);
    
    // current oper point is null; This should probably happen in
    // dialog destructor
    if (current_dialog == dlg)
         current_dialog = NULL;

    dialogs.remove(dlg);
    delete dlg;
}



//------------------------------------------
// regenerate_view
//
// Function to regenerate of all views within a dialog
//------------------------------------------

void driver::regenerate_view( viewPtr vw )
{
    Initialize(driver::regenerate_view);

    dialogPtr dialog = find_dialog_for_view (vw);
    if(dialog)
	dialog->regenerate_view (vw);
}

//------------------------------------------
// detach_view
//
// Detaches view from dialog since it is being
// deleted
//------------------------------------------

void driver::detach_view ( viewPtr vw )
{
    Initialize(driver::detach_view);
    
    // find dialog for this view
    dialog *dlg = find_dialog_for_view (vw);
    
    // detach view from the dialog
    if (dlg)
	dlg->detach_view (vw);
}

//------------------------------------------
// contains_dialog
//
// Funtion to see if the given dialog is valid
//------------------------------------------

boolean driver::contains_dialog( dialogPtr dlg )
{
    Initialize(driver::contains_dialog);

    Obj* el;
    ForEach (el, dialogs)
	if ( checked_cast(dialog,el) == dlg )
	    return true;
    
    return false;
}

//------------------------------------------
// dispatch
//
// Function to dispatch a command
//------------------------------------------

void driver::dispatch( button& but )
{
    Initialize(driver::dispatch);

    driver_command_ptr command = dispatch_tab[but.which][but.type];
    
    if ( command )
	(this->*command)();
}


//------------------------------------------
// put_selection
// 
// Funtion to handle appliction tree selection
//------------------------------------------

SelectionResult driver::put_selection( view& vw, button& but, 
				      viewSelection& view_sel)
{
    Initialize(driver::put_selection);

    DBG{
       viewTree * node = view_sel.view_node;
       viewTree * conn = view_sel.connector;
       int sub = view_sel.sub_type;
       
       viewTree * root = (viewTree*) obj_tree_common_root(node, conn);

       cout << " ---- PUT_SELECTION view " << (&vw ? vw.get_name() : "null")
         << " but " << but.type << ' ' << but.which << 
          " node " << node << " conn " << conn << " sub " << sub << endl;

    }

    ldrSelection ldr_sel;
    

    if ( &vw == NULL || view_sel.view_node == NULL ) {
	current_oper_pt = (OperPoint *)NULL;
    } else {
	current_oper_pt = new OperPoint (NULL_symbolPtr,FIRST,0,0,0,view_sel.view_node);
	
	viewTree* view_node = view_sel.view_node;
	viewTree* view_conn = view_sel.connector;
	
	ldr_sel.ldr_symbol    = NULL;
	ldr_sel.ldr_connector = NULL;
	ldr_sel.subtype       = 0;
	
	if (view_node != NULL) {
	    ldr_sel.ldr_symbol = checked_cast(ldrSymbolNode, view_node->get_ldrTree ());
	}
	
	if (view_conn != NULL) {
	    ldr_sel.ldr_connector = 
		checked_cast(ldrConnectionNode, view_conn->get_ldrTree ());
	}
	
	ldrHeader* ldr_head = checked_cast(ldrHeader, vw.get_ldrHeader ());
	
//      obsolete view ?
	if (ldr_sel.ldr_symbol == NULL && ldr_sel.ldr_connector == NULL){
            current_oper_pt = (OperPoint *)NULL;
            dispatch (but);
            dis_message (C_DRIVER, MSG_ERROR, "M_INCOMPLETE");

            return selACCEPT;
        }
	ldr_head->build_selection (ldr_sel, *current_oper_pt);
	
	// get subnode
        
	if (!view_sel.sub_node && is_viewSymbolNode (view_node)) {
	    
	    // get the picture
	    graParametricSymbol *picture =
		(checked_cast(viewSymbolNode, view_node))->get_picture();
	    
	    // and the subnode
	    if (picture) {
		view_sel.sub_node = checked_cast(appTree, picture->get_modified_token());
	    }
	}
	
	current_oper_pt->subnode = view_sel.sub_node;
		// picture->hilite_modification_text();

	// fix this ldr bug here for now... 
	// fix this ldr bug fix here for now ...
	// do not overwrite type - this is done in build_selection
	current_dialog = find_dialog_for_view (&vw);
    }
    dispatch (but);
    
    return selACCEPT;
}


//------------------------------------------
// clear_selection
//
// Function to clear all selected nodes
//------------------------------------------

void driver::clear_selection (bool including_browser_selection)
{
    Initialize(driver::clear_selection);

    start_transaction(){
	opened_node = NULL;

	Obj* el;
	ForEach (el, dialogs)
	    clear_selection (checked_cast(dialog, el));
    
	if(current_dialog != NULL)
	    current_dialog->set_string(NULL);
    } end_transaction();

    if (including_browser_selection){
#ifndef NEW_UI
        browserShell::clear_all_browser_selections ();
	miniBrowser::clear_selections();
#else
  sel_list.remove_all();
#endif
    }
    
    ste_interface_unselect ((viewPtr)NULL);

     if(cmd_begin()){
         cmd_dr_gen("dr_clear_selection();");
     }
     cmd_end();

}
extern "C" void dr_clear_selection()
{
  driver_instance->clear_selection();
}

void driver::clear_selection(dialogPtr dlg)
{
    Initialize(driver::clear_selection);

    start_transaction() {
	for(int i = dlg->first_view(); i >= 0; i = dlg->next_view(i))
	    unselect(dlg->get_view_ptr(i));
    } end_transaction();
}


//------------------------------------------
// move_node
//
// Function to move one node in a tree to another spot. The node is converted
// to text, removed and then inserted
//------------------------------------------

view * cmd_get_cur_view();
void cmd_record_view(view*);

struct cmd_transaction{
 int on;
 cmd_transaction() : on(cmd_begin()) {}
 ~cmd_transaction() {cmd_end();}
};

void driver::move_node()
{
    Initialize(driver::move_node);

cmd_transaction ct;

    cmd_dr_gen("dr_move_node();");

    OperPoint*  dest;
    OperPoint*  source;
    viewTreePtr vw_nd;
    viewTreePtr src_view_node;
    viewTreePtr dest_view_node;
    genError *err;
    view *view_head;
    ldr *ldr_head;
    app *app_head;
    
    dest = sel_list.pop_pt ();
    source = sel_list.pop_pt ();
    
    clear_selection();
    
    if ( dest == NULL || source == NULL )
	return;
    
    src_view_node = vw_nd = checked_cast(viewTree, source->view_node);
    if (vw_nd != checked_cast(viewTree,NULL))
	if (is_viewSymbolNode (vw_nd))
	    checked_cast(viewSymbolNode,vw_nd)->unselect();
	    
    dest_view_node = vw_nd = checked_cast(viewTree, dest->view_node);
    if (vw_nd != checked_cast(viewTree,NULL))
	if (is_viewSymbolNode (vw_nd))
	    checked_cast(viewSymbolNode,vw_nd)->unselect();

    view_head = (checked_cast(viewTree,dest->view_node))->get_header();
    ldr_head = view_head->get_ldrHeader();
    app_head = ldr_head->get_appHeader();

    // Test for moving under itself
    appTreePtr  dest_node = dest->node;
    objTree *wnode = (dest->type == AFTER) ? dest_node->get_parent() :
    dest_node;

    // Now wnode is future parent of moveing subtree
    appTreePtr source_node = source->node;
    if(!wnode->subtree_of(source_node)) {
        view_head = src_view_node->get_header();
	genTry {
	    // expand if we need to
	    viewSymbolNodePtr symb = NULL;
	    if (is_viewSymbolNode (src_view_node)) {
		symb = checked_cast(viewSymbolNode,src_view_node);
		if (symb->viewIsA ( viewGroupType )) {
		    view *hd = symb->get_header();
		    hd->expand (symb, -1);
		}
	    }
    
	    if (is_viewSymbolNode (dest_view_node)) {
		symb = checked_cast(viewSymbolNode,dest_view_node);
		if (symb->viewIsA ( viewGroupType )) {
		    view *hd = symb->get_header();
                    hd->expand (symb, -1);
                }
            }
    
	    obj_insert (app_head, dest->type, source_node, dest_node,
			NULL);
	} genCatch(err) {
		this->error();
	}
	genEndtry;
    }
    
    delete dest;
    delete source;
}
// 
// Fills specified array with selected app nodes in the specified view.
// Null view implies all views.
void driver::fill_array_with_selected_appnodes
                  (viewPtr v, objArr& sel_arr, int subnodes)
{
    Initialize(driver::fill_array_with_selected_appnodes);
    
    Obj*        el;
    OperPoint*  pt;
    viewTreePtr vw_nd;

    /* collect all nodes from steEpochList type views
       (selection is handled dirrerently for these views) */
    symbolArr temp; 
    steEpochList::fill_selected_nodes(v, temp);
    miniBrowser::fill_selected_nodes(temp);
    symbolPtr symp;
    ForEachS(symp, temp) {
       sel_arr.insert_last((Obj *)symp);
    }
    
    /* selection from all other views */
    ForEach (el, sel_list.oper_pts) {
	
	if (el != NULL) {
	    pt = (OperPoint*)el;
	    
	    vw_nd = checked_cast(viewTree, pt->view_node);
	    if (vw_nd) {
		if (v == NULL || v == get_view_header(vw_nd)) {
		    if (subnodes && (int)pt->subnode)
			sel_arr.insert_last ((Obj *)pt->subnode);
		    else
			sel_arr.insert_last ((Obj *)pt->node);
		}
	    } else if (v == NULL)
		sel_arr.insert_last ((Obj *)pt->node);
	}
    }
}

void driver::fill_array_with_selected_appnodes                  (viewPtr view, symbolArr& sel_arr, int subnodes)
{
    Initialize(driver::fill_array_with_selected_appnodes);
    
    Obj*        el;
    OperPoint*  pt;
    viewTreePtr vw_nd;

    /* collect all nodes from steEpochList type views
       (selection is handled dirrerently for these views) */
    steEpochList::fill_selected_nodes(view, sel_arr);
    miniBrowser::fill_selected_nodes(sel_arr);
    
    /* selection from all other views */
    ForEach (el, sel_list.oper_pts) {
	
	if (el != NULL) {
	    pt = (OperPoint*)el;
	    
	    vw_nd = checked_cast(viewTree, pt->view_node);
	    if (vw_nd) {
		if (view == NULL || view == get_view_header(vw_nd)) {
		    if (subnodes && (int)pt->subnode)
			sel_arr.insert_last ((Obj *)pt->subnode);
		    else
			sel_arr.insert_last ((Obj *)pt->node);
		}
	    }
	    else if (view == NULL)
		sel_arr.insert_last ((Obj *)pt->node);
	}
    }
}

// Fills array view-nodes that are selected in view.
// Null view implies all views.
void driver::selected_view_nodes(viewPtr view, objArr& array)
{
    Initialize(driver::selected_view_nodes);
    
    Obj* el;
    ForEach(el, sel_list.oper_pts)
    {
	if(el)
	{
	    viewTreePtr view_node = checked_cast(viewTree,((OperPoint*)el)->view_node);
	    if(view_node)
	    {
		if(view == NULL || view == get_view_header(view_node))
		    array.insert_last(view_node);
	    }
	}
    }
}

// removes an aset object
// this method will be called from UI to
// remove any alphaset object (both header and tree)

void driver::remove_object (commonTreePtr aset_obj)
{
    Initialize(driver::remove_object);
    start_transaction() {
	if (aset_obj && aset_obj->get_id() > 0)
	{
	    if(is_Hierarchical(aset_obj) 
          /*   || is_SoftAssoc(aset_obj) || is_SoftId(aset_obj) */
	       )
		obj_delete(aset_obj);
	    else
	    {
		app * head = checked_cast(app,aset_obj->get_header());
		if(head){
		    if ( is_smtHeader(head) ){
			smtHeaderPtr h1 = checked_cast(smtHeader,head);
			appTreePtr ap_t = checked_cast(appTree, aset_obj);
			h1->suspend_parsing();
			enable_reparse_buttons( h1, 1 );
			obj_insert(h1, REMOVE, ap_t, 0, 0, 0);
		    }else
			obj_remove(head, checked_cast(appTree, aset_obj));
		}else
		    obj_delete(aset_obj);
	    }
	}
    } end_transaction ();
}

//------------------------------------------
// delete
//
// delete selected objects in a specified view.
// delete all selected objects if specified view
// is null.
//------------------------------------------

void driver::del (viewPtr view)
{
    Initialize(driver::del);
    
    objArr nodes_to_remove;
    
    opened_node = NULL;
    
    start_transaction() {
	update_selection (view, 0);
    
	fill_array_with_selected_appnodes (view, nodes_to_remove);
    
	// unselect these objects
	unselect (view);
    
	Obj *el1;

	ForEach (el1, nodes_to_remove) {
	    remove_object ( checked_cast(commonTree,el1) );
	}
    } end_transaction();
}

//------------------------------------------
// cut
//
// Cut selected objects in a specified view.
// Cut all selected objects if specified view
// is null.
//------------------------------------------

void driver::cut (viewPtr vvv, viewTreePtr node)
{
    Initialize(driver::cut);

    if( vvv && is_steView(vvv) )
    {
	steBufferPtr b = checked_cast(steBuffer,get_relation(view_buffer,vvv));
	if(b)
	{
	    genString kill;
	    kill.printf("(%s %d)", "di-kill-selections", b->get_buffer());
	    ste_epoch_evaluate(kill);
	}
	return;
    }
    
    cmd_transaction ct;
    if(ct.on){
      IF(node);
      cmd_record_view(vvv);
      cmd_dr_gen("dr_cut();");
    }

    objArr      nodes_to_cut;
    
    opened_node = NULL;
    
    start_transaction(){
	if (node == NULL) {
	    update_selection (vvv, 0);

	    fill_array_with_selected_appnodes (vvv, nodes_to_cut);

	    for (int i=0; i < nodes_to_cut.size();)
	    {
		if (! (is_steTextNode (RelationalPtr(nodes_to_cut[i])) ||
		       is_smtTree (RelationalPtr(nodes_to_cut[i]))) )
		    nodes_to_cut.remove ((Obj *) nodes_to_cut[i]);
		else
		    i++;
	    }
	} else {
	    ldrTreePtr ldr_tree = node->get_ldrTree();
	    appTreePtr app_tree = ldr_tree ? ldr_tree->get_appTree() : NULL;
	    if (!app_tree)
	    {
		ldr_tree = checked_cast(ldrTree,ldr_tree->get_parent());
		app_tree = ldr_tree ? ldr_tree->get_appTree() : NULL;
	    }
	    if (app_tree)
	    {
		if (is_ddElement(app_tree))
		    app_tree = checked_cast(appTree,get_relation(def_smt_of_dd, app_tree));

		if (is_smtTree(app_tree) || is_steTextNode(app_tree))
		    nodes_to_cut.insert_last (app_tree);
	    }
	}

	if (nodes_to_cut.size())
	    scrapbook_instance->cut (nodes_to_cut, 0);
	else
	    dis_message (C_DRIVER, MSG_ERROR, "M_NOSELECTIONCUT");

	if ( !node )
	    unselect (vvv);
	
    } end_transaction();
}

extern void smt_paste_region(smtTreePtr src, int off, smtTreePtr trg);

// paste previously cut object
void driver::_paste (viewPtr vvv)
{
    Initialize(driver::_paste);
    
    if (!vvv)
	return;

    if( vvv && is_steView(vvv) )
    {
	genString yank("(di-yank-selections)");
	ste_epoch_evaluate(yank);
	return;
    }

 cmd_transaction ct;
    if(ct.on){
      cmd_record_view(vvv);
      cmd_dr_gen("dr_paste();");
    }

    ldrPtr ldrp = checked_cast(ldr, vvv->get_ldrHeader());
    appPtr appp = checked_cast(app, ldrp->get_appHeader());
    steViewPtr v = (is_steView(vvv)) ? steViewPtr(vvv) : NULL;

    objArr *temp_arr = NULL;
    start_transaction(){  
	if (!(is_smtHeader(appp) || is_steDocument (appp))) {
	    dis_message (C_DRIVER, MSG_ERROR, "M_CANNOTPASTE");
	    return;
	}

	// get objects to paste
	temp_arr = scrapbook_instance->objects_to_paste (appp);

	if (!temp_arr) {
	    dis_message (C_DRIVER, MSG_ERROR, "M_NOTHINGONCLIPB");
	    return;
	}

	// paste them
	Obj *el;
	int smt_first_flag = 0;
        start_smt_editing();
        appTreePtr trg_el = NULL;
        appTreePtr src_el = NULL;
        OperPoint *curr_pt = NULL;
	ForEach (el, *temp_arr) {
	    if (is_appTree ( (Relational *) el) ) {
                src_el = checked_cast(appTree, el);
		// get current point
                if ( v ){
		    if ( trg_el && is_smtTree(trg_el) && is_smtTree(src_el) ){
                        curr_pt = v->ste_insertion_point(src_el, curr_pt);
                        trg_el = checked_cast(appTree,src_el->get_last_leaf());
		    }else{
                        curr_pt = v->ste_insertion_point(src_el);
                        if ( curr_pt && is_smtTree(curr_pt->node) &&
			     is_smtTree(src_el) ){
			    smt_first_flag = 1;
			    trg_el = checked_cast(appTree,src_el->get_last_leaf());
                        }
		    }
		}else
		    curr_pt = vvv->insertion_point();
		    
		// insert at point
		if (curr_pt) {
		    if (smt_first_flag == 1) 
			smt_first_flag = 0;

		    insert_at (curr_pt, src_el);

		    if ( !trg_el ){
			delete curr_pt;
			curr_pt = NULL;
		    }else{
			curr_pt->node = checked_cast(appTree,trg_el);
			curr_pt->type = AFTER;
		    }
		}
	    }
	}	    
        commit_smt_editing();
        if (curr_pt)
	    delete curr_pt;

	if ( src_el && v ){
	    OperPoint pt_pnt( src_el, FIRST );
            pt_pnt.offset_of = -1;
	    v->insertion_point( &pt_pnt );
	}
        if (v)
	    unselect (v);
    } end_transaction();  
    delete temp_arr;
}

extern void  ste_smt_send_native( smtTreePtr, ostream&);
// insert object at specified oper point
void driver::insert_at (OperPoint *pt, appTreePtr obj)
  {
  Initialize(driver::insert_at);
  if (!pt)
      return;
  appTreePtr appt = pt->node;
  if (!appt)
      return;

  appPtr hd = checked_cast(app,appt->get_header());

  if ( !hd )
     return;

  if ( is_smtHeader( hd ) )
      (checked_cast(smtHeader,hd))->suspend_parsing();

  start_transaction() {
    if ( is_smtTree( obj ) && is_smtTree( appt ) ) 
 	obj_insert(hd, pt->type, obj, appt, NULL, (void *)pt);
    else if (is_steTextNode(appt))
      {
      appTreePtr new_obj = obj->app_to_ste (); 
      if(new_obj)
 	obj_insert(hd, pt->type, new_obj, appt, NULL, (void *)pt);
      else
	{
//	obj_delete(obj); // Faild to convert to ste node;
	ostrstream s;
        if ( is_smtTree( obj ) ){
           s << "\n\n";
           ste_smt_send_native( checked_cast(smtTree,obj), s);
        }else
	    obj->send_string(s);

	s << ends;
	char * txt = s.str();
 	obj_insert(hd, pt->type, 0, appt, txt, (void *)pt);
	delete txt;
	}
      }
    else if (is_smtTree (appt))
      {
      if (is_smtRefNode(obj))
        {
	insert_reference_at_smt (pt, obj);
	return;
	}
      viewTreePtr vn = checked_cast(viewTree, pt->view_node);
      if (vn)
        {
	view *vw = get_view_header (vn);
	if (!is_smtTree (obj))
	  {
	  ostrstream s;
	  obj->send_string (s);
	  s << ends;
	  char *buf = s.str();
//	  obj_delete (obj);
          obj_insert (checked_cast(app,appt->get_header()), pt->type, NULL, appt, buf);
	  if (buf) delete buf;
	  }
        else
	  {
	  obj = checked_cast(appTree,obj->get_first());	// go to real root
          obj_insert (checked_cast(app,appt->get_header()), pt->type, obj, appt, 0);
	  }
	}
      }
    else
      obj_insert (checked_cast(app,appt->get_header()), pt->type, obj, appt, NULL);
    }
  end_transaction();
  }

// copy selected object
void driver::copy (viewPtr view)
{
    Initialize(driver::copy);

    if( view && is_steView(view) )
    {
	steBufferPtr b = checked_cast(steBuffer,get_relation(view_buffer,view));
	if(b)
	{
	    genString copy;
	    copy.printf("(%s %d)", "di-copy-selections", b->get_buffer());
	    ste_epoch_evaluate(copy);
	}
	return;
    }

 cmd_transaction ct;
    if(ct.on){
      cmd_record_view(view);
      cmd_dr_gen("dr_copy();");
    }

    objArr nodes_to_copy;
    
    opened_node = NULL;
    
    start_transaction() {
	update_selection (view, 0);
    
	fill_array_with_selected_appnodes (view, nodes_to_copy);

	for (int i=0; i < nodes_to_copy.size();)
	{
	    if (! (is_steTextNode (RelationalPtr(nodes_to_copy[i])) ||
		   is_smtTree (RelationalPtr(nodes_to_copy[i]))) )
		nodes_to_copy.remove ((Obj *) nodes_to_copy[i]);
	    else
		i++;
	}
	if (nodes_to_copy.size())
	    scrapbook_instance->copy(nodes_to_copy, 0);
	else
	    dis_message (C_DRIVER, MSG_ERROR, "M_NOSELECTIONCOPY");
    } end_transaction ();
}

// Put reference to the selected objects on the scrapbook
void driver::reference (viewPtr view_p)
{
    Initialize(driver::reference);
    
    objArr selected_nodes;
    start_transaction(){
//    // Structured selections only for now
	update_selection (view_p, 0);
	fill_array_with_selected_appnodes (NULL, selected_nodes);

	boolean success = 0;
//    // restricted to a single structured node for now
//    if (selected_nodes.size() == 1)
    {
	// Check the type of the single node (STE or SMT only)
	RelationalPtr relational_node = RelationalPtr(selected_nodes[0]);
	if (is_steTextNode(relational_node))
	{
//		// Pass in the array containing the single node
	    scrapbook_instance->reference(selected_nodes);
	    success = 1;
	}
	else if (is_smtTree(relational_node))
	{
//		// Pass in the array containing the single node
	    scrapbook_instance->reference(selected_nodes);
	    success = 1;
	}
    }

	if (success)
	{
	    // clear selection here
	    unselect ( checked_cast(view, NULL) );
	}
	else
	    dis_message (C_DRIVER, MSG_ERROR, "M_NOSELECTIONCOPY");

    } end_transaction();

    return;
}

// move selected object to the specified view.
void driver::move (viewPtr view)
{
    Initialize(driver::move);

 cmd_transaction ct;
    if(ct.on){
      cmd_record_view(view);
      cmd_dr_gen("dr_move();");
    }

    
    objArr nodes_to_move;
    
    start_transaction(){
	update_selection (view, 0);
    
	fill_array_with_selected_appnodes (view, nodes_to_move);
    
	// use ste cut/paste mechanism
	if (nodes_to_move.size() > 0) {
	
	    Obj *el1;
	
	    ForEach (el1, nodes_to_move) {
		ste_insert_at_point (checked_cast(appTree,el1),
				     checked_cast(steView, view));
	    }
	}
    }end_transaction();
}

// replace selected object in this view with new one
void driver::replace (viewPtr, RelationalPtr)
{
    Initialize(driver::replace);
}

// assign category to selected objects 
void driver::assign_category (viewPtr v, steSlotPtr slot)
{
    Initialize(driver::assign_category);
    start_transaction () {
	update_selection( v, 0 );
	objArr selected_nodes;
	fill_array_with_selected_appnodes( v, selected_nodes );
	smod_put_vision( selected_nodes );
	if (selected_nodes.size() > 0) {
	    unselect ( v );
	
	    Obj *el1;
            appTreePtr an;
            char *nm = ( slot ) ? slot->get_name() : 0 ;
	    ForEach (el1, selected_nodes) {
		an = checked_cast(appTree,el1);
		style_manual_put( an, nm, 1 );
	    }

	    // set app modified  --boris
	    appPtr h = NULL;
	    ldrPtr lh = view_get_ldr(v);
	    if ( lh )
		h = ldr_get_app( lh );

	    if ( h ) h->set_modified();
	}
    } end_transaction ();
}

// deassigns specified category on selected objects
// if category is null, deassign visible category on
// selected objects.
void driver::deassign_category (viewPtr v, steSlotPtr slot)
{
    Initialize(driver::deassign_category);
    start_transaction () {
	update_selection( v, 0 );
	objArr selected_nodes;
	fill_array_with_selected_appnodes( v, selected_nodes );

        appTreePtr an;	
        char *nm = ( slot ) ? slot->get_name() : 0 ;
        appPtr h = NULL;
	ldrPtr lh = view_get_ldr(v);
	if ( lh )
	   h = ldr_get_app( lh );

	if (selected_nodes.size() > 0) {
	    unselect( v );
	    Obj *el1;
	    ForEach (el1, selected_nodes) {
		an = checked_cast(appTree,el1);
		style_manual_remove( an, nm, 1 );
	    }

	    // set app modified  --boris
	    if (h) h->set_modified();
	}else{
            OperPoint *pt = v->insertion_point();
            if (pt) {
               if (!pt->node.isnull()){
                  an = checked_cast(appTree,pt->node);
                  int res = style_manual_find_remove( an, nm, 1);
                  if ( res )
                     if (h) h->set_modified();
               }
               delete pt;
            }
        }
    } end_transaction();
}

// assign style to selected objects 
void driver::assign_style (viewPtr v, steSlotPtr slot)
{
    Initialize(driver::assign_style);
    start_transaction(){
	update_selection( v, 0 );
	objArr selected_nodes;
	fill_array_with_selected_appnodes( v, selected_nodes );
	if (selected_nodes.size() > 0) {
	    smod_put_vision( selected_nodes );
	    unselect ( v );
	    Obj *el1;
            appTreePtr an;
            char *nm = ( slot ) ? slot->get_name() : 0 ;
	    ForEach (el1, selected_nodes) {
                an = checked_cast(appTree,el1);
                style_manual_put( an, nm, 0 );
	    }
	    // set app modified  --boris
	    appPtr h = NULL;
	    ldrPtr lh = view_get_ldr(v);
	    if ( lh )
		h = ldr_get_app( lh );

	    if ( h ) h->set_modified();
	}
    } end_transaction ();
}

// deassigns specified style on selected objects
// if style is null, deassign visible style on
// selected objects.
void driver::deassign_style (viewPtr v, steSlotPtr slot)
{
    Initialize(driver::deassign_style);
    start_transaction(){
	update_selection( v, 0 );

	objArr selected_nodes;
	fill_array_with_selected_appnodes( v, selected_nodes );

        appTreePtr an;
        char *nm = ( slot ) ? slot->get_name() : 0 ;
        appPtr h = NULL;
	ldrPtr lh = view_get_ldr(v);
	if ( lh )
	   h = ldr_get_app( lh );

	if (selected_nodes.size() > 0) {
	    unselect( v );
	    Obj *el1;
	    ForEach (el1, selected_nodes) {
		an = checked_cast(appTree,el1);
		style_manual_remove( an, nm, 0 );
	    }

	    // set app modified  --boris
	    if ( h ) h->set_modified();
	}else{
            OperPoint *pt = v->insertion_point();
            if (pt) {
               if (!pt->node.isnull()){
                  an = checked_cast(appTree,pt->node);
                  int res = style_manual_find_remove( an, nm, 0);
                  if ( res )
                     if (h) h->set_modified();
               }
               delete pt;
            }
        }
    } end_transaction();
}

static void fill_array_with_nonRTL_objects(
    viewPtr view, objArr& sel_arr, int subnodes)
{
    Initialize(fill_array_with_nonRTL_objects);

    Obj*        el;
    OperPoint*  pt;
    viewTreePtr vw_nd;
    viewPtr vw;
    
    ForEach (el, driver_instance->sel_list.oper_pts) {
	
	if (el != NULL) {
	    pt = (OperPoint*)el;
	    
	    vw_nd = checked_cast(viewTree, pt->view_node);
	    if (vw_nd != checked_cast(viewTree,NULL)) {
		if (!(vw = get_view_header(vw_nd))) continue;
		if (view == NULL || view == vw) {
		    switch (vw->get_type()) {
		      case Rep_Grid:
			break;

		      default:
			if (subnodes && (int)pt->subnode)
			    sel_arr.insert_last ((Obj *)pt->subnode);
			else
			    sel_arr.insert_last ((Obj *)pt->node);
			break;
		    }
		}
	    }
	}
    }
}

// Replace non leaf nodes with first leaf;
void leaf_checking( objArr &ar ){
 Initialize(leaf_checking);
   int sz;
   if ( ar.size() > 0 ){
      sz = ar.size();
      commonTree *cur_nd;
      for ( int ii = 0 ; ii < sz ; ii++ ) {
          cur_nd = checked_cast(commonTree,ar[ii]);
          if ( cur_nd->get_first() )
             ar[ii] = cur_nd->get_first_leaf();
      }
   }
 return;
}

// expands selected nodes in specified view to
// specified level. Null view implies all views.
void driver::expand (viewPtr view, int level)
{
    Initialize(driver::expand);
    
    objArr         temp;
    Obj            *el, *el1;
    viewTreePtr    vw_nd;
    OperPoint      *pt;
    
    start_transaction(){
	// update selection from local selections
	update_selection (view, 1);
    
	// Accumulate all selected nodes in local array
	ForEach (el, sel_list.oper_pts) {
	
	    if (el != NULL) {
		pt = (OperPoint*)el;
		if (vw_nd = checked_cast(viewNode, pt->view_node)) {
		    if (view == NULL || view == get_view_header(vw_nd))
			temp.insert_last (vw_nd);
		}
	    }
	}
    
	if (temp.size() > 0) {
	
	    // For each node do expand
	    ForEach (el1, temp) {
		//unselect (checked_cast(viewTree, el1));
		expand( view, checked_cast(viewTree, el1), level);
	    }
	} else {
	
	    if (view)
		expand (view, checked_cast(viewTree, NULL), level);
	}
    }end_transaction();
}

// expands specified viewnode. if viewnode is null
// do view specific expand (current node or top level node).
void driver::expand (viewPtr vw, viewTreePtr view_node, int level)
{
    Initialize(driver::expand);
    
    if (vw == NULL && view_node != NULL)
        vw = get_view_header(view_node);
    
    if (vw) {
	vw->expand (view_node, level);
    }
}

// collapses selected nodes in specified view to
// specified level. Null view implied all views.
void driver::collapse (viewPtr view, int level)
{
    Initialize(driver::collapse);
    
    objArr         temp;
    Obj            *el, *el1;
    viewTreePtr    vw_nd;
    OperPoint      *pt;
    
    start_transaction(){
	// update selection from local selections
	update_selection (view, 1);
    
	// Accumulate all selected nodes in local array
	ForEach (el, sel_list.oper_pts) {
	
	    if (el != NULL) {
		pt = (OperPoint*)el;
		if (vw_nd = checked_cast(viewNode, pt->view_node)) {
		    if (view == NULL || view == get_view_header(vw_nd))
			temp.insert_last (vw_nd);
		}
	    }
	}
    
	if (temp.size() > 0) {
	
	    // For each node do collapse
	    ForEach (el1, temp) {
		//unselect ( checked_cast(viewTree, el1 ));
		collapse ( view, checked_cast(viewTree, el1), level);
	    }
	} else {
	    if (view)
		collapse (view, checked_cast(viewTree, NULL), level);
	}
    }end_transaction();
}

// collapses specified viewnode. if viewnode is null
// do view specific collapse (current node or top level node).
void driver::collapse (viewPtr view, viewTreePtr view_node, int level)
{
    Initialize(driver::collapse);
    
    if (view == NULL && view_node != NULL)
        view = get_view_header(view_node);
    
    if (view) {
	view->collapse (view_node, level);
    }
}

//------------------------------------------
// replace
//------------------------------------------

void driver::replace()
{
    Initialize(driver::replace);

    genError*  err;
    appTreePtr source_node = sel_list.top_node ();
    appPtr     root;
    ostrstream strm;
    char*      text;
    
    if (source_node != checked_cast(appTree,NULL)) {
	root = source_node->get_header ();
	
	if (current_dialog != NULL) {
	    current_dialog->send_string (strm);
	    text = strm.str ();
	}
	
	genTry {
	    if ( text == NULL || *text == '\0' ) {
		obj_remove (root, source_node);
	    } else {
		obj_insert (root, REPLACE, NULL, source_node, text);
	    }
	} genCatch (err) {
	    this->error ();
	}
	genEndtry;
    }
}

//------------------------------------------
// select
//
// Routine to select a node or insert a string into the application tree
//------------------------------------------

void driver::select()
{
    Initialize(driver::select);

    ostrstream  os;
    
    if (current_dialog != NULL) {
	current_dialog->send_string (os);
	char* str = os.str ();
	
	if (strcmp (str, "") == 0) {
	    clear_selection ();
	    addselect ();
	} else {
	    create ();		// Non-empty string: create
	}
    }
}

// Updates selection array from local selections.
void driver::update_selection (viewPtr view, int structured_selection)
{
    Initialize(driver::update_selection);
    return; // Dont use any local selections. Get them from editor only.
    if (view == NULL || view->get_type() == Rep_TextText
	|| view->get_type() == Rep_SmtText)
    {
	if (structured_selection)
	    ste_interface_struct_apply (view);
	else
	    ste_interface_region_apply (view);
    }
}

// Return true if v_node is currently selected.
int driver::is_selected(viewTreePtr v_node, OperPoint** pt)
{
    Initialize(driver::is_selected);
    
    if(v_node)
    {
	Obj* el;
	ForEach(el, sel_list.oper_pts)
	{
	    if(el && ((OperPoint*)el)->view_node == v_node)
	    {
		if(pt)
		    *pt = (OperPoint*)el;
		
		return 1;
	    }
	}
    }
    return 0;
}

// Unselects viewnode
void driver::unselect(viewTreePtr v_node)
{
    Initialize(driver::unselect);
    
    OperPoint* pt;

    int selected = 0;

    while (is_selected(v_node, &pt))
    {
	sel_list.remove_pt(*pt);	// Remove from global selection list
        selected = 1;
	
	if (is_viewSymbolNode (v_node)) // Visually unselect
	    checked_cast(viewSymbolNode,v_node)->unselect();
    }

    if ( selected && is_steButton (v_node))
	ste_interface_unselect (v_node);
}

// Unselects all viewnodes of a view header
void driver::unselect(viewPtr vvv)
{
    Initialize(driver::unselect);
    
    objArr      selected_nodes;
    Obj*        el;
    OperPoint*  pt;
    
    // takes care about steRegions in Epoch STE/SMOD Views,
    if (vvv == NULL || is_steView(vvv))
        ste_interface_unselect (vvv);
    
    // Match each v_node header in the selection list with vvv, if vvv != 0,
    // otherwise unselectAll
    ForEach (el, sel_list.oper_pts) {
	if (el != NULL) {
	    pt = (OperPoint *) el;
	    viewTree *vw_nd = checked_cast(viewTree,pt->view_node);

	    if (vw_nd) {  // match??
		if (vvv == NULL || (vvv == get_view_header (vw_nd)))
		    selected_nodes.insert_last (vw_nd);
	    }
	}
    }
    
    Obj *el1;
    ForEach (el1, selected_nodes) {
	unselect ( checked_cast(viewTree,el1));
    }
    
    // case unselectAll
    if (vvv == NULL) {
	sel_list.remove_all();
	browserShell::clear_all_browser_selections ();
	miniBrowser::clear_selections();
    }
}

// Selects viewtreenode
void driver::select_node(viewTreePtr v_node)
{
    Initialize(driver::select_node);
    
    if (v_node && is_ldrSymbolNode(v_node->get_ldrTree())) {
	
	ldrSelection ldr_sel;
	
	current_oper_pt = new OperPoint (NULL_symbolPtr,FIRST, 0, 0, 0, v_node);
	
	ldr_sel.ldr_symbol    = NULL;
	ldr_sel.ldr_connector = NULL;
	ldr_sel.subtype       = 0;
	
	ldr_sel.ldr_symbol = checked_cast(ldrSymbolNode, v_node->get_ldrTree ());
	
	ldrHeader* ldr_head = checked_cast(ldrHeader, (checked_cast(view,v_node->get_header()))->get_ldrHeader());
	
	ldr_head->build_selection (ldr_sel, *current_oper_pt);
	
	// get subnode
	if (is_viewSymbolNode (v_node)) {
	    
	    // get the picture
	    graParametricSymbol *picture =
		(checked_cast(viewSymbolNode, v_node))->get_picture();
	    
	    // and the subnode
	    if (picture) {
		current_oper_pt->subnode = checked_cast(appTree, picture->get_modified_token());
		// picture->hilite_modification_text();
	    }
	}
	
	current_dialog = find_dialog_for_view ( checked_cast(view, v_node->get_header()));
	
	addselect();
    }
}

//------------------------------------------
// addselect
//
// Routine to add a node to the selection stack
//------------------------------------------

void driver::addselect(OperPoint *pt)
{
    Initialize(driver::addselect);

    viewTreePtr vw_nd;
    
    if (pt) current_oper_pt = pt;
    else pt =  current_oper_pt;

    if (current_oper_pt != (OperPoint*)NULL) {
	sel_list.push_pt (*current_oper_pt);
	vw_nd = checked_cast(viewTree, current_oper_pt->view_node);
	if (vw_nd != checked_cast(viewTree,NULL)) {
	    if (is_viewSymbolNode (vw_nd)) {
		(checked_cast(viewSymbolNode,vw_nd))->select ();
	    }
	}
    }

   if(0) {

     appTree * node = pt->node;
     viewTree * vn = (viewTree*) pt->view_node;
     view * vh = (view*) (vn ? vn->get_header() : 0);
     DBG
      {
         cout << "  addselect node " << node << " view " << vh->get_name()
                << " oper " << (int)pt->type << " off " << pt->offset_of << endl;
     }
     if(cmd_begin()){
         cmd_dr_gen("driver_addselect(%n, %d);", node, pt->type);
     }
     cmd_end();
  }
}


//------------------------------------------
// create
//
// Function to insert a string into the application tree
//------------------------------------------

void driver::create()
{
    Initialize(driver::create);

    genError*   exc;
    appPtr      root;
    OperPoint*  oper_pt;
    appTreePtr  ins_pt;
    objOperType type;
    appTreePtr  source_node = sel_list.top_node ();
    ostrstream  strm;
    char*       entity = NULL;
    
    if (current_dialog != NULL)
    {
	// Calculate insertion point
	// If there is a current node, use that -- drag and drop
	
	if (current_oper_pt != NULL) {
	    oper_pt = current_oper_pt;
	} else {
	    // If there is a current insertion point, use that
	    oper_pt = current_dialog->get_current_pt ();
	}
	
	if ( oper_pt != NULL ) {
	    
	    ins_pt = oper_pt->node;
	    type = oper_pt->type;
	    
	    current_dialog->send_string (strm);
	    entity = strm.str ();
	    
	    clear_selection();
	    
	    // Either entity or source_node should be defined for obj_insert
	    
	    if (source_node == checked_cast(appTree,NULL)) {
		viewPtr vw = current_dialog->get_current_view ();
		ldrPtr ldr = vw->get_ldrHeader ();
		root = ldr->get_appHeader ();
		
		
	    } else {
		root = source_node->get_header ();
	    }
	    
	    genTry {
		obj_insert (root, type, source_node, ins_pt, entity);
	    }
	    genCatch (exc) {
		driver::error ();
	    }
	    genEndtry;
	    
	    free(entity);
	}
	current_dialog->set_string (NULL);
    }
}

//------------------------------------------
// error
//
// Error - so generate w warning sound
//------------------------------------------

void driver::error()
{
    Initialize(driver::error);
}


//------------------------------------------
// flush_events
//
// Flush all X events
//------------------------------------------

void driver::flush_events()
{
    // graphics_world->flush ();
}

#ifdef NEW_UI

//------------------------------------------
// run
//
// Run the event loop
//------------------------------------------

veventDispatcherProc old_dispatcher;

void EventPreblock(void)
{
#ifdef _WIN32
    WaitMessage();
    while(Tcl_DoOneEvent(TCL_DONT_WAIT) != 0) {
	/* Empty */
    }
#endif
}

int EventDispatch(vevent *event, const veventHandler *handler)
{
	static int dispatching = 0;

    int ret_val = FALSE;
    start_transaction() {
		ret_val = old_dispatcher(event, handler);
    } end_transaction();
	if(dispatching == 0){
		dispatching = 1;
		int count   = 10;
		while(Tcl_DoOneEvent(TCL_DONT_WAIT) != 0 && count != 0) {
			count--; // do not go into inifinite loop (it's possible)
	    }
		dispatching = 0;
	}
    return ret_val;
}

#endif

void driver::run()
{
#ifndef NEW_UI
  mpb_destroy();
    done = 0;
    if (graphics_world)
        graphics_world->run(done);
#else 
    static regularTimer myRegTimer;
    myRegTimer.SetRecurrent();
    myRegTimer.SetPeriod(2, 0);
    myRegTimer.SetSynchronous();
    myRegTimer.Start();

    old_dispatcher = vevent::GetDispatcher();
    vevent::SetDispatcher(EventDispatch);
//    vevent::SetPreBlockHandlerProc(EventPreblock);
    vevent::Process();
#endif

}

//------------------------------------------
// stop_running
//
// Request the termination of the event loop.
//------------------------------------------

void driver::stop_running()
{
    done = 1;
}

void driver::process_pending_events()
{
    if (graphics_world)
        graphics_world->process_pending_events();
}


//------------------------------------------
// find_dialog_for_app
//
// Find the first dialog for the specified app
//------------------------------------------

dialogPtr driver::find_dialog_for_app( appPtr app )
{
    Initialize(driver::find_dialog_for_app);

    dialogPtr found_dialog = NULL;
    Obj*      dlg;
    
    if (app== NULL)
	return NULL;
    
    ForEach (dlg, dialogs) {
	if ( checked_cast(dialog,dlg)->contains_app (app) ) {
	    found_dialog = checked_cast(dialog,dlg);
	    break;
	}
    }
    
    return found_dialog;
}

//------------------------------------------
// find_dialog_for_view
//
// Find the dialog which contains the specified view
//------------------------------------------

dialogPtr driver::find_dialog_for_view(viewPtr vw)
{
    Initialize(driver::find_dialog_for_view);

    if (vw == NULL)
	return NULL;

    dialogPtr found_dialog = NULL;
    Obj*      dlg;

    ForEach(dlg, dialogs) {
	 if ( (checked_cast(dialog,dlg))->contains_view (vw) ) {
	    found_dialog = checked_cast(dialog,dlg);
	    break;
	}
    }

    return found_dialog;
}


void driver::_paste (appPtr hd)
{
    Initialize(driver::_paste);
    if (hd && is_smtHeader(hd)) {
	appTreePtr trg_el = NULL, src_el = NULL;
	int off;
	appMarker::get_mark (hd, trg_el, off);
	if (trg_el) {
	    OperPoint curr_pt(trg_el, AFTER);
	    curr_pt.offset_of = off;
	    if (!(trg_el->type == SMT_token) && !trg_el->get_first())
		curr_pt.type = FIRST;
	    else if (off == 0)
		curr_pt.type = BEFORE;
		
	    objArr *temp_arr = NULL;
	    start_transaction() {  
		// get objects to paste
		temp_arr = scrapbook_instance->objects_to_paste (hd);

		if (!temp_arr) {
		    dis_message (C_DRIVER, MSG_ERROR, "M_NOTHINGONCLIPB");
		    return;
		}

		// paste them
		Obj *el;
		start_smt_editing();
		appTreePtr src_el = NULL;
		ForEach (el, *temp_arr) {
		    if (is_appTree ( (Relational *) el) )
			src_el = checked_cast(appTree, el);
		    // insert at point

		    insert_at (&curr_pt, src_el);

		    curr_pt.node = checked_cast(appTree,trg_el);
		    curr_pt.type = AFTER;
		}
		commit_smt_editing();
		unselect ((view *)NULL);
	    } end_transaction();  

	    delete temp_arr;
	}
    }
}

// Returns 0 on succes ,
//         1 - zero id ,
//         2 - wrong smtTree structure ,
//         3 - id is not macrocall ,
//         4 - no parsing information available ,
//         5 - parser error(s) occured
//
extern steViewPtr get_current_view();

// so far, for Epoch *compilation* buffer only
// second parameter should define expantion type
//


void view_delete_hook (viewPtr p)
{
    driver_instance->detach_view(p);
}

void driver_instance_unselect_view (view *v)
{
  driver_instance->unselect (v);
}

void driver_instance_unselect_viewTree (viewTree *v)
{
  driver_instance->unselect (v);
}
