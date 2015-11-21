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
#ifndef _gtCascadeB_h
#define _gtCascadeB_h

// gtCascadeB.h
//------------------------------------------
// synopsis:
// class description of generic CascadeButton (gtCascadeButton)
//
//------------------------------------------

#ifndef _gtPrim_h
#include <gtPrim.h>
#endif

class gtCascadeButton : public gtPrimitive
{
  public:
    // Creates a gtCascadeButton appropriate to the toolkit.  Make SURE you
    // set the sub_menu shortly after creating a gtCascadeButton!
    // Pre: Must have a valid gt object parent passed to it.
    // Post: The pointer returned will point to a valid gtCascadeButton for
    // the toolkit.  When mapped, the button contains the text of the
    // contents argument.  When pushed, the callback argument will be
    // invoked.  
    static gtCascadeButton* create(
	gtBase *parent, const char *name, const char *contents,
	gtCascadeCB callback, void *client_data);

    ~gtCascadeButton();

    // Determines which gt object is called up when the Cascade button is
    // clicked on.  The gtPrimitive pointer that you pass to it should 
    // be a pointer to one of the gt menu classes.
    // Pre: The gtCascadeButton must have already been created for a 
    // particular toolkit.  The gtPrimitive that is pointed to by the sub
    // argument must be of a type that can be popped up appropriately.
    // Post: When the gtCascadeButton is clicked on, it will pop up the
    // gt object pointed to by the sub parameter.
    virtual void sub_menu(gtPrimitive *sub) = 0;

    // Set the text of the label
    virtual void label(const char *) = 0;

    // Variadically create the menu which is opened when the button
    // is pressed.  Variadic format is same as for gtPopupMenu.

    virtual void pulldown_menu(const char *name ...) = 0;
    virtual void insert_entries(int loc, ...) =0;
    virtual void remove_entry(int loc) = 0;
    virtual gtPrimitive *entry(int n) = 0;
    virtual gtCascadeButton *cascade(int) = 0;
    virtual gtPrimitive *button(const char *) = 0;

  protected:
    gtCascadeButton();
};

#endif // _gtCascadeB_h

/*
   START-LOG-------------------------------------------

 $Log: gtCascadeB.h  $
 Revision 1.1 1993/07/28 19:47:38EDT builder 
 made from unix file
 * Revision 1.2.1.2  1992/10/09  18:09:57  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:33:10  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:33:09  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:48  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:18  smit
 * Initial revision
 * 
// Revision 1.8  92/09/11  11:11:52  builder
// Make destructor public.
// 
// Revision 1.7  92/09/11  04:07:09  builder
// Add ctor and dtor.
// 
// Revision 1.6  92/08/11  10:28:49  rfermier
// Better access to associated pulldown, added pulldown_menu to replace
// gtPulldownMenu class
// 
// Revision 1.5  92/08/10  10:26:13  rfermier
// *** empty log message ***
// 
// Revision 1.4  92/08/10  09:52:27  rfermier
//  added pulldown_menu function
// 
// Revision 1.3  92/07/30  23:44:34  glenn
// GT method name changes
// 
// Revision 1.2  92/07/29  18:41:18  rfermier
// added label(const char *)
// 
// Revision 1.1  92/07/08  20:38:51  rfermier
// Initial revision
// 

   END-LOG---------------------------------------------

*/
