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
#ifndef _ui_h
#define _ui_h

#ifndef _XtAtom_h_
#include <X11/StringDefs.h>
#endif
#include <X11/X.h>

#undef _prototypes_h
#include <prototypes.h>

BEGIN_C_DECL

typedef struct
{
    unsigned int size;
    void*        addr;
}
uiValue, *uiValuePtr;


void  ui_resize_window PROTO((Window window));
char* ui_get_value PROTO((char* name, char* option));
psetCONST char* ui_get_resource PROTO(
    (const char* rsrc_name, const char* rsrc_class, uiValuePtr));

#if defined(_UXLIB_INCLUDED)
void* popup_transfer PROTO((swidget (*func)(), void* ptr1, void* ptr2));
#endif

#if defined(_XtIntrinsic_h)
void  ui_set_scroll_range PROTO((Widget, int  min, int  max));
void  ui_get_scroll_range PROTO((Widget, int* min, int* max));
void  ui_set_scroll_value PROTO((Widget, int value));
int   ui_get_scroll_value PROTO((Widget));
void  ui_set_slider_size  PROTO((Widget, int size));
#endif

END_C_DECL


/*
   START-LOG-------------------------------------------

   $Log: ui.h  $
   Revision 1.2 1999/08/27 09:58:54EDT sschmidt 
   HP 10 port
 * Revision 1.2.1.8  1994/01/04  23:51:30  boris
 * Bug track: Macro CONST
 * Renamed macro CONST into psetCONST
 *
 * Revision 1.2.1.7  1993/05/13  18:41:19  glenn
 * Use standard C/C++ macro protocol from prototypes.h.
 *
 * Revision 1.2.1.6  1993/03/04  18:52:53  trung
 * Arrrggghhh.  Ok, let's try defining the funcs as extern "C"  -jef
 *
 * Revision 1.2.1.5  1993/03/04  18:13:22  trung
 * Force prototypes.h to readload so that CONST is actually defined. -jef
 *
 * Revision 1.2.1.4  1993/03/01  00:25:14  glenn
 * Use prototypes.h
 *
 * Revision 1.2.1.3  1992/12/11  14:18:24  oak
 * Added ui_get_srcoll_value.
 *
   END-LOG---------------------------------------------
*/

#endif /* !_ui_h */
