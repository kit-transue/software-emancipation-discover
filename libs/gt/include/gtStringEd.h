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
#ifndef _gtStringEd_h 
#define _gtStringEd_h

// gtStringEd.h
//------------------------------------------
// synopsis:
// class description of generic StringEditor (gtStringEditor)
//
//------------------------------------------

#ifndef _gtPrim_h
#include <gtPrim.h>
#endif

class gtStringEditor : public gtPrimitive
{
  public:
    // Creates a gtStringEditor appropriate to the toolkit.
    // Note that gtStringEditors are for single-line work while gtTextEditors
    // are for multi-line work.
    // Pre: Must have a valid gt object parent passed to it.
    // Post: The returned pointer will point to a valid gtStringEditor.
    static gtStringEditor* create(
	gtBase *parent, const char *name, const char *contents);

    ~gtStringEditor();

    virtual long get_last_position() = 0;
    virtual char *text() = 0;
    virtual void text(const char *) = 0;
    virtual void set_selection(long first, long last) = 0;
    virtual void set_callback(gtStringCB callback, void *client_data) = 0;
    virtual void editable(int) = 0;
    virtual void read_only() = 0;
    virtual void columns(int c) = 0;

    virtual void activate_callback(gtStringCB callback, void *client_data) = 0;
    virtual void focus_callback(gtStringCB callback, void *client_data) = 0;
    virtual void unfocus_callback(gtStringCB callback, void *client_data) = 0;
    virtual void show_cursor(int) = 0;
    virtual void hide_unfocused_cursor() = 0;
    virtual void changed_callback(gtStringCB callback, void *client_data) = 0;

  protected:
    gtStringEditor();
};

#endif // _gtStringEd_h

/*
   START-LOG-------------------------------------------

// $Log: gtStringEd.h  $
// Revision 1.1 1993/07/29 10:36:25EDT builder 
// made from unix file
 * Revision 1.2.1.3  1993/01/21  14:57:40  jon
 * Added member function changed_callback
 *
 * Revision 1.2.1.2  1992/10/09  18:10:43  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:35:04  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:35:03  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:20:00  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:26  smit
 * Initial revision
 * 
// Revision 1.6  92/09/11  11:12:37  builder
// Make destructor public.
// 
// Revision 1.5  92/09/11  04:07:52  builder
// Add ctor and dtor.
// 
// Revision 1.4  92/08/16  20:41:22  glenn
// Add activate_callback, focus_callback, unfocus_callback,
// show_cursor, hide_unfocused_cursor.
// 
// Revision 1.3  92/07/30  23:45:05  glenn
// GT method name changes
// 
// Revision 1.2  92/07/24  11:06:58  glenn
// Use const char* when appropriate.
// 
// Revision 1.1  92/07/08  20:39:21  rfermier
// Initial revision
// 
//------------------------------------------

   END-LOG---------------------------------------------
*/
