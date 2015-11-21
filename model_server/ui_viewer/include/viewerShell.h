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
#ifndef _viewerShell_h
#define _viewerShell_h

// viewerShell.h
//------------------------------------------
// synopsis:
// 
// define the class for viewerShell
//------------------------------------------

#ifndef _Xm_h
#define XTFUNCPROTO
#define Object XObject
#include <Xm/Xm.h>
#undef Object
#endif

#ifndef _representation_h
#include <representation.h>
#endif

#ifndef _Relational_h
#include <Relational.h>
#endif

#ifndef _dialog_h
#include <dialog.h>
#endif

#ifndef _RTL_h
#include <RTL.h>
#endif

#ifndef _viewer_modes_h
#include <viewer_modes.h>
#endif

#ifndef steScreen_h
#include <steScreen.h>
#endif

RelClass(steSlot);
RelClass(view);
RelClass(steShellView);
RelClass(viewListHeader);
RelClass(viewTree);
RelClass(smtTree);

class viewerShell;
class dbgSession;
class miniBrowser;

typedef Widget (*slotfill_func)(Widget, void*);

define_relation(screen_of_viewer,viewer_of_screen);

class viewer : public Relational
{
  public:
    define_relational(viewer,Relational);
    virtual void print(ostream&, int) const;
    void set_screen(steScreen *scr = NULL);
    steScreen *get_screen();
#ifdef __GNUG__  
    viewer (const viewer& oo){*this = oo;}
#endif
    declare_copy_member(viewer);    
    
    static viewer* preferred_viewer();

    viewer(viewerShell*, Widget, int);
    void init(Window, Widget, Widget, Widget, Widget);
    void debug_mode(int flag);
    void manage_widget();
    ~viewer();

    void init_view_type();
    int get_view_type();

    void change_view(viewPtr, int old_view_deleted = 0);
    void change_view_type(repType);
    void change_view_from_history(int delta, int old_view_deleted = 0);
    void add_view_to_history();

    viewPtr get_view_ptr();
    viewListHeaderPtr get_rtl_view_ptr();
    viewerShell* get_shell();

    Widget get_widget();
    void target_lock(int);
    void set_current();
    void unset_current();
    void open_window();
    void close_window();
    void manipulate_perspective(int, int, int, int, int);

    void del();
    void cut();
    void copy();
    void _paste();
    void reference();

    void reparse();
    void enable_reparse(boolean);
    void update_views_option_menu();
    Window get_win_behind_epoch();
    
    RTLPtr obtain_history_list();
    void   set_history_index(int idx);
    
    static steScreenPtr get_screen_of_view (viewPtr);
    static viewPtr      get_current_view_of_screen (steScreenPtr);
    static viewer*      get_viewer_of_view (viewPtr);
  private:

    viewPtr      cView;
    viewerShell* shell;
    short        viewNum;
    viewType     current_type;

    Widget wij;
    Widget viewMenu;            // option menu of available views
    Widget hScroll;             // scroll bars
    Widget vScroll;             // scroll bars
    Window wind;                // window to draw in
    Widget vform;               // form containing target, reparse, viewMenu & viewType

    viewListHeaderPtr rtl_view;   // all views under viewerShell.

    RTLPtr history_list;        // previous views in this viewer.
    short  history_index;	// used when browsing history_of_views.

    void add_view_menu_rtl(RTLPtr, int viewer_number);
};

generate_descriptor(viewer,Relational);

inline viewPtr         viewer::get_view_ptr() { return cView; }
inline viewListHeaderPtr viewer::get_rtl_view_ptr() { return rtl_view; }
inline viewerShell*    viewer::get_shell() { return shell; }
inline Widget          viewer::get_widget() { return wij; }

void add_view_type_to_name(view *v, genString& name);

extern "C" int can_add_viewer(viewerShell*);
extern "C" int can_remove_viewer(viewerShell*);
/*Guy: declaration of the function I added */
extern "C" int viewerShell_Is_viewer_null(viewerShell*);

#define CREATE_MINIBROWSER 1
#define NO_MINIBROWSER     0

class viewerShell : public Obj
{
  public:
    enum { MAX_VIEWERS = 3 };


    static void cm_action(Widget menu_item,XtPointer client_data,XtPointer call_data);
    static void         set_status_line(const char*);
    // Set the viewer with the focus.
    static void         focused_viewer(viewer*);
    static void         focused_view  (viewer*, viewPtr);
    // Notify the viewerShell that the viewer should not have the focus.
    static void         not_focused_viewer(viewer*);

    // Return the current viewer shell, or create a new one if necessary.
    static viewerShell* get_current_vs();
    static viewerShell* get_current_vs(int no_create);
    static void         set_current_vs(viewerShell*);

    static viewerShell* get_vs_by_idx(int idx);
 
    // find_target_viewer() may only be called by functions in view_creation.C.
    static viewer*      find_target_viewer();

    static void         pre_hook(viewerShell*);
    static void         post_hook();

    static viewerShell* viewerShell_of_view(viewPtr);
    static viewerShell* viewerShell_of_screen(steScreenPtr);
    static viewer*      viewer_of_rtlview (viewListHeaderPtr);


    viewerShell(int create_minibrowser);
    void initWidgetSlots(Widget, Widget, Widget, Widget, Widget, Widget);

    ~viewerShell();

    void map();
    void bring_to_top();

    Widget    get_widget();
    dialogPtr getDialog();

    bool buildCMMenu();
    viewer* create_viewer();
    void    split(viewPtr newview = NULL);
    void    unsplit();
    inline int     get_num_viewers() const;
    inline int     get_current_viewer() const;
    inline viewer* get_viewer(int ind) const;

    viewPtr open_view(appPtr, repType, appTreePtr);
    viewPtr open_view(repType, symbolPtr);
    viewPtr find_view(ldrPtr);
    viewPtr find_view(viewPtr);
    viewPtr find_view_for_viewer(ldrPtr, viewer*);
    viewPtr find_view (int);
    void    close_current_view();

    void view_type(viewType);

    void cut();
    void copy();
    void _paste();
    void reference();

    void manipulate_perspective(int, int, int, int, int);
    void panner_callback(int, int, int);
    void collapse_current(int);
    void explode_current(int);
    void view_history(int);

    viewPtr get_current_view();
    viewPtr request_view(viewPtr);
    viewer* viewer_of_view(viewPtr);
    void    viewer_changed(viewer*);

    void           set_mode(viewerEditMode);
    viewerEditMode get_mode();

    void           is_customizing(viewerCustMode);
    viewerCustMode is_customizing();

    void add_buttonbar_item(Widget);
    void add_buttonbar_item(int, Widget);
    void add_buttonbar_item(int, char*);
    void save_buttonbars();
    void sensitize_buttonbar();
    void manage_buttonbar_buttons();

    void enable_manager(int state);
    static void enable_managers();

    // configure debugger viewer && view
    int configure(app *, viewer * &, view * &);

    RTLPtr  get_rtl();
    viewPtr find_duplicate_view(viewPtr);
    void    remove_view_from_menu(viewPtr);
    void    add_view_to_menu(viewPtr);

    void app_save();
    void app_save_as();
    void soft_associate();
    void hard_associate();
    void remove_association();
    void decorate_source(int);
    void enable_decorate_source();

    void register_destroy_CB(void (*destroy_CB)());

    viewPtr next_view (viewPtr) const;
    steScreen *next_screen (steScreen *curr, viewPtr&) const;

    viewPtr     find_existing_view(appTreePtr, viewer * * = 0);
    miniBrowser *get_mini_browser() { return mini_browser; };
    static void set_closure_state(bool enabled);

  private:
    static objArr vsh_array;    // Array of all existing viewershells

    // Functions to initialize extra pulldown menus for each repType
    static slotfill_func* create_pullMenu[view_Last + 1];


    // viewerShell that started the currently executing command
    static viewerShell*    hook_shell;

    // The viewer that everybody wants to know about
    static viewer* the_focused_viewer;
    static viewPtr the_focused_view_header;

    static void detach_hook(void* vsh, int view_num);

    RTLPtr rtl;
    dialogPtr dlg;

    Widget  wij;                // Top-level shell
    Widget  menuBar;            // menu-bar
    Widget  cm_button;		// cm menu
    Widget  cmMenu;
    Widget  buttonBarSlot;      // slot for button bars
    Widget  viewWindow;         // paned window
    Widget  panner;             // two-D scrollbar interface
    Widget  debugCommandWidget; // slot for epoch gdb-mode
    Widget  buttonBar[view_Last+1]; // Custom button bar for each repType
    Widget* pullMenu[view_Last+1];  // Extra pulldown menus for each repType

    viewType   current_type;
    viewer*    viewers[MAX_VIEWERS];
    short      current_viewer;
    short      num_viewers;
    viewerEditMode edit_mode;
    viewerCustMode cust_mode;

    viewPtr duplicate_view(viewPtr);
    int     view_in_use(viewPtr);
    void    load_buttonbars();
    void    remove_debug_view();
    viewer* available_viewer();
    void    init_interface(viewer*);
    viewPtr find_existing_view(viewer*, appTreePtr);

    miniBrowser  *mini_browser;
    Widget       icon_place;
};

typedef viewerShell* viewerShellPtr;

inline RTLPtr         viewerShell::get_rtl() { return rtl; }
inline viewerEditMode viewerShell::get_mode() { return edit_mode; }
inline viewerCustMode viewerShell::is_customizing() { return cust_mode; }
inline viewer*        viewerShell::get_viewer(int ind) const {return viewers[ind];}
inline int            viewerShell::get_num_viewers() const {return num_viewers;}
inline int            viewerShell::get_current_viewer() const {return current_viewer;}
inline Widget         viewerShell::get_widget(){return wij;}


Widget create_view_type_menu(Widget, void*, char* ...);
Widget CreatePulldown(Widget, void*, char*, XtCallbackProc ...);

extern void change_debug_PD_buttons(int new_debug_mode, viewerShell *vsh);
extern void change_debug_H_buttons(int new_debug_mode, viewer *v);
extern "C"
{
    // From viewer.h.C:
    void viewer_target_lock(viewer*, int);

    // From viewerShell.h.C:
    int  viewerShell_finalize(viewerShell*, Widget);
    void viewerShell_post_hook(Widget);

    // From viewerShell_cmd.C:
    viewerShell* new_viewerShell();
    void viewerShell_open_view(app*, repType, appTree*);
    void viewerShell_map();
}

/*
    START-LOG-------------------------------

    $Log: viewerShell.h  $
    Revision 1.23 2002/03/24 23:15:04EST Dmitry Ryachovsky (dmitry) 
    Adding the CM menu to a viewerShell
 * Revision 1.2.1.51  1994/07/26  20:52:20  kws
 * Allow flowchart routing to be cancelled
 *
 * Revision 1.2.1.50  1994/07/20  16:33:54  farber
 * Bug track: 7860 and 6224
 * added  extern "C" int viewerShell_Is_viewer_null(viewerShell*); declaration
 *
 * Revision 1.2.1.49  1994/07/18  14:59:38  boris
 * Bug track: 7395
 * Unsensetize Cut/Paste menu items for read_only buiffers
 *
 * Revision 1.2.1.48  1994/07/11  17:18:46  so
 * Bug track: n/a
 * project for re-load pdf
 *
 * Revision 1.2.1.47  1994/05/27  21:14:15  boris
 * Bug track: 7151
 * Fixed UI Debugger crash
 *
 * Revision 1.2.1.46  1994/05/23  15:28:05  azaparov
 * Changes to support new debugger interface
 *
 * Revision 1.2.1.45  1994/01/21  00:35:49  kws
 * New rtl display
 *
 * Revision 1.2.1.44  1993/12/13  20:40:35  andrea
 * Bug track: n/a
 * fixed viewerShell
 *
 * Revision 1.2.1.43  1993/12/03  16:31:25  boris
 * Bug track: #4969
 * C-x o Emacs command for non ParaSET buffers
 *
 * Revision 1.2.1.42  1993/11/24  22:54:12  boris
 * Bug track: #5325
 * Fixed synchronization and fmr viewer problems
 *
 * Revision 1.2.1.41  1993/11/19  01:55:30  boris
 * Bug track: ParaSET window buffer
 * Window buffer towards Emacs behavior
 *
 * Revision 1.2.1.40  1993/11/02  17:14:32  boris
 * Bug track: #5114
 * Fixed restoring previous view after closing the current one
 *
 * Revision 1.2.1.39  1993/08/17  22:07:07  andrea
 * fixed bug 4369
 *
 * Revision 1.2.1.38  1993/06/28  23:53:38  jon
 * Added member function get_current_viewer()
 *
 * Revision 1.2.1.37  1993/06/10  20:17:42  andrea
 * added enable_manager declaration
 *
 * Revision 1.2.1.36  1993/06/08  14:11:14  kws
 * Added enable_manager to viewerShell
 *
 * Revision 1.2.1.35  1993/05/16  19:23:55  smit
 * stop at in smode allows you to choose interpreted/compiled breakpoint
 *
 * Revision 1.2.1.34  1993/05/14  20:48:13  glenn
 * Follow SET coding standards.
 *
 * Revision 1.2.1.33  1993/05/14  20:36:07  mg
 * idget get_widget(){return wij;}
 *
 * Revision 1.2.1.32  1993/05/14  15:47:41  glenn
 * Remove viewer::get_widget().  Add viewer::manage_widget().
 *
 * Revision 1.2.1.31  1993/05/11  22:57:36  boris
 * added get_current_vs() static member with no_create int argument
 *
 * Revision 1.2.1.30  1993/05/07  20:00:25  jon
 * Added viewer::get_widget, viewerShell::get_widget, viewerShell::get_num_viewers, and
 * viewerShell::get_viewer for command tool
 *
 * Revision 1.2.1.29  1993/04/30  03:01:25  kws
 * Added find_view(int)
 *
 * Revision 1.2.1.28  1993/04/26  20:21:52  glenn
 * Substitute find_existing_view for find_or_create_view.
 *
 * Revision 1.2.1.27  1993/04/24  21:25:17  boris
 * Added next_view() member function to perform Emacs other window command
 *
 * Revision 1.2.1.26  1993/04/20  01:36:05  glenn
 * Add comment about not calling find_target_viewer.
 *
 * Revision 1.2.1.25  1993/04/14  14:09:15  glenn
 * Add viewer::help_mode.  Change viewer::viewNum to signed short.
 *
 * Revision 1.2.1.24  1993/04/13  17:39:19  smit
 * Use the view in the target viewer if possible.
 *
 * Revision 1.2.1.23  1993/04/08  23:44:16  smit
 * fix bug#3168
 *
 * Revision 1.2.1.22  1993/03/15  15:26:03  smit
 * support bringing up raw view for unloaded file.
 *
 * Revision 1.2.1.21  1993/02/24  14:30:15  glenn
 * Add class static viewer::preferred_viewer.
 * Add viewer::target_lock.
 * Remove decl of viewerShell_pre_hook.
 * Add decl of viewer_target_lock.
 *
 * Revision 1.2.1.20  1993/02/21  00:07:25  glenn
 * Add extra flag to viewer::change_view.
 * Comment out viewerShell::add_view.
 *
 * Revision 1.2.1.19  1993/02/15  15:05:17  jon
 * Added member function help_shell()
 *
 * Revision 1.2.1.18  1993/02/12  02:36:30  glenn
 * Add get_num_viewers.
 * Remove friend access for can_add_viewer, can_remove_viewer.
 *
 * Revision 1.2.1.17  1993/02/12  02:13:22  glenn
 * Rename set_obj_type to init_view_type in class viewer.
 * Remove delete_previous_view arg to change_view.
 * Add class static functions focused_viewer, not_focused_viewer.
 * Remove detach_view (folded into detach_hook).
 * Remove close_view (folded into detach_hook).
 * Rename set_obj_typeto view_type in class viewerShell.
 * Remove num_viewers.
 * Rename change_view to viewer_changed.
 * Rename get_alternate_view to find_duplicate_view.
 * Rename add_alternate_view_to_list to remove_view_from_menu.
 * Rename add_view_to_list to add_view_to_menu.
 * Rename v_shell_array to vsh_array.
 * Add class static member variables the_focuse_viewer,
 * the_focused_view_header.
 * Rename numActiveViewers to num_viewers.
 * Add init_interface.
 *
 * Revision 1.2.1.16  1993/02/08  22:45:45  oak
 * Added manage_buttonbar_buttons.
 *
 * Revision 1.2.1.15  1993/02/03  14:55:15  jon
 * Added member function find_view_for_viewer for view creation.
 *
 * Revision 1.2.1.14  1993/02/02  21:54:03  smit
 * add support for enable/disable breakpoint menu items.
 *
 * Revision 1.2.1.13  1993/01/07  02:19:36  smit
 * make new_viewerShell return created viewerShell.
 *
 * Revision 1.2.1.12  1993/01/05  23:02:24  smit
 * Declare new static function.
 *
 * Revision 1.2.1.11  1992/12/31  19:45:15  aharlap
 * added viewPtr open_view(repType, symbolPtr);
 *
 * Revision 1.2.1.10  1992/12/21  20:07:38  glenn
 * Add enable_decorate_source.
 *
 * Revision 1.2.1.9  1992/12/17  21:36:09  wmm
 * Add declarations of extern "C" viewerShell entries.
 *
 * Revision 1.2.1.8  1992/12/17  00:15:34  jon
 * Change refer to reference and remove refer_graphics. woo.
 *
 * Revision 1.2.1.7  1992/12/14  22:32:32  glenn
 * Add change_view_from_history, add_view_to_history.
 * Change rtl_view to viewUIHeaderPtr.
 * Add history_list, history_index, pre_hook, post_hook.
 * Add view_history, sensitize_buttonbar, hook_shell.
 * Add available_viewer.
 *
 * Revision 1.2.1.6  1992/12/12  07:09:57  glenn
 * New view creation interface.
 *
 * Revision 1.2.1.5  1992/11/11  22:17:54  jon
 * Added viewerShell::change_arguments
 *
 * Revision 1.2.1.4  1992/11/05  16:54:01  jon
 * added attach_process() member function
 *
 * Revision 1.2.1.3  1992/10/12  19:41:52  smit
 * Fix bug #1538
 *
 * Revision 1.2.1.2  92/10/09  19:57:28  kws
 * Fix comments
 * 
    END-LOG---------------------------------
*/

#endif // _viewerShell_h
