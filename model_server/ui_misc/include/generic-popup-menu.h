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
#ifndef _generic_popup_menu_h
/*
  generic-popup-menu.h
  
  goes with generic-popup-menu.c
  encapsulates popup menu widget, built with Motif
  this file\'s functionality is to be replaced by gt\'s
  
*/

#define _generic_popup_menu_h

#ifndef NEW_UI

#ifndef _XtIntrinsic_h
#  define Object XObject
#  include <X11/Intrinsic.h>
#  undef Object
#endif

#include <prototypes.h>

#endif

enum menu_child_type
{
    PUSH,
    TOGGLE,
    SEPARATOR,
    CASCADE,
    END_SUBMENU
};

#ifndef NEW_UI

BEGIN_C_DECL

void popup_context_menu PROTO((int, int, Widget));
void ui_set_child_sensitive PROTO((Widget, char *, int));
void ui_set_toggle_child PROTO((Widget, char *, int));

END_C_DECL

#endif

#endif /* _generic_popup_menu_h */
