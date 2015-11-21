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
#ifndef _gtTranShell_h
#define _gtTranShell_h

// gtTranShell.h
//------------------------------------------
// synopsis:
// Transient Shell
//
//------------------------------------------

#ifndef _gtShell_h
#include <gtShell.h>
#endif

class gtTransientShell : public gtShell
{
  public:
    // Creates a Transient Shell specific to the toolkit.
    // Pre: It must have a valid gt object parent passed to it.
    // Post: The returned pointer points to a valid gtTransientShell 
    // which is specific to the chosen toolkit.  If the title argument is 
    // passed, then that will be the title of the new shell.

    static gtTransientShell* create(
	gtBase *parent, const char *name);

    static gtTransientShell* create(
	gtBase *parent, const char *name, const char *title);
};

#endif // _gtTranShell_h

/*
   START-LOG-------------------------------------------

// $Log: gtTranShell.h  $
// Revision 1.1 1993/07/28 19:47:55EDT builder 
// made from unix file
 * Revision 1.2.1.2  1992/10/09  18:13:22  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:35:24  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:35:23  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:20:02  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:28  smit
 * Initial revision
 * 
// Revision 1.3  92/07/30  23:45:33  glenn
// GT method name changes
// 
// Revision 1.2  92/07/13  15:20:44  rfermier
// Added title paramter
// 
// Revision 1.1  92/07/08  20:39:26  rfermier
// Initial revision
// 
//------------------------------------------

   END-LOG---------------------------------------------
*/
