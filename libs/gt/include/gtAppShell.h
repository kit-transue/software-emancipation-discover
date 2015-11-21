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
#ifndef _gtAppShell_h
#define _gtAppShell_h

// gtAppShell.h
//------------------------------------------
// synopsis:
// GT Application shell
//
//------------------------------------------

#include<gtShell.h>

class gtApplicationShell : public gtShell
{
  public: 
    static gtRep *global_app_rep;

    // Use this function only once to create a new gtApplicatonShell.
    static gtApplicationShell* create(char *name, int argc, char *argv[]);

    ~gtApplicationShell();

    // Calling event_loop will make the interface start receiving and
    // handling events.  This is the function to "start" the interface going.
    virtual void event_loop() = 0;
};

#endif // _gtAppShell_h

/*
   START-LOG-------------------------------------------

 $Log: gtAppShell.h  $
 Revision 1.1 1994/03/23 19:23:06EST builder 
 made from unix file
 * Revision 1.2.1.3  1994/02/11  03:52:14  builder
 * Port
 *
 * Revision 1.2.1.2  1992/10/09  18:09:50  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:32:56  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:32:55  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:47  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:17  smit
 * Initial revision
 * 
// Revision 1.2  92/09/11  11:11:28  builder
// Make destructor public.
// 
// Revision 1.1  92/07/08  20:38:44  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------

 */
