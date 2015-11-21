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
#ifndef _gtScrollWin_h
#define _gtScrollWin_h

// gtScrollWin.h
//------------------------------------------
// synopsis:
// generic ScrolledWindow manager (gtScrolledWindow)
//
//------------------------------------------

#ifndef _gtManager_h
#include <gtManager.h>
#endif

class gtScrolledWindow : public gtManager
{
  public:
    // Creates a ScrolledWindow.
    static gtScrolledWindow* create(gtBase *parent, const char *name);

    static gtScrolledWindow* create(
	gtBase *parent, const char *name, int height, int width);

    ~gtScrolledWindow();

    virtual gtScrollbar* horizBar() = 0;
    virtual gtScrollbar* vertBar() = 0;
    virtual void set_horz_bar(gtScrollbar *sb) = 0;
    virtual void set_vert_bar(gtScrollbar *sb) = 0;
    virtual void set_work_area(gtPrimitive *wa) = 0;
    virtual void scrollbar_pos(gtCardinalDir, gtCardinalDir) = 0;

  protected:
    gtScrolledWindow();
};

#endif // _gtScrollWin_h

/*
   START-LOG-------------------------------------------

// $Log: gtScrollWin.h  $
// Revision 1.1 1993/07/28 19:47:51EDT builder 
// made from unix file
 * Revision 1.2.1.2  1992/10/09  18:10:33  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:34:40  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:34:39  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:58  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:24  smit
 * Initial revision
 * 
// Revision 1.5  92/09/11  11:12:28  builder
// Make destructor public.
// 
// Revision 1.4  92/09/11  04:07:44  builder
// Add ctor and dtor.
// 
// Revision 1.3  92/08/18  12:24:09  rfermier
// added scrollbar_pos
// 
// Revision 1.2  92/07/30  23:44:59  glenn
// GT method name changes
// 
// Revision 1.1  92/07/08  20:39:15  rfermier
// Initial revision
// 
//------------------------------------------

   END-LOG---------------------------------------------
*/
