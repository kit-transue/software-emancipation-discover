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
#ifndef _gtShell_h
#define _gtShell_h

// gtShell.h
//------------------------------------------
// synopsis:
// Class definitions of the gtShell class
//
//------------------------------------------

#ifndef _gtBase_h
#include <gtBase.h>
#endif

class gtBitmap;

class gtShell : public gtBase
{
  public:
    int (*override_CB)(void*);

    ~gtShell();

    void popup();			// Realize and map the shell.
    void popdown();			// Unmap the shell.
    void bring_to_top();		// Bring the shell to the top of the stack.

    // Request a change in the location and/or size of a top-level window.
    void move(int x, int y);
    void resize(int w, int h);
    void move_resize(int x, int y, int w, int h);

    // Change the normal destruction behavior for when the window
    // is destroyed via the window-manager protocols.  If the provided
    // callback returns true, then the window will still be destroyed.

    void override_WM_destroy(int (*callback)(void*));

    static gtBitmap* default_bitmap(gtBitmap*);
    static gtBitmap* bitmap;

    void icon_title(const char*);

  protected:
    gtShell();

    void destroy_init();	// Extension of gtBase::destroy_init()
};

extern "C" void gtShell_init_focus_handler(void*);
extern "C" void gtShell_set_focus(void*);


/*
   START-LOG-------------------------------------------

   $Log: gtShell.h  $
   Revision 1.2 1994/07/08 17:59:48EDT builder 
   
 * Revision 1.2.1.6  1994/07/05  20:16:20  kws
 * Bug track: 7637
 * Implement icon titles on gtSHellm, and change icon titles for
 * the browser and viewerShell
 *
 * Revision 1.2.1.5  1993/12/17  16:07:57  jon
 * Bug track: 5090
 * Added code to gtShell to set a window manager hint as to what the
 * bitmap on our icon should be. Also added code to viewerShell.if for the
 * same reason.
 *
 * Revision 1.2.1.4  1993/04/30  01:48:01  glenn
 * Remove static version of bring_to_top.
 * Add gtShell_init_focus_handler, gtShell_set_focus.
 *
 * Revision 1.2.1.3  1993/01/20  03:43:13  glenn
 * Add move, resize, move_resize.
 *
// Revision 1.9  92/09/11  11:12:35  builder
// Make destructor public.
// 
// Revision 1.8  92/09/11  04:07:50  builder
// Add ctor and dtor.
// 
// Revision 1.7  92/09/11  00:40:23  glenn
// Remove static bring_to_top.
// Add popup, popdown.  Override destroy_init.
// 
// Revision 1.6  92/08/15  09:07:44  rfermier
// minor cleanup
// 
// Revision 1.5  92/07/30  23:45:04  glenn
// GT method name changes
// 
// Revision 1.4  92/07/30  13:50:14  rfermier
// added WM override capacity
// 
// Revision 1.3  92/07/17  13:40:19  rfermier
// added alternative form of bring_to_top
// 
// Revision 1.2  92/07/09  11:03:47  rfermier
// temporarily disable resize function
// 
// Revision 1.1  92/07/08  20:39:20  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtShell_h
