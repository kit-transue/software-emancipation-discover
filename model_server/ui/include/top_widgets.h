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
#ifndef _top_widgets_h
#define _top_widgets_h

/* top_widgets.h
 *------------------------------------------
 * synopsis:
 * A group of subroutines to remember and operate on the
 * top-level widgets in an application.
 *------------------------------------------
 * restrictions:
 * This file should be C/C++ compatible.
 *------------------------------------------
 */

#ifndef _x11_intrinsic_h
#include <x11_intrinsic.h>
#endif

#ifndef _prototypes_h
#include <prototypes.h>
#endif

BEGIN_C_DECL

/*Guy: destroy_top_widgets() decleration*/
void add_top_widget PROTO((Widget));
void rem_top_widget PROTO((Widget));
void set_cursor PROTO((Cursor));
void push_cursor PROTO((Cursor));
void pop_cursor PROTO(());
void reset_cursor PROTO(());
unsigned long get_busy_cursor PROTO(());
void push_busy_cursor PROTO(());
void reset_current_cursor PROTO(());

END_C_DECL

#ifdef DESIGN_TIME	/* Stubs for UIM/X interpreter. */
void add_top_widget(w) Widget w; {}
void rem_top_widget(w) Widget w; {}
#endif

/* make this c++ code digestible to C files */
#ifdef __cplusplus
     
class tempCursor
{
  public:
    tempCursor (Cursor c = None);
    ~tempCursor ();
};

typedef void (*PVOIDFUNC)( void * ); 
void pset_send_own_callback (PVOIDFUNC func = 0, void *dt = 0);

#endif


/*
    START-LOG-------------------------------

    $Log: top_widgets.h  $
    Revision 1.7 2000/01/05 15:06:23EST sschmidt 
    Pruning more OODT stuff
 * Revision 1.2.1.9  1994/08/01  19:18:01  farber
 * Bug track: 7898
 * add function to destroy all the widgets pointed to by array top_widgets (and reset the array top_widgets)
 *
 * Revision 1.2.1.8  1994/07/18  15:00:14  boris
 * Bug track: 7395
 * Unsensetize Cut/Paste menu items for read_only buiffers
 *
 * Revision 1.2.1.7  1994/07/07  01:09:43  boris
 * Bug track: 6713
 * Added Batch mode to Ext Clients
 *
 * Revision 1.2.1.6  1994/03/12  19:22:59  jp
 * *** empty log message ***
 *
 * Revision 1.2.1.5  1994/03/10  16:53:11  jon
 * Bug track: 6624
 * Set cursor to pointer when cursor stack is dry instead of
 * setting cursor to "None". Added hook into gtShellXm.h.C
 * to reset the cursor when gt dialogs are popped up
 * (the reset in add_top_widgets is too early, the widget
 * has not been realized at that point).
 *
 * Revision 1.2.1.4  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.3  1992/10/23  17:48:52  glenn
 * Add get_busy_cursor.
 *
 * Revision 1.2.1.2  92/10/09  19:57:22  kws
 * Fix comments
 * 
    END-LOG---------------------------------
*/

#endif /* _top_widgets_h */
