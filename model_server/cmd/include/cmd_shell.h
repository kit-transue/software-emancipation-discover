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
// File cmd_shell.h 
//
//------------------------------------------
// synopsis:
//    function declarations and classes for Command Journal
//
// description:
// ...
//------------------------------------------
// 
// Restrictions:
// ...
//------------------------------------------

// include files


#ifndef _CMD_SHELL_H_
#define _CMD_SHELL_H_

#define CMD_BROWSER_SHELL         1
#define CMD_VIEWER_SHELL          2
#define CMD_PROJECT_SHELL         3
#define CMD_FILE_BROWSER_SHELL    4


class viewerShell;

extern "C" {

// Menus / buttons ---------------------------

   Widget cmd_push_named_menu_button  ( char *TopShellName, char *button_name );
   Widget cmd_push_menubar_button     ( Widget curr_top_shell, char *button_name );
   Widget cmd_push_submenu_button     ( Widget parent_button, char *button_name );


// patterns to be selected follow n_items argument
//           (example: cmd_select_in_list(dd, 3, "abc", "proj1", "proj2");)

   void   cmd_select_in_list          ( Widget curr_dialog, int n_items, ...); 
   void   cmd_select_in_named_list    ( Widget curr_top_shell, char *dialog_name, int n_items,
                                        ...);

   void cmd_select_in_proj            ( Widget top_shell, int column_numb, int n_items, ...);
   void cmd_select_pos_in_proj        ( Widget top_shell, int column_numb, int n_items, ...);


// integer positions of items to be selected follow n_items argument
//    (example: cmd_select_pos_in_list(dd, 2, 8,9); )

   void   cmd_select_pos_in_list      ( Widget curr_dialog, int n_items, ...); 
                                           



// ViewerShell Window operations ---------------

   int     cmd_select_in_view         ( viewerShell *v_shell, char *pattern, 
                                                    int occur_numb = 1, int n_select = 1);
   int     cmd_select_pattern         ( viewerShell *v_shell, char *pattern, 
                                                     int occur_numb = 1, int n_select = 1);
   int     cmd_select_word            ( viewerShell *v_shell, char *pattern, 
                                                     int occur_numb = 1, int n_select = 1);
   int     cmd_select_from_to         ( viewerShell *curr_window, 
                                                     char *pattern1, int occurr_numb1,
                                                     char *pattern2, int occurr_numb2 );
   void    cmd_unselect_all           ( viewerShell *curr_window);
   void    cmd_replace_text           ( viewerShell *curr_vs, char *old_text, char *new_text,
                                                     int occur_numb = 1, int n_select = 1);
   void    cmd_insert_text_after      ( viewerShell *curr_window, char *pattern, char *insert_text,
                                                                  int occurr_numb = 1);
   void    cmd_click_MB1              ( viewerShell *curr_window, appTreePtr sel_obj );
   void    cmd_click_MB2              ( viewerShell *curr_window, appTreePtr sel_obj );
   void    cmd_click_MB3              ( viewerShell *curr_window, appTreePtr sel_obj );

   void    cmd_dbl_click_MB1          ( viewerShell *curr_window, appTreePtr sel_obj );
   void    cmd_dbl_click_MB2          ( viewerShell *curr_window, appTreePtr sel_obj );
   void    cmd_dbl_click_MB3          ( viewerShell *curr_window, appTreePtr sel_obj );

   void    cmd_collapse               ( viewerShell *curr_window, appTreePtr sel_obj );
   void    cmd_explode                ( viewerShell *curr_window, appTreePtr sel_obj );

// Ste related calls

   int cmd_ste_search_word            (viewerShell *vs, char * c_string);


// Getting internal name

   char   *cmd_lookup(char *names[], int n_names, int *return_flag);


// Currency handling

   void   cmd_make_current(int shell_type, Widget shell_widget);
   Widget cmd_get_current (int shell_type);
   void   cmd_show_current();


   void   cmd_set_control(char *func_call);
   char  *cmd_get_control();

  }


/*

   START-LOG-------------------------------------------

   $Log: cmd_shell.h  $
   Revision 1.1 1993/07/29 10:31:58EDT builder 
   made from unix file
 * Revision 1.11  1993/01/26  22:55:53  sergey
 * Removed dead code ( ... _dialog_... ).
 *
 * Revision 1.10  1993/01/26  18:42:33  sergey
 * Added cmd_select_in_view() declaration; removed obsolete cmd_look_up().
 *
 * Revision 1.9  1993/01/15  20:41:49  sergey
 * Changed return value of cmd_select_from_to from void to int.
 *
 * Revision 1.8  1993/01/10  20:46:31  sergey
 * Changed arguments at text handling routines (removed rep_type).
 *
 * Revision 1.7  1993/01/05  21:15:35  sergey
 * Added flag argument to cmd_lookup().
 *
 * Revision 1.6  1992/12/21  21:52:09  sergey
 * Added cmd_lookup() and removed CMD_MAX_NAME_LEN.
 *
 * Revision 1.4  1992/12/12  00:14:23  sergey
 * *** empty log message ***
 *
 * Revision 1.3  1992/12/08  00:19:26  sergey
 * Added dialog types to deal with same names for dialogs and buttons.
 *
 * Revision 1.2  1992/12/03  19:37:39  sergey
 * Added cmd_set_control declaration.
 *
 * Revision 1.1  1992/12/03  19:18:55  sergey
 * Initial revision
 *

   END-LOG---------------------------------------------
*/

#endif
