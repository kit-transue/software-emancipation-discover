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
#ifndef _gtPopMenu_h
#define _gtPopMenu_h

// gtPopMenu.h
//------------------------------------------
// synopsis:
// class description of generic PopupMenu (gtPopupMenu)
//
//------------------------------------------

#ifndef _gtPrim_h
#include <gtPrim.h>
#endif

class gtPopupMenu : public gtPrimitive
{
  public:
    // Creates a gtPopupMenu appropriate to the toolkit.  Format for
    // the variadic arguments is as follows:
    // for gtMenuStandard:
    // gtMenuChoice entry, char *name, char *contents,
    // 		void *client_data, gtPushCB callback
    // for gtMenuCheck:
    // gtMenuChoice entry, char *name, char *contents, void *client_data,
    // 		gtToggleCB callback
    // for gtMenuCascade:
    // gtMenuChoice entry, char *name, char *contents, void *client_data,
    // 		gtCascadeCB callback
    // for gtMenuSeparator:
    // gtMenuChoice entry, char *name
    // for gtMenuLabel:
    // gtMenuChoice entry, char *name, char *contents
    // The variadic list is null-terminated.
    // Pre: Must have a valid gt object parent passed to it.  The variadic
    // arguments must be in the correct order and in complete sets.  Any
    // callback passed to it must be appropriately prototyped to the menu
    // entry that it belongs with.
    // Post: The returned pointer points to a valid gtPopupMenu
    // for the specific toolkit.
    static gtPopupMenu* create(gtBase *parent, const char *name ...);

    ~gtPopupMenu();

    // Moves the gtPopupMenu to the location of the buttonpress
    // represented by the gtEvent argument.  This should be used right
    // before managing the object in order to make it appear in the 
    // correct location
    virtual void move_to(gtEventPtr) = 0;

    // Inserts entries into the menu using the same variadic format
    // as create uses.  Location 0 means the top of the menu, and location
    // -1 means the end of the menu.
    // Pre: Must have been created for the appropriate toolkit
    // Post: The specified entries will now be in the gtPopupMenu.
    virtual void insert_entries(int loc ...) = 0; 

    // Removes the entry from the Menu.  Locations 0 & -1 are the
    // same as for insert_entries.
    // Pre: The gtPopupMenu has to have been created, and the location
    // specified has to exist.
    virtual void remove_entry(int loc) = 0;

    // Returns the nth entry in the menu, where entry 0 is the earlier
    // entry in the menu.
    // Pre: There must be at least n+1 entries in the menu, and the menu
    // must have already been created for a particular toolkit.
    // Post: The returned pointer will point to the gtPrimitive which
    // is the nth entry in the menu.
    virtual gtPrimitive* entry(int n) = 0;

    // adds a callback to be called when the menu is mapped
    virtual void map_callback(gtPopupCB, void *) = 0;

  protected:
    gtPopupMenu();
};

#endif // _gtPopMenu_h

/*
   START-LOG-------------------------------------------

// $Log: gtPopMenu.h  $
// Revision 1.1 1993/07/28 19:47:47EDT builder 
// made from unix file
 * Revision 1.2.1.2  1992/10/09  18:10:26  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:34:20  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:34:18  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:56  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:23  smit
 * Initial revision
 * 
// Revision 1.6  92/09/24  21:38:40  glenn
// Remove cascade member function.
// 
// Revision 1.5  92/09/11  11:12:21  builder
// Make destructor public.
// 
// Revision 1.4  92/09/11  04:07:37  builder
// Add ctor and dtor.
// 
// Revision 1.3  92/07/31  16:39:06  rfermier
// added callbacks
// 
// Revision 1.2  92/07/30  23:44:52  glenn
// GT method name changes
// 
// Revision 1.1  92/07/08  20:39:08  rfermier
// Initial revision
// 
//------------------------------------------

   END-LOG---------------------------------------------
*/
