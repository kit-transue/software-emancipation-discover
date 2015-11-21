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
#ifndef _gtScrollbar_h
#define _gtScrollbar_h

// gtScrollbar.h
//------------------------------------------
// synopsis:
// class description of generic Scrollbar (gtScrollbar)
//
//------------------------------------------

#ifndef _gtPrim_h
#include <gtPrim.h>
#endif


class gtScrollbar : public gtPrimitive
{
  public:
    // Creates a gtScrollbar appropriate to the toolkit.
    static gtScrollbar* create(
	gtBase* parent, const char* name,
	gtScrollCB callback, void* client_data, gtDirection dir);

    ~gtScrollbar();

    // Sets the minimum and maximum values of the scrollbar
    virtual void set_limits(int min, int max) = 0;

    // Fetches or sets the current setting of the gtScrollbar.
    virtual int  value() = 0;
    virtual void value(int v) = 0;

    // Add a callback
    virtual void add_CB(
	gtScrollCB callback, void *client_data, gtReason rsn) = 0;

    // change the slider size 
    virtual void slider_size(int s) = 0;

    // Specify whether or not the arrows will be shown
    virtual void arrows (int flag) = 0;

  protected:
    gtScrollbar();
};


/*
   START-LOG-------------------------------------------

// $Log: gtScrollbar.h  $
// Revision 1.1 1993/07/29 10:36:23EDT builder 
// made from unix file
 * Revision 1.2.1.3  1993/01/26  05:11:12  glenn
 * cleanup
 *
// Revision 1.5  92/09/11  11:12:30  builder
// Make destructor public.
// 
// Revision 1.4  92/09/11  04:07:45  builder
// Add ctor and dtor.
// 
// Revision 1.3  92/07/30  23:45:00  glenn
// GT method name changes
// 
// Revision 1.2  92/07/16  13:56:30  rfermier
// added slider_size and arrows
// 
// Revision 1.1  92/07/08  20:39:16  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtScrollbar_h
