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
#ifndef _baseBrowser_h
#define _baseBrowser_h

// baseBrowser.h
//------------------------------------------
// synopsis:
// Common base class for file and project browser.
//
//------------------------------------------

#ifndef _gt_h
#include <gt.h>
#endif


class browserShell;

class baseBrowser
{
  public:
    virtual ~baseBrowser();

    void select();
    void up();
    void back();

    gtPrimitive* top();
    gtStringEditor* gt_string_ed;

    gtStringEditor* scope_ed;
    gtOptionMenu*scope_mode_menu;

    virtual void filter(const char*) = 0;
    virtual void deselect() = 0;
    static void list_action(gtList*, gtEventPtr, void*, gtReason);

  protected:
    static void filter_CB(gtStringEditor*, gtEventPtr, void*, gtReason);
//  static void list_action(gtList*, gtEventPtr, void*, gtReason); // made public

    browserShell& bsh;
    gtForm* gt_form;
//    gtStringEditor* gt_string_ed;

    baseBrowser(browserShell*, const char* label);

    void build_interface(const char*);

    virtual gtPrimitive* build_body(gtBase*) = 0;
};


/*
    START-LOG-------------------------------

    $Log: baseBrowser.h  $
    Revision 1.3 1996/11/28 15:46:58EST mg 
    
 * Revision 1.2.1.5  1993/06/15  20:14:55  andrea
 * fixed bug 3613
 *
 * Revision 1.2.1.4  1993/01/10  06:09:29  glenn
 * Add label to ctor.
 *
 * Revision 1.2.1.3  1993/01/07  14:51:16  glenn
 * New browserShell interface.
 *
 * Revision 1.2.1.2  1992/10/09  19:56:34  kws
 * Fix comments
 *
    END-LOG---------------------------------
*/

#endif // _baseBrowser_h
