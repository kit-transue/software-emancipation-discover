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
#ifndef _gtDialog_h
#define _gtDialog_h

// gtDialog.h
//------------------------------------------
// synopsis:
// GT Dialog class descripion
//
//------------------------------------------

#ifndef _gtShell_h
#include <gtShell.h>
#endif

class gtDialog : public gtShell
{
  public:
    ~gtDialog();
    int ChildHasFocus();
    void popup(int modal);
    void popdown();
    void set_default_parent(gtBase *p) { dflt_parent = p; };

  protected:
    gtDialog();
    static gtBase* dflt_parent;
};

#endif // _gtDialog_h

/*
   START-LOG-------------------------------------------

// $Log: gtDialog.h  $
// Revision 1.3 1996/08/26 18:49:18EDT mstarets 
// OpenWin Filter Dialog Bug
 * Revision 1.2.1.3  1993/08/04  15:48:26  swu
 * part of the fix for bug 4119
 *
 * Revision 1.2.1.2  1992/10/09  18:10:00  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:33:17  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:33:16  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:49  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:18  smit
 * Initial revision
 * 
// Revision 1.5  92/09/11  11:11:54  builder
// Make destructor public.
// 
// Revision 1.4  92/09/11  04:07:14  builder
// Add ctor and dtor.
// 
// Revision 1.3  92/09/11  00:36:59  glenn
// popup is no longer virtual, nor does it take a default arg.
// popdown is now in gtShell.
// 
// Revision 1.2  92/08/15  09:05:05  rfermier
// Change inheritance to gtShell
// 
// Revision 1.1  92/08/06  18:18:19  rfermier
// Initial revision
// 
// 
//------------------------------------------

   END-LOG---------------------------------------------
*/
