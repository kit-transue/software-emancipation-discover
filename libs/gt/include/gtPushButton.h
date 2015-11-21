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
#ifndef _gtPushButton_h
#define _gtPushButton_h

// gtPushButton.h
//------------------------------------------
// synopsis:
// class description of generic Push Button (gtPushButton)
//
//------------------------------------------

#ifndef _gtPrim_h
#include <gtPrim.h>
#endif

class gtBitmap;

class gtPushButton : public gtPrimitive
{
  public:
    // Creates a gtPushButton appropriate to the toolkit.
    static gtPushButton* create(
	gtBase *parent, const char *name, const char *contents,
	gtPushCB callback, void *client_data);

    static gtPushButton* create(
	gtBase *parent, const char *name, const char *contents,
	gtPushCB callback, void *client_data, gtAlign);

    static gtPushButton* create(
	gtBase *parent, const char *name, gtBitmap* contents,
	gtPushCB callback, void *client_data);

    // Set the help-context for the next button created that is called "help".
    static void next_help_context(const char*);
    static const char* next_help_context();

    ~gtPushButton();

    // Register a new callback with the gtPushButton
    virtual void set_callback(gtPushCB callback, void *client_data) = 0;

    virtual void shadow_thickness(int thick) = 0;
    virtual void title(const char *t) = 0;
    virtual char *title() = 0;

  protected:
    gtPushButton();
};

#endif // _gtPushButton_h

/*
   START-LOG-------------------------------------------

// $Log: gtPushButton.h  $
// Revision 1.1 1993/11/18 09:48:12EST builder 
// made from unix file
 * Revision 1.2.1.5  1993/11/17  19:48:25  jon
 * Bug track: 0
 * added constructor for push buttons with gtBitmaps
 * added member to return help context
 *
 * Revision 1.2.1.4  1993/04/19  14:09:57  glenn
 * Add next_help_context.
 *
 * Revision 1.2.1.3  1993/01/22  17:17:22  glenn
 * Add create function that specifies alignment.
 *
 * Revision 1.2.1.2  1992/10/09  18:10:28  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:34:26  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:34:25  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:57  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:24  smit
 * Initial revision
 * 
// Revision 1.6  92/09/11  11:12:24  builder
// Make destructor public.
// 
// Revision 1.5  92/09/11  04:07:39  builder
// Add ctor and dtor.
// 
// Revision 1.4  92/08/13  12:10:39  rfermier
// added title()
// 
// Revision 1.3  92/07/30  23:44:54  glenn
// GT method name changes
// 
// Revision 1.2  92/07/20  13:28:25  rfermier
// Added title function
// 
// Revision 1.1  92/07/08  20:39:11  rfermier
// Initial revision
// 
//------------------------------------------

   END-LOG---------------------------------------------
*/
