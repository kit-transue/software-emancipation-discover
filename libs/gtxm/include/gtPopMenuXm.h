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
#ifndef _gtPopMenuXm_h
#define _gtPopMenuXm_h

// gtPopMenuXm.h
//------------------------------------------
// synopsis:
// Class description for the gtPopupMenu object in OSF Motif
//------------------------------------------

#include <gtPopMenu.h>
#include <gtCallbackMgrXm.h>


class gtPopupMenuXm : public gtPopupMenu, public gtCallbackMgrXm
{
  public:
    gtPopupMenuXm(gtBase* parent, const char* name, va_list&);
    ~gtPopupMenuXm();

    void move_to(gtEventPtr eve);
    void insert_entries(int loc ...);
    void remove_entry(int loc);
    gtPrimitive* entry(int n);
    void map_callback(gtPopupCB, void*);

  private:
    static Cardinal ins_loc;
    static Cardinal insert_at_loc(Widget);

    int	num_entries;
    gtPrimitive **entries;

    void var_insert(int loc, va_list&);
};


/*
   START-LOG-------------------------------------------

// $Log: gtPopMenuXm.h  $
// Revision 1.1 1993/07/29 10:37:52EDT builder 
// made from unix file
 * Revision 1.2.1.4  1993/01/26  05:13:40  glenn
 * Fewer include files.
 * Use gtCallbackMgrXm.
 * Remove unnecessary member variables.
 * Add dtor.
 *
 * Revision 1.2.1.3  1992/11/30  21:16:35  smit
 * Use stdarg not stdargs.
 *
 * Revision 1.2.1.2  1992/10/09  18:43:56  jon
 * RCS History Marker Fixup
 *
 * Revision 1.2.1.1  92/10/07  22:10:48  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  22:10:47  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:27:42  smit
 * Initial revision
 * 
// Revision 1.4  92/09/24  21:38:01  glenn
// Remove many unused member functions and variables.
// 
// Revision 1.3  92/07/31  16:38:59  rfermier
// added callbacks
// 
// Revision 1.2  92/07/30  23:55:57  rfermier
// GT method name changes
// 
// Revision 1.1  92/07/08  20:41:05  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtPopMenuXm_h
