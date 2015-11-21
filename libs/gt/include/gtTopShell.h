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
#ifndef _gtTopShell_h
#define _gtTopShell_h

// gtTopShell.h
//------------------------------------------
// synopsis:
// GT TopLevel Shell class definitions
//
//------------------------------------------

#ifndef _gtShell_h
#include <gtShell.h>
#endif

class gtTopLevelShell : public gtShell
{
  public:
    // Create a top-level window.  There must have been a successful
    // creation of a gtApplicationShell.
    static gtTopLevelShell* create(gtBase*, const char*);

    ~gtTopLevelShell();

    // Resets the background of the the window to a cleared state.
    virtual void clear_background() = 0;

    virtual void destroy_callback(gtTLShellCB callback, void *client_data) = 0;

    virtual void title(const char*) = 0;
    virtual void min_width(int) = 0;
    virtual void min_height(int) = 0;
    virtual void max_width(int) = 0;
    virtual void max_height(int) = 0;

  protected:
    gtTopLevelShell();
};

#endif // _gtTopShell_h

/*
   START-LOG-------------------------------------------

// $Log: gtTopShell.h  $
// Revision 1.1 1993/12/08 09:59:31EST builder 
// made from unix file
 * Revision 1.2.1.3  1993/12/07  20:44:06  andrea
 * Bug track: 5197
 * I added min_width, min_height, max_width, max_height member functions
 *
 * Revision 1.2.1.2  1992/10/09  18:13:21  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:35:21  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:35:20  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:20:01  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:27  smit
 * Initial revision
 * 
// Revision 1.7  92/09/24  23:39:31  glenn
// Add title.
// 
// Revision 1.6  92/09/11  11:12:45  builder
// Make destructor public.
// 
// Revision 1.5  92/09/11  08:17:57  wmm
// Fix compilation errors due to incorrect protected/private access.
// 
// Revision 1.4  92/09/11  04:07:58  builder
// Add ctor and dtor.
// 
// Revision 1.3  92/09/11  00:43:04  glenn
// Remove class_name from ctor.
// 
// Revision 1.2  92/07/30  23:45:32  glenn
// GT method name changes
// 
// Revision 1.1  92/07/08  20:39:25  rfermier
// Initial revision
// 
//------------------------------------------

   END-LOG---------------------------------------------
*/
