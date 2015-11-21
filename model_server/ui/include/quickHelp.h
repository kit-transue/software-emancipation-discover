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
#ifndef _QuickHelp_h
#define _QuickHelp_h

#ifndef _x11_intrinsic_h
#include <x11_intrinsic.h>
#endif

#ifndef _prototypes_h
#include <prototypes.h>
#endif

BEGIN_C_DECL

void popup_QuickHelp PROTO(());
void destroy_QuickHelp PROTO(());

void qh_show PROTO((const char*));
void qh_show_notrans PROTO((const char*));
void qh_clear PROTO(());
void qh_register PROTO((Widget, const char*));
void qh_register_notrans PROTO((Widget, const char*));

END_C_DECL


/*
    START-LOG-------------------------------

   $Log: quickHelp.h  $
   Revision 1.1 1993/05/27 16:23:25EDT builder 
   made from unix file
 * Revision 1.2.1.5  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.4  1992/10/11  16:24:44  jon
 * Removed some comments that used // so this could be used in .c files
 *
 * Revision 1.2.1.3  92/10/10  16:29:33  builder
 * *** empty log message ***
 * 
 * Revision 1.2.1.2  92/10/09  19:57:15  kws
 * Fix comments
 * 
    END-LOG---------------------------------
*/

#endif // _QuickHelp_h
