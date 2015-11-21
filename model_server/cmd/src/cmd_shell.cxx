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
// cmd_shell.C
//------------------------------------------
// synopsis:
//    A group of calls simulating UI functions
//
// description:
//    cmd_make_current(), cmd_get_current(), cmd_select_in_view()
//-----------------------------------------------------------------
// 
// Restrictions:
// ...
//------------------------------------------

// include files


#include "genError.h"
#include "viewerShell.h"

#include "cmd.h"
#include "cmd_current.h"
#include "cmd_shell.h"
#include "em.h"
#include <Search.h>

static char *cmd_control = 0;


//  ATTENTION :  All names used as arguments in the following calls are exactly
//               the same as they appear on the PARASet windows. They are displayed
//               names, not the X-Windows or other internal names.
//               Local variable item names are actual names of menu items, but not
//               full path names.



// set current TopShell according to type

   void  cmd_make_current(int shell_type, Widget shell_widget)
   {

      Initialize(cmd_make_current);

      switch (shell_type) {

         case CMD_BROWSER_SHELL:
               curr_browser = shell_widget;
               break;

         case CMD_VIEWER_SHELL :
               curr_viewer = shell_widget;
               break;

         case CMD_PROJECT_SHELL:
               curr_project = shell_widget;
               break;

         case CMD_FILE_BROWSER_SHELL   :
               curr_file_browser = shell_widget;           // not used ?
      }
   }


// get current TopShell according to type

   Widget  cmd_get_current (int shell_type)
   {
      Widget shell_widget;
      viewerShellPtr vs;

      Initialize(cmd_get_current);

      switch (shell_type) {

         case CMD_BROWSER_SHELL:
//       assumes there is only one browser - does not handle multiple browser case
               shell_widget = XtNameToWidget(UxTopLevel,"browser");
               break;

         case CMD_VIEWER_SHELL :
               vs = em_get_current_viewer();
               shell_widget = vs->get_widget();
               break;

         case CMD_PROJECT_SHELL:
//       assumes there is only one browser - does not handle multiple browser case
               shell_widget = XtNameToWidget(UxTopLevel,"*panedWindow.project_browser");
               break;

         case CMD_FILE_BROWSER_SHELL   :                   // not used ?
               shell_widget = XtNameToWidget(UxTopLevel,"*panedWindow.file_browser");
               break;
      }

      return shell_widget;
   }




   void cmd_set_control(char *func_call){

      Initialize( cmd_set_control );

      if (func_call == NULL)
          cmd_control = NULL;

      else{
          int len = strlen(func_call);
          cmd_control = new char[len + 1];
          strcpy(cmd_control, func_call);
      }
   }

   char * cmd_get_control(){
      Initialize( cmd_get_control );
      return cmd_control;
   }


// selects in any view (except STE ?) 


int cmd_select_in_view ( viewerShell *vs,  char *pattern, int occur_numb,
                         int n_select)
{
    int        found = 0;
    viewPtr    view_hdr;

    Initialize(cmd_select_pattern);

    view_hdr = vs->get_current_view();
    Search* search = new Search(view_hdr, pattern, 0);

//  find the patterns starting from given occurance

    int i, j;
    for (i = 0, j = occur_numb - 1; i < n_select; i++, j++){    
	found = search->select(j + 1);
           if ( found == 0)
                break;
    }

    delete search;
    return found;
}

/*

   START-LOG-------------------------------------------

   $Log: cmd_shell.C  $
   Revision 1.1 1994/03/17 16:29:03EST builder 
   made from unix file
 * Revision 1.3  1993/12/11  16:58:12  jon
 * Bug track: 0
 * Fixed to work with the new quick search object
 *
 * Revision 1.2  1993/01/26  18:44:24  sergey
 * Removed cmd_look_up(), wrote cmd_select_in_view().
 *
 * Revision 1.1  1993/01/25  22:33:59  builder
 * Initial revision
 *
 * Revision 1.3  1992/12/21  21:53:40  sergey
 * Added cmd_get_control() call.
 *
 * Revision 1.2  1992/12/03  19:37:02  sergey
 * Added cmd_set_control for syncronization events.
 *
 * Revision 1.1  1992/12/03  19:21:53  sergey
 * Initial revision
 *

   END-LOG---------------------------------------------
*/






