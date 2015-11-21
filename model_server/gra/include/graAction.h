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
#ifndef _graAction_h
#define _graAction_h

#ifndef  _prototypes_h
#include <prototypes.h>
#endif

EXTERN void gra_action_highlight PROTO((class graWindow*, int x, int y));
EXTERN void gra_action_focus_in PROTO((graWindow*));
EXTERN void gra_action_focus_out PROTO((graWindow*));
EXTERN void gra_action_highlight PROTO((graWindow*, int x, int y));
EXTERN void gra_action_unhighlight PROTO((graWindow*));
EXTERN void gra_action_open_node PROTO((graWindow*, int x, int y));
EXTERN void gra_action_collapse_expand PROTO((graWindow*, int x, int y));
EXTERN void gra_action_move_begin PROTO((graWindow*, int x, int y));
EXTERN void gra_action_move_tracking PROTO((graWindow*, int x, int y));
EXTERN void gra_action_move_end PROTO((graWindow*, int x, int y));
EXTERN void gra_action_zoom_rectangle PROTO((graWindow*, int x, int y));
EXTERN void gra_action_recenter PROTO((graWindow*, int x, int y));
EXTERN void gra_action_text_input PROTO((graWindow*, CONST char*));
EXTERN void gra_action_symbol_menu PROTO((graWindow*, int x, int y));
EXTERN void gra_action_select PROTO((graWindow*, int x, int y));

#endif /* _graAction_h */

/*
// graAction.h
//------------------------------------------
// synopsis:
// Declare X Intrinsics Actions for graphics window
// 
// description:
// ...
//------------------------------------------
// $Log: graAction.h  $
// Revision 1.1 1993/03/04 12:35:22EST builder 
// made from unix file
 * Revision 1.2.1.3  1993/03/04  17:34:07  trung
 * Included prototypes.h, got rid of local defines (and undefs) for EXTERN, PROTO, and CONST -jef
 *
 * Revision 1.2.1.2  1992/10/09  18:52:11  builder
 * fixed rcs heaqer
 *
 * Revision 1.2.1.1  92/10/07  20:41:49  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:41:48  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:20:30  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:56:09  smit
 * Initial revision
 * 
// Revision 1.1  92/03/03  18:38:37  glenn
// Initial revision
// 
//------------------------------------------
// Restrictions:
// This file must be compatible with C and C++.
//------------------------------------------
*/
