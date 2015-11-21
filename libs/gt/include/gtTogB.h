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
#ifndef _gtTogB_h
#define _gtTogB_h

// gtTogB.h
//------------------------------------------
// synopsis:
// class description of generic ToggleButton, nonexclusive (gtToggleButton)
//
//------------------------------------------

#ifndef _gtPrim_h
#include <gtPrim.h>
#endif

class gtToggleButton : public gtPrimitive
{
  public:
    // Creates a non-exclusive gtToggleButton specific to the toolkit.
    // Pre:  A valid parent gt object must be passed to it.  The callback
    // passed to it must have a valid prototype to be a gtToggleCB.
    // Post:  The returned pointer will point to a valid gtToggleButton
    // of the chosen toolkit.  The contents string will be written in the
    // button, and the callback argument will be called whenever the state
    // of the button changes.  
    static gtToggleButton* create(
	gtBase *parent, const char *name, const char *contents,
	gtToggleCB callback, void *client_data);

    ~gtToggleButton();

    // Determines whether or not the gtToggleButton has been selected.
    // Pre: The gtToggleButton must have been created for the appropriate
    // toolkit.
    // Post: Will return a boolean which is true if the gtToggleButton is
    // currently selected, false otherwise.
    virtual int set() = 0;

    // Pre: The gtToggleButton must have been created.
    // Post: The state of the gtToggleButton will be that specified by the
    // state argument.  If notify is true, then the usual callbacks will
    // be signalled.
    virtual void set(int state, int notify) = 0;

    // sets the pixmap of the label
    virtual void pixmap(const char *) = 0;

    // adds a new callback to the button
    virtual void set_callback(gtToggleCB, void *) = 0;

    // set or retrieve the contents of the button
    virtual void title(const char *) = 0;
    virtual char *title() = 0;

  protected:
    gtToggleButton();
};

#endif // _gtTogB_h

/*
   START-LOG-------------------------------------------

// $Log: gtTogB.h  $
// Revision 1.1 1993/07/28 19:47:54EDT builder 
// made from unix file
 * Revision 1.2.1.2  1992/10/09  18:13:19  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:35:14  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:35:13  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:20:01  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:27  smit
 * Initial revision
 * 
// Revision 1.5  92/09/11  11:12:41  builder
// Make destructor public.
// 
// Revision 1.4  92/09/11  04:07:56  builder
// Add ctor and dtor.
// 
// Revision 1.3  92/08/13  12:11:22  rfermier
// added set_callback, title functions.
// 
// Revision 1.2  92/07/30  23:45:29  glenn
// GT method name changes
// 
// Revision 1.1  92/07/08  20:39:23  rfermier
// Initial revision
// 
//------------------------------------------

   END-LOG---------------------------------------------
*/
