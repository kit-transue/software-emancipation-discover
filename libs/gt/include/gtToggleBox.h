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
#ifndef _gtToggleBox_h
#define _gtToggleBox_h

// gtToggleBox.h
//------------------------------------------
// synopsis:
// class description of generic ToggleBox (gtToggleBox)
//
//------------------------------------------

#ifndef _gtPrim_h
#include <gtPrim.h>
#endif

class gtToggleBox : public gtPrimitive
{
  public:
    // Creates a gtToggleBox (a set of toggle buttons) 
    // appropriate to the toolkit.  The
    // variadic format used to describe the buttons themselves is as
    // follows:
    // char *button_title, char *button_name,
    // gtToggleCB callback, void *client_data
    // The variadic list is null-terminated.  The title argument sets
    // a title for the entire box.  If this is NULL, the title will be
    // excluded.
    // The prototype for gtToggleCB functions can be found in gtTogB.h.
    // Pre: A valid gt object parent must be passed to the function.  There
    // must be a full set of the variadic arguments for each button to
    // be created.  The function pointed to by the callback arguments
    // must be of suitable type.
    // Post: The returned pointer will point to a valid gtToggleBox which
    // will be filled with exclusive toggle buttons as described in the
    // variadic arguments.  Only one of these buttons will be selected
    // at any given time.  Whenever the state of the buttons is changed, the
    // the callback specified for that button will be called.  Note that this
    // means two callbacks will be generated for changing between one
    // Toggle button and a different one in the same box, one for the
    // deactivaton of the old selection and one for the activation of the
    // new one.
    static gtToggleBox* create(
	gtBase *parent, const char *name, const char *title,  ...);

    ~gtToggleBox();

    // Returns the number of the currently set button.  A zero means that 
    // the result is indeterminate (no button pressed or box in state of
    // change).
    // Pre: The gtToggleBox must have been created for the appropriate toolkit.
    // Post: The number returned will either be a zero, indicating some 
    // neutral or indeterminate state, or the number of the button which is
    // currently selected.
    virtual int item_set(int num) = 0;

    // Set the numth button to the state indicated by value (true or false),
    // and call the appropriate callbacks if flag is true.
    virtual void item_set(int num, int value, int flag) = 0;

    // Return the gtToggleButton whose name is nm
    virtual gtToggleButton* button(const char *) = 0;

    // Change the layout of the box to have N columns
    virtual void num_columns(int n) = 0;

  protected:
    gtToggleBox();
};

#endif // _gtToggleBox_h

/*
   START-LOG-------------------------------------------

// $Log: gtToggleBox.h  $
// Revision 1.1 1993/07/28 19:47:54EDT builder 
// made from unix file
 * Revision 1.2.1.2  1992/10/09  18:13:20  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:35:18  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:35:17  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:20:01  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:27  smit
 * Initial revision
 * 
// Revision 1.5  92/09/11  11:12:43  builder
// Make destructor public.
// 
// Revision 1.4  92/09/11  04:07:57  builder
// Add ctor and dtor.
// 
// Revision 1.3  92/07/30  23:45:31  glenn
// GT method name changes
// 
// Revision 1.2  92/07/09  12:54:31  rfermier
// Added multiple column support, more documentation
// 
// Revision 1.1  92/07/08  20:39:24  rfermier
// Initial revision
// 
//------------------------------------------

   END-LOG---------------------------------------------
*/
