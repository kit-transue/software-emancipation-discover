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
#ifndef _driver_h
#define _driver_h

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
//------------------------------------------
// Restrictions:
// There is only one instance of this class instantiated at a time.
// It is pointed to by the global driver_instance.
//------------------------------------------

// include files
#ifndef _objRelation_h
#include <objRelation.h>
#endif
#ifndef _objOper_h
#include <objOper.h>
#endif
#ifndef _OperPoint_h
#include "OperPoint.h"
#endif
 
#ifndef _representation_h
#include "representation.h"
#endif
 
#ifndef __selList_h      
#include <selList.h>
#endif      
#ifndef steSlot_h 
#include <steSlot.h>
#endif      
 
#ifndef _oodt_ui_decls_h
#include "oodt_ui_decls.h"
#endif
 
#ifndef _dialog_h
#include "dialog.h"
#endif

#ifndef _objArr_h
#include <objArr.h>
#endif
      
// classes referenced

class graWorld;
class button;
class viewerShell;
class fileXref;

RelClass(dialog);
RelClass(dialogINTERP);
RelClass(view);
RelClass(SoftId);

// type definitions
enum SelectionResult { selREJECT, selACCEPT, selMODIFY };

//
// driver
//
class driver : public Relational
{
  private:
    objArr     dialogs;        // Array of dialog objects
    dialogPtr  current_dialog;
    OperPoint* current_oper_pt;

  public:
    selectionList sel_list;	// The global selection stack
    graWorld* graphics_world;	// The root world for graphics
    int done;			// True => request end of event-loop.

    define_relational(driver, Relational);

    driver(int argc, char* argv[]);
    driver ();

    ~driver();
    driver(const driver&){}

    static fileXref* get_file_xref();

    copy_member(driver);

    dialogPtr open_dialogRTL(
	char* title, void* (fn)(dialogPtr, void*) = NULL, void* info = NULL);

    dialogPtr open_dialogPlayGround (
	char* title, void* (fn)(dialogPtr, void*) = NULL, void* info = NULL);

    void    close_dialog (dialogPtr);

    void    close_view(viewPtr);

    void    change_view(viewPtr, repType);
    void    regenerate_view(viewPtr);
    void    detach_view (viewPtr);

    boolean contains_dialog(dialogPtr);

    void    dispatch(button&);

    void    select   ();
    void    addselect(OperPoint *pt = NULL);
    void    create   ();
    void    hierarchy_control();
    void    addhierarchy_control();
    void    move_node();

    // operations used by UI begins here

    void    remove_object (commonTreePtr aset_obj); // removes an aset object
    void    del (viewPtr = NULL);   // delete selected
    void    cut(viewPtr = NULL, viewTreePtr node = NULL); // cut selected
    void    _paste (viewPtr); // paste from clipboard
    void    _paste (appPtr);  // paste from clipboard
    void    copy (viewPtr);   // copy selected to clipboard
    void    clone (viewPtr);  // ??
    void    move (viewPtr);   // move selected to insertion point
    void    reference (viewPtr); // add reference to globally selected objects
    void    insert_at (OperPoint *, appTreePtr);
    void    replace (viewPtr, RelationalPtr); // 

    void    assign_category (viewPtr, steSlotPtr);
    void    deassign_category (viewPtr, steSlotPtr = NULL);
    void    assign_style (viewPtr, steSlotPtr);
    void    deassign_style (viewPtr, steSlotPtr);

    void    hard_associate (viewPtr, char *assoc_name);
    void    soft_associate (viewPtr, SoftIdPtr, const char *assoc_name);
    void    remove_hard_association (viewPtr, char *assoc_name);
    void    remove_soft_association (viewPtr, char *assoc_name);

    // operations used by UI ends here.

    void    replace  ();
    void    collapse (viewPtr view, int level);
    void    collapse (viewPtr, viewTreePtr, int level);
    void    expand   (viewPtr view, int level);
    void    expand   (viewPtr, viewTreePtr, int level);
    void    error    ();

    void    flush_events();
    void    run();
    void    stop_running();
    void    process_pending_events();

    //    SelectionResult put_selection(view&, button&, viewSelection&);
    void    clear_selection (bool including_browser_selection = true);
    void    clear_selection(dialog*);
    void    fill_array_with_selected_appnodes(viewPtr, objArr&, int subnodes = 0);
  void    fill_array_with_selected_appnodes(viewPtr,symbolArr&, int);

    void    selected_view_nodes(viewPtr, objArr&);

    dialogPtr find_dialog_for_app(appPtr);
    dialogINTERPPtr find_interp_dialog(int syspid);
    dialogPtr find_dialog_for_view(viewPtr);

    void      add_dialog(dialog *);
    void      remove_dialog (dialog *);

    int  is_selected(viewTreePtr, OperPoint** = NULL);
    void select_node (viewTreePtr);
    void unselect (viewTreePtr);
    void unselect (viewPtr);
    
    int gpi_unselect (appTreePtr, int nogpi = 0);

    void update_selection (viewPtr view, int structured_selection = 0);
    void expand_c_macro (viewPtr v, int mode = 0);
};
#ifdef XXX_driver
inline void driver::add_dialog (dialog *dlg)
{
    if (dlg)
	dialogs.insert_last (dlg);
}

inline void driver::remove_dialog (dialog *dlg)
{
    if (dlg)
	dialogs.remove (dlg);
}
#endif
generate_descriptor(driver,Relational);

extern driverPtr driver_instance;

/*
   START-LOG-------------------------------------------

   $Log: driver.h  $
   Revision 1.7 1996/11/22 09:48:07EST bing 
   Took out vtimer.
 * Revision 1.2.1.7  1994/02/08  02:48:11  boris
 * added macro expansion
 *
 * Revision 1.2.1.6  1993/11/16  00:56:42  trung
 * Bug track: 0
 * propagation project
 *
 * Revision 1.2.1.5  1993/11/06  22:03:52  so
 * Bug track: n/a
 * driver_instance->operation without viewshell
 * This is to support -batch to merge two files.
 * Done by boris
 *
 * Revision 1.2.1.4  1993/02/08  16:46:29  glenn
 * Remove open_dialogSearch.
 *
 * Revision 1.2.1.3  1992/12/17  14:35:24  jon
 * Removed refer_graphics, changed refer to reference.
 *
 * Revision 1.2.1.2  1992/10/09  18:53:08  smit
 * *** empty log message ***
 *

   END-LOG---------------------------------------------

*/
#endif // _driver_h
