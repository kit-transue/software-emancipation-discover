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
#ifndef _gtArrowButton_h
#define _gtArrowButton_h

// gtArrowButton.h
//------------------------------------------
// synopsis:
// class description of generic ArrowButton (gtArrowButton)
//
//------------------------------------------

#ifndef _gtPrim_h
#include <gtPrim.h>
#endif

class gtArrowButton : public gtPrimitive
{
  public:
    // Creates a gtArrowButton appropriate to the toolkit.
    // Pre: Must have a valid gt object parent passed to it, and the function
    // specified by the callback argument must match the prototype of 
    // a gtArrowCB, shown above.
    // Post: The pointer returned will point to a valid gtArrowButton for
    // the toolkit.  When mapped, the button contains the text of the
    // contents argument.  When Arrowed, the callback argument will be
    // invoked.
    static gtArrowButton* create(
	gtBase *parent, const char *name, gtCardinalDir dir,
	gtArrowCB callback, void *client_data);

    ~gtArrowButton();

    // Register a new callback with the gtArrowButton
    // Pre: The gtArrowButton must have been created for the appropriate
    // toolkit.
    // Post: When the gtArrowButton is pressed, the specified callback
    // will be called with the client_data set accordingly.
    virtual void set_callback(gtArrowCB callback, void *client_data) = 0;

    virtual void set_direction(gtCardinalDir) = 0;
    virtual void shadow_thickness(int thick) = 0;

  protected:
    gtArrowButton();
};

#endif // _gtArrowButton_h

/*
   START-LOG-------------------------------------------

 $Log: gtArrowButton.h  $
 Revision 1.1 1993/07/28 19:47:37EDT builder 
 made from unix file
 * Revision 1.2.1.2  1992/10/09  18:09:53  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:33:00  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:32:59  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:47  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:17  smit
 * Initial revision
 * 
// Revision 1.5  92/09/11  11:11:46  builder
// Make destructor public.
// 
// Revision 1.4  92/09/11  04:06:51  builder
// Add ctor and dtor.
// 
// Revision 1.3  92/07/30  23:44:19  glenn
// GT method name changes
// 
// Revision 1.2  92/07/28  17:27:19  rfermier
// Moved cardinalDir to gtBase
// 
// Revision 1.1  92/07/08  20:38:47  rfermier
// Initial revision
// 

   END-LOG---------------------------------------------

*/
