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
// em.h
//------------------------------------------
// synopsis:
//    em function declarations and structrures.
//
// description:
// 
//------------------------------------------
// 
// Restrictions:
// 
//------------------------------------------

// include files

#ifndef _em_h
#define _em_h

enum em_button {

    LEFT  	= 0x01,
    MIDDLE	= 0x02,
    RIGHT	= 0x04,
    CONTROL	= 0x08,

    SHIFT	= 0x10,
    META	= 0x20,

    PUSH	= 0xf0,
    RELEASE,
    CLICK
};

extern XtAppContext UxAppContext;
extern Widget       UxTopLevel;

extern "C" {
    viewerShellPtr em_get_current_viewer();
    viewPtr em_get_current_view (viewerShellPtr);
    Window em_get_view_window (viewerShellPtr, viewPtr);
    Widget em_get_viewer_menu (viewerShellPtr);
    Widget em_get_viewer_widget (viewerShellPtr);
    Widget em_get_menubar(Widget);
    Widget em_get_menu_item (Widget, char*);
    Widget em_get_item_submenu (Widget);
    void em_moveto_widget(Widget);
    void em_moveto_window(Window, int, int);
    Widget em_find_widget (Widget top, char *name);
    Widget em_lookup(char *name);
    Window em_get_window (Widget);

    void em_set_speed(int);

    void em_dump_window (Window, char *file);
    void em_flush_events();
    void em_query_tree (Widget w, int level);
    void em_new_query_tree (Widget top, int level);
    void em_send_event (Window win, int event, int modifiers);
    void em_send_detailed_event (Window win, int event, 
				 unsigned int modifiers, 
				 int mouse_button);

    //  list manipulations

    void em_add_item(Widget theWidget, char* item_name, int item_pos) ;
    void em_select_item(Widget theWidget, char* item_name, bool notify = FALSE);
    void em_deselect_item(Widget theWidget, char* item_name);
    void em_select_pos(Widget theWidget, int item_pos, bool notify = FALSE);
    void em_deselect_pos(Widget theWidget, int item_pos);
    void em_deselect_all(Widget theWidget);
    void em_delete_item(Widget theWidget, char* item_name);
    void em_delete_pos(Widget theWidget, int item_position) ;
    void em_delete_items_at_pos(Widget theWidget, int number, int item_position);
    int em_item_exists(Widget theWidget, char* item_name);

    // Entering text
    void em_enter_string(Window win, char* theString, unsigned int modifier_keys); 
    void em_set_string_editor(Widget widget, const char *contents);

    // Finding Widgets in a tree with duplicate widget names.
    Widget em_name_to_nth_child(Widget parent, char* child_name, 
			     int occurance);
    Widget em_name_to_nth_widget(Widget top, char* widget_name, 
			      int occurance);
    Widget em_indexed_path_to_widget(Widget top, char* widget_path, 
				     int index_num);
    void em_print_event(XEvent xe);
}

extern Display* UxDisplay;

/*
   START-LOG-------------------------------------------

   $Log: em.h  $
   Revision 1.2 1995/07/27 20:20:30EDT rajan 
   Port
 * Revision 1.7  1993/01/11  16:30:24  oak
 * Added em_print_event.
 *
 * Revision 1.6  1993/01/04  16:19:20  oak
 * Added em_indexed_path_to_widget.
 *
 * Revision 1.5  1992/12/18  18:51:26  oak
 * Added em_indexed_path_to_widget.
 *
 * Revision 1.4  1992/12/17  21:39:24  oak
 * Added more commands.
 *
 * Revision 1.3  1992/12/01  19:37:44  oak
 * Added em_enter_string.
 *
 * Revision 1.2  1992/11/24  21:13:44  oak
 * Added em_item_exists
 *
 * Revision 1.1  1992/11/24  16:47:20  sergey
 * Initial revision
 *

   END-LOG---------------------------------------------
*/

#endif
