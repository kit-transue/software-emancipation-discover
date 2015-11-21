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
/*
// dialog_set_current_window
//------------------------------------------
// synopsis:
// sets uimx window global
//
// description:
// Method to set the global current_root_window:
//   dialog_set_current_window
//------------------------------------------
// $Log: dialog_set_current_window.c  $
// Revision 1.3 1994/07/27 12:15:31EDT jethran 
// comment is in RCS log
 * Revision 1.2.1.2  1994/07/26  23:47:02  jethran
 * changed #include uimx to #include X
 *
 * Revision 1.2.1.1  1992/10/07  20:37:23  smit
 * *** empty log message ***
 *
 * Revision 1.2  92/10/07  20:37:22  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:20:15  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:52  smit
 * Initial revision
 * 
 * Revision 1.2  92/08/23  17:46:23  builder
 * defined Object
 * 
 * Revision 1.1  91/09/01  23:32:41  kws
 * Initial revision
 * 
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------
*/

/* include files */
#include <X11/Xlib.h>

/* global variable declaration */

Window current_root_window;

/*
//------------------------------------------
// dialog_set_current_window
//
// This functions setsup the parent window so InterViews can apply some
// parenting magic
//------------------------------------------
*/

void dialog_set_current_window(win)
void* win;
{
    current_root_window = (Window)win;
}
