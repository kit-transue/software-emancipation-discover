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
#ifndef _x11_intrinsic_h
#define _x11_intrinsic_h

/*
 * x11_intrinsic.h
 *
 * This is a convenience "wrapper" for using <X11/Intrinsic.h> with C++.
 */

#define Object XObject
#define XTFUNCPROTO

#ifndef X_H
#include <X11/X.h>
#endif

#ifndef _XLIB_H_
#include <X11/Xlib.h>
#endif

#ifndef _XtIntrinsic_h
#include <X11/Intrinsic.h>
#endif

#undef Object

/*
    START-LOG-------------------------------

 * $Log: x11_intrinsic.h  $
 * Revision 1.2 1996/02/28 10:03:46EST rajan 
 * remove c_machdep.h
 * Revision 1.2.1.6  1993/09/23  15:14:37  kws
 * Bug track: Port
 * Portability
 *
 * Revision 1.2.1.5  1993/05/27  20:17:16  glenn
 * Protect nested includes using protect_includes script.
 *
 * Revision 1.2.1.4  1993/01/29  15:35:08  smit
 * *** empty log message ***
 *
 * Revision 1.2.1.3  1993/01/29  07:37:17  glenn
 * Include more files before X11/Intrinsic.h to reduce cpp failures.
 *
 * Revision 1.2.1.2  1992/10/09  19:57:35  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif
