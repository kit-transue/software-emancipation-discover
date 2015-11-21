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
#ifndef _gtPDMenu_h
#define _gtPDMenu_h

// gtPDMenu.h
//------------------------------------------
// synopsis:
// class description of generic PulldownMenu (gtPulldownMenu)
//
//------------------------------------------

#ifndef _gtPrim_h
#include <gtPrim.h>
#endif


class gtPulldownMenu : public gtPrimitive
{
  public:
    // Creates a gtPulldownMenu appropriate to the toolkit.  Format for
    // the variadic arguments is as follows:
    // for gtMenuStandard:
    //     gtMenuChoice entry, char *name, char *contents,
    //     void *client_data, gtPushCB callback
    // for gtMenuCheck or gMenuRadio:
    //     gtMenuChoice entry, char *name, char *contents,
    //     void *client_data, gtToggleCB callback
    // for gtMenuCascade:
    //     gtMenuChoice entry, char *name, char *contents,
    //     void *client_data, gtCascadeCB callback
    // for gtMenuSeparator:
    //     gtMenuChoice entry, char *name
    // for gtMenuLabel:
    //     gtMenuChoice entry, char *name, char *contents
    // The variadic list is null-terminated.

    static gtPulldownMenu* create(gtBase *parent, const char *name ...);

    ~gtPulldownMenu();

    // Inserts entries into the menu using the same variadic format
    // as create uses.  Location 0 means the top of the menu, and location
    // -1 means the end of the menu.
    virtual void insert_entries(int loc ...) = 0; 

    // Removes the entry from the menu.
    // where entry 0 is the earliest entry in the menu.
    virtual void remove_entry(int loc) = 0;

    // Returns the nth entry in the menu,
    // where entry 0 is the earliest entry in the menu.
    virtual gtPrimitive* entry(int n) = 0;

    // Returns the nth cascade in the menu,
    // where cascade 0 is the earliest entry in the menu.
    virtual gtCascadeButton *cascade(int n) = 0;

    virtual void _filler1_() = 0;
    virtual void _filler2_() = 0;

    // returns the entry named name
    virtual gtPrimitive *button(const char *name) = 0;

    // sets a callback to be called when the menu is mapped
    virtual void map_callback(gtPulldownCB, void*) = 0;

  protected:
    gtPulldownMenu();
};

#endif

/*
   START-LOG-------------------------------------------

// $Log: gtPDMenu.h  $
// Revision 1.1 1993/07/29 10:36:15EDT builder 
// made from unix file
 * Revision 1.2.1.4  1993/01/26  05:09:07  glenn
 * Change total_entries to _filler1_.
 * Change most_recent to _filler2_.
 *
 * Revision 1.2.1.3  1992/11/30  21:14:36  smit
 * Use stdarg not stdargs.
 *
 * Revision 1.2.1.2  1992/10/09  18:10:23  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:34:13  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:34:11  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:56  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:23  smit
 * Initial revision
 * 
// Revision 1.6  92/09/11  11:12:17  builder
// Make destructor public.
// 
// Revision 1.5  92/09/11  04:44:41  builder
// Fix typo.
// 
// Revision 1.4  92/09/11  04:07:35  builder
// Add ctor and dtor.
// 
// Revision 1.3  92/07/31  16:39:34  rfermier
// added callbacks
// 
// Revision 1.1  92/07/08  20:39:06  rfermier
// Initial revision
// 
//------------------------------------------

   END-LOG---------------------------------------------
*/
