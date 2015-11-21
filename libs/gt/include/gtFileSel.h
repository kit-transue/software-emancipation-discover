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
#ifndef _gtFileSel_h
#define _gtFileSel_h

// gtFileSel.h
//------------------------------------------
// synopsis:
// class description of generic FileSelector (gtFileSelector)
//
//------------------------------------------

#ifndef _gtDialog_h
#include <gtDialog.h>
#endif

class gtFileSelector : public gtDialog
{
  public:
    // Creates a gtFileSelector appropriate to the toolkit.
    // If the callback information is supplied, then those callbacks
    // will be called by the OK and Cancel buttons, respectively.
    // Pre: Must have a valid gt object parent passed to it.
    // Post: The returned pointer will point to a valid gtFileSelector.
    static gtFileSelector* create(gtBase *parent, const char *name);
    static gtFileSelector* create(gtBase *parent, const char *name,
				  gtPushCB OK_CB, void *OK_data,
				  gtPushCB Cancel_CB, void *Cancel_data);

    ~gtFileSelector();

    // Returns the name of the selected file.
    // Pre: Must have created the gtFileSelector for the apporpriate toolkit.
    // Post: The string returned is the name of the selected file.
    virtual char *selected() = 0;

    // Returns the appropriate component of the box
    virtual gtPushButton *help_button() = 0;
    virtual gtPushButton *ok_button() = 0;
    virtual gtPushButton *cancel_button() = 0;
    virtual gtLabel *select_label() = 0;
    virtual gtLabel *list_label() = 0;
    virtual gtStringEditor *text() = 0;
    virtual gtList *list() = 0;
    virtual gtList *dir_list() = 0;

    // Set the pattern
    virtual void pattern (const char *) = 0;

    // Set the title of the dialog
    virtual void title(const char *) = 0;

    // Retrieve the title of the dialog
    virtual char *title() = 0;

    // Set the selection label
    virtual void select_label(const char *) = 0;

    // Set the file selector in or out of directory mode
    virtual void dir_mode(int mode) = 0;

  protected:
    gtFileSelector();
};

#endif // _gtFileSel_h

/*
   START-LOG-------------------------------------------

 $Log: gtFileSel.h  $
 Revision 1.1 1993/07/28 19:47:41EDT builder 
 made from unix file
 * Revision 1.2.1.2  1992/10/09  18:10:07  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:33:28  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:33:26  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:49  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:19  smit
 * Initial revision
 * 
// Revision 1.10  92/09/11  11:12:00  builder
// Make destructor public.
// 
// Revision 1.9  92/09/11  04:07:19  builder
// Add ctor and dtor.
// 
// Revision 1.8  92/08/06  18:12:22  rfermier
// new constructor, hierarchy change, dir_mode
// 
// Revision 1.7  92/07/30  23:44:38  glenn
// GT method name changes
// 
// Revision 1.6  92/07/22  13:39:26  rfermier
// Added more child access functions
// 
// 
// Revision 1.5  92/07/17  14:34:38  rfermier
// *** empty log message ***
// 
// Revision 1.4  92/07/17  14:31:49  rfermier
// fixed virtual typo
// 
// Revision 1.3  92/07/17  13:39:36  rfermier
// added serious functionality
// 
// Revision 1.2  92/07/14  13:07:14  rfermier
// Eliminated gtHelp conflict
// 
// Revision 1.1  92/07/08  20:38:55  rfermier
// Initial revision
// 

   END-LOG---------------------------------------------
*/

