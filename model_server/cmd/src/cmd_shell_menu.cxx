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
// cmd_shell_menu.C
//------------------------------------------
// synopsis:
//    A group of calls for simulating menu related UI functions
//
// description:
// ...
//------------------------------------------
// 
// Restrictions:
// ...
//------------------------------------------

// include files


#include "genError.h"
#include "viewerShell.h"
#include "gtAppShellXm.h"
#include "cmd.h"
#include "cmd_shell.h"
#include "em.h"





//  ATTENTION :  All names used as arguments in the following calls are exactly
//               the same as they appear on the PARASet windows. They are displayed
//               names, not the X-Windows or other internal names.
//               Local variable item names are actual names of menu items, but not
//               full path names.


// Find the button of the TopShell menubar where TopShell given by its name.
// Push it. Return the widget corresponding to the menu item chosen.
// NOTE, for ViewerShell will return always the first Widget in the array. So, the currency
//       should be used instead.


   Widget cmd_push_named_menu_button(char *TopShellName, char *button_name)
   {

      char   *top_intern_name;
      Widget top_shell;
      Widget item;
      int    dummy; // flag

      Initialize(cmd_push_named_menu_button);

//    get shell name

      top_intern_name = cmd_lookup(&TopShellName, 1, &dummy);
      top_shell       = XtNameToWidget(UxTopLevel, top_intern_name);
      item            = cmd_push_menubar_button(top_shell, button_name);
      return item;
   }


// Find the button of the TopShell menubar where TopShell given by its Widget.
// Push it. Return the widget corresponding to the menu item chosen.


  Widget cmd_push_menubar_button(Widget curr_top_shell, char *button_name)
  {
      Widget item;
      Widget menu_bar;
      char   *item_name;
      int    dummy; // flag

//    get menu bar

      Initialize(cmd_push_menubar_button);

      menu_bar = em_get_menubar(curr_top_shell);

//    get item

      item_name = cmd_lookup(&button_name, 1, &dummy);
      item      = em_get_menu_item (menu_bar, item_name);

//    push item     

      em_moveto_widget(item);
      em_send_event (XtWindow(item), CLICK, LEFT);

      return item;
   }



// Get the Widget of the button and push it.
// Return the widget.


  Widget cmd_push_submenu_button(Widget parent_button, char *button_name)
  {
      Widget item;
      Widget sub_menu;
      char   *item_name;
      int    dummy; // flag

//    get item

      Initialize(cmd_push_submenu_button);

      item_name = cmd_lookup(&button_name, 1, &dummy);
      sub_menu  = em_get_item_submenu (parent_button);
      item      = em_get_menu_item (sub_menu, item_name);

//    push_item     

      em_moveto_widget(item);
      em_send_event (XtWindow(item), CLICK, LEFT);

      em_flush_events();
      return item;

   }

/*

   START-LOG-------------------------------------------

   $Log: cmd_shell_menu.C  $
   Revision 1.1 1994/03/17 16:29:01EST builder 
   made from unix file
 * Revision 1.1  1993/01/25  22:51:28  builder
 * Initial revision
 *
 * Revision 1.3  1993/01/05  21:17:59  sergey
 * Changed cmd_lookup() interface.
 *
 * Revision 1.2  1993/01/04  22:02:17  sergey
 * Switched to a new lookup call.
 *
 * Revision 1.1  1992/12/03  19:22:38  sergey
 * Initial revision
 *

   END-LOG---------------------------------------------
*/






