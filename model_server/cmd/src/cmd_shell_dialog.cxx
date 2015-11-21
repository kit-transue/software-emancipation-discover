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
// cmd_shell_dialog.C
//------------------------------------------
// synopsis:
//    A group of calls simulating dialog related UI functions
//    ( obsolete dialog related calls removed )
//
// description:
//    Usefull calls to deal with lists.
//
//------------------------------------------
// 
// Restrictions:
// ...
//------------------------------------------

// include files

#include "genError.h"
#include "viewerShell.h"
#include "systemMessages.h"
#include "cmd.h"
#include "cmd_shell.h"
#include "em.h"


// Select n items given by their visible names in list.
// List specified by the widget.

   void   cmd_select_in_list  ( Widget curr_list, int n_items, ... )
   {
      int  i;
      char *curr_item;
      va_list arg_ptr;

      Initialize(cmd_select_in_list);

      va_start(arg_ptr, n_items);
      for (i= 0; i < n_items; i++){
           curr_item = va_arg(arg_ptr, char *);
           em_select_item(curr_list, curr_item, 1);
      }
      va_end(arg_ptr);
   }



// Select n items given by their positions (starting from 1, not 0 !!) in list.
// List specified by the widget.

   void   cmd_select_pos_in_list  ( Widget curr_list, int n_items, ... )
   {
      int  i;
      int  curr_pos;
      va_list arg_ptr;

      Initialize(cmd_select_pos_in_list);

      va_start(arg_ptr, n_items);
      for (i= 0; i < n_items; i++){
           curr_pos = va_arg(arg_ptr, int);
           em_select_pos(curr_list, curr_pos, 1);
      }
      va_end(arg_ptr);
   }



/*

   START-LOG-------------------------------------------

   $Log: cmd_shell_dialog.C  $
   Revision 1.3 2000/07/10 23:01:10EDT ktrans 
   mainline merge from Visual C++ 6/ISO (extensionless) standard header files
 * Revision 1.3  1993/09/22  19:02:12  kws
 * Bug track: Port
 * Use aset_stdarg.h for port
 *
 * Revision 1.3  1993/09/22  19:02:12  kws
 * Bug track: Port
 * Use aset_stdarg.h for port
 *
 * Revision 1.2  1993/01/26  22:56:49  sergey
 * Removed dead code.
 *
 * Revision 1.1  1993/01/25  22:51:28  builder
 * Initial revision
 *
 * Revision 1.4  1992/12/12  00:13:19  sergey
 * *** empty log message ***
 *
 * Revision 1.3  1992/12/08  00:20:25  sergey
 * Added dialog type argument to look_up.
 *
 * Revision 1.2  1992/12/04  22:03:46  sergey
 * Correction to avoid syncronization problems.
 *
 * Revision 1.1  1992/12/03  19:22:17  sergey
 * Initial revision
 *

   END-LOG---------------------------------------------
*/
