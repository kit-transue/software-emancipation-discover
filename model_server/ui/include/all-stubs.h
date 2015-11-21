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
#ifndef _all_stubs_h
#define _all_stubs_h

#ifndef DESIGN_TIME

#ifndef  _prototypes_h
#include <prototypes.h>
#endif

BEGIN_C_DECL

/* List of all known ui-section external calls */

char *dialog_name PROTO((void *dialog));
int dialog_untitled PROTO((void *dialog));
void dialog_create_window PROTO((void *dialog, void *window));
void dialog_resize_window PROTO((void *dialog, void *window));

END_C_DECL

/*
    START-LOG-------------------------------

   $Log: all-stubs.h  $
   Revision 1.3 2000/01/05 15:06:13EST sschmidt 
   Pruning more OODT stuff
 * Revision 1.2.1.5  1994/02/09  16:55:56  builder
 * Port
 *
 * Revision 1.2.1.4  1994/02/09  01:04:52  builder
 * Port
 *
 * Revision 1.2.1.3  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.2  1992/10/09  19:56:32  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif /* DESIGN_TIME */
#endif /* _all_stubs_h */
