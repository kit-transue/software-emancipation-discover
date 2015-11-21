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
#ifndef _gtAppShellXm_h
#define _gtAppShellXm_h

// gtAppShellXm.h
//------------------------------------------
// synopsis:
// XMotif implementation of gtApplicationShell
//------------------------------------------

#include <gtAppShell.h>

#include <x11_intrinsic.h>
#include <gtBaseXm.h>


class gtApplicationShellXm: public gtApplicationShell
{
  public:
    static Widget find_existing();
    static XtAppContext get_context();

    gtApplicationShellXm(const char* name, int argc, char* argv[]);
    ~gtApplicationShellXm();

    void event_loop();
    gtBase* container();
};


/*
   START-LOG-------------------------------------------

// $Log: gtAppShellXm.h  $
// Revision 1.1 1993/03/04 01:19:13EST builder 
// made from unix file
 * Revision 1.2.1.3  1993/01/26  05:12:57  glenn
 * Fewer include files.
 *
 * Revision 1.2.1.2  1992/10/09  18:43:10  jon
 * RCS History Marker Fixup
 *
 * Revision 1.2.1.1  92/10/07  22:09:46  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  22:09:45  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:27:37  smit
 * Initial revision
 * 
// Revision 1.3  92/07/30  23:55:30  rfermier
// GT method name changes
// 
// Revision 1.2  92/07/16  10:38:33  rfermier
// Added get_context
// 
// Revision 1.1  92/07/08  20:40:43  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtAppShellXm_h
