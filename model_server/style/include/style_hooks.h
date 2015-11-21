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
//                  style_hooks.h
//	Application-layer style/category functions
//

#ifndef _style_hooks_h
#define _style_hooks_h

#include <objOper.h>
#include <steSlot.h>

define_relation(ddsty_of_node,nodes_of_ddsty);
define_relation(ddcats_of_node,nodes_of_ddcat);

extern void style_manual_put (appTreePtr, char *nm, boolean cat);
extern void style_manual_remove (appTreePtr, char *nm, boolean cat);
extern int style_manual_find_remove (appTreePtr, char *nm, boolean cat);

#endif
/*
   START-LOG-------------------------------------------

   $Log: style_hooks.h  $
   Revision 1.1 1993/04/19 16:31:13EDT builder 
   made from unix file
// Revision 1.2  1993/04/19  20:30:02  boris
// Fixed bug #3354 with style/category deassign.
//
// Revision 1.1  1993/01/17  23:57:15  boris
// Initial revision
//

   END-LOG---------------------------------------------

*/
