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
#ifndef _gtOptionMenu_h
#define _gtOptionMenu_h

// gtOptionMenu.h
//------------------------------------------
// synopsis:
// class description of generic OptionMenu (gtOptionMenu)
//
//------------------------------------------

#ifndef _gtPrim_h
#include <gtPrim.h>
#endif

class gtOptionMenu : public gtPrimitive
{
  public:
    // Creates a gtOptionMenu appropriate to the toolkit.
    // Format for the variadic arguments is as follows:
    // for gtMenuStandard:
    //     gtMenuChoice entry, char *name, char* contents,
    //     void *client_data, gtPushCB callback
    // for gtMenuCheck:
    //     gtMenuChoice entry, char *name, char *contents,
    //     void *client_data, gtToggleCB callback
    // for gtMenuCascade:
    //     gtMenuChoice entry, char *name, char *contents,
    // 	   void *client_data, gtCascadeCB callback
    // for gtMenuSeparator:
    //     gtMenuChoice entry, char *name
    // for gtMenuLabel:
    //     gtMenuChoice entry, char *name, char *contents
    // The variadic list is null-terminated.

    static gtOptionMenu* create(
	gtBase *parent, const char *name, const char *title ...);

    ~gtOptionMenu();

    // Inserts entries into the menu using the same variadic format
    // as create uses.  Location 0 means the top of the menu, and location
    // -1 means the end of the menu.
    virtual void insert_entries(int loc ...) = 0;

    // Removes the entry from the Menu.  Locations 0 & -1 are the
    // same as for insert_entries.
    virtual void remove_entry(int loc) = 0;

    // Returns the nth entry in the menu, where entry 0 is the earliest
    // entry in the menu.
    virtual gtPrimitive *entry(int n) = 0;

    // Returns the nth cascade in the menu, where cascade 0 is the earliest
    // entry in the menu.
    virtual gtCascadeButton *cascade(int n) = 0;

    // Returns the gtLabel which acts as the label component of the
    // Option Menu.
    virtual gtLabel *label() = 0;

    // returns the total number of entries in the menu
    virtual int total_entries() = 0;

    virtual void _filler_() = 0;

    // sets the menu_history of the option menu
    virtual void menu_history(int index) = 0;

    // returns the menu that the option menu creates
    virtual gtPulldownMenu *sub_menu() = 0;

    // returns the menu item object named name
    virtual gtPrimitive *button(const char *) = 0;

    // returns the name of the menu_history entry
    virtual char *menu_history() = 0;

    // sets a callback to be called when the menu is mapped
    virtual void map_callback(gtOptionCB, void *) = 0;

    // sets the menu history to the entry named name
    virtual void menu_history(const char *) = 0;

    // sets the font on the top-level cascade button
    virtual void font (const char*) = 0;

  protected:
    gtOptionMenu();
};


/*
   START-LOG-------------------------------------------

// $Log: gtOptionMenu.h  $
// Revision 1.2 1995/05/17 14:04:56EDT jerry 
// Bug fixes, phase II of new links.
 * Revision 1.2.1.3  1993/01/26  05:08:39  glenn
 * Change most_recent to _filler_.
 *
 * Revision 1.2.1.2  1992/10/09  18:10:22  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:34:09  smit
 * *** empty log message ***
 *
 * Revision 1.2  92/10/07  20:34:08  smit
 * *** empty log message ***
 *
 * Revision 1.1  92/10/07  18:19:55  smit
 * Initial revision
 *
 * Revision 1.1  92/10/07  17:55:22  smit
 * Initial revision
 *
// Revision 1.6  92/09/11  11:12:16  builder
// Make destructor public.
//
// Revision 1.5  92/09/11  04:07:33  builder
// Add ctor and dtor.
//
// Revision 1.4  92/07/31  16:40:11  rfermier
// added callbacks, expanded menu_history functions
//
// Revision 1.3  92/07/30  23:44:49  glenn
// GT method name changes
//
// Revision 1.2  92/07/22  13:39:58  rfermier
// added named function
// added more docs
//
// Revision 1.1  92/07/08  20:39:05  rfermier
// Initial revision
//
   END-LOG---------------------------------------------
*/

#endif // _gtOptionMenu_h
