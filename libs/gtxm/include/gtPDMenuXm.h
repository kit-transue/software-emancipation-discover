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
#ifndef _gtPDMenuXm_h
#define _gtPDMenuXm_h

// gtPDMenuXm.h
//------------------------------------------
// synopsis:
// Class description for the gtPulldownMenu object in OSF Motif
//------------------------------------------

#include <gtPDMenu.h>
#include <gtCallbackMgrXm.h>


class gtPulldownMenuXm : public gtPulldownMenu, public gtCallbackMgrXm
{
  public:
    static Cardinal insert_at_loc(Widget);

    gtPulldownMenuXm(gtBase* parent, const char* name, va_list*);
    ~gtPulldownMenuXm();

    void var_insert(int loc, va_list*);
    void insert_entries(int loc ...);
    void remove_entry(int loc);

    gtPrimitive*     entry(int n);
    gtCascadeButton* cascade(int n);
    gtPrimitive*     button(const char *name);

    void map_callback(gtPulldownCB, void*);

    void move_to(gtEventPtr eve);

    void _filler1_();
    void _filler2_();

  private:
    static Cardinal ins_loc;

    unsigned short    num_entries;
    unsigned short    num_cascades;
    gtPrimitive**     entries;
    gtCascadeButton** cascades;
};


/*
   START-LOG-------------------------------------------

// $Log: gtPDMenuXm.h  $
// Revision 1.1 1993/02/23 23:39:56EST builder 
// made from unix file
 * Revision 1.2.1.4  1993/01/26  05:13:40  glenn
 * Fewer include files.
 * Use gtCallbackMgrXm.
 * Remove unnecessary member variables.
 * Add dtor.
 *
 * Revision 1.2.1.3  1992/11/30  21:15:56  smit
 * Use stdarg not stdargs.
 *
// Revision 1.4  92/08/11  10:30:29  rfermier
// make var_insert public so that gtCascadeButton can call it.
// 
// Revision 1.3  92/07/31  16:39:27  rfermier
// added callbacks
// 
// Revision 1.2  92/07/30  23:55:55  rfermier
// GT method name changes
// 
// Revision 1.1  92/07/08  20:41:03  rfermier
// Initial revision

   END-LOG---------------------------------------------
*/

#endif // _gtPDMenuXm_h
