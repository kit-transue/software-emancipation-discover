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
#ifndef _button_bar_h
#define _button_bar_h

#ifndef _prototypes_h
#include <prototypes.h>
#endif

BEGIN_C_DECL
#ifdef __cplusplus
int  viewerShell_finalize (viewerShell*, Widget);
void viewerShell_post_hook (Widget);
#else
int  viewerShell_finalize ();
void viewerShell_post_hook ();
#endif
END_C_DECL

#define BB_BEGIN(vs) { if (viewerShell_finalize(vs, UxWidget)) { {
#define BB_END       } viewerShell_post_hook(UxWidget); } }

enum vsSpecialButtons
{
    vsSpecialButtonsNone,
    vsSpecialButtonsHyper,
    vsSpecialButtonsDebug
};


/*
    START-LOG-------------------------------

    $Log: button_bar.h  $
    Revision 1.5 2000/07/07 08:18:01EDT sschmidt 
    Port to SUNpro 5 compiler
 * Revision 1.2.1.6  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.5  1992/12/12  07:09:57  glenn
 * Add vsSpecialButtons.
 *
 * Revision 1.2.1.4  1992/10/14  12:57:34  glenn
 * Fix prototype of viewerShell_post_hook.
 *
 * Revision 1.2.1.3  92/10/14  12:12:25  glenn
 * Add viewerShell_post_hook to BB_END macro.
 * 
 * Revision 1.2.1.2  92/10/09  19:56:36  kws
 * Fix comments
 * 
    END-LOG---------------------------------
*/

#endif /* _button_bar_h */
