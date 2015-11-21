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
#ifndef _gtOptionMenuXm_h
#define _gtOptionMenuXm_h

// gtOptionMenuXm.h
//------------------------------------------
// synopsis:
// Class description for the gtOptionMenu object in OSF Motif
//------------------------------------------

#include <gtOptionMenu.h>
#include <gtCallbackMgrXm.h>


class gtOptionMenuXm : public gtOptionMenu, public gtCallbackMgrXm
{
  public:
    gtOptionMenuXm(
	gtBase* parent, const char* name, const char* title, va_list*);
    ~gtOptionMenuXm();

    void move_to(gtEventPtr eve);
    void insert_entries(int loc ...);
    void remove_entry(int loc);
    int  total_entries();

    gtPrimitive*     entry(int n);
    gtPrimitive*     button(const char *);
    gtCascadeButton* cascade(int n);
    gtLabel*         label();
    gtPulldownMenu*  sub_menu();

    void  menu_history(int index);
    void  menu_history(const char *);
    char* menu_history();

    void map_callback(gtOptionCB, void *);

    gtBase* container();
    void _filler_();

    void font (const char*);

  private:
    static Cardinal ins_loc;
    static Cardinal insert_at_loc(Widget);

    unsigned short    num_entries;
    unsigned short    num_cascades;
    gtPrimitive**     entries;
    gtCascadeButton** cascades;
    gtLabel*          gt_casc;
    gtPulldownMenu*   pulldown_menu;

    void var_insert(int loc, va_list *pap);
};


/*
   START-LOG-------------------------------------------

// $Log: gtOptionMenuXm.h  $
// Revision 1.2 1995/05/17 14:05:05EDT jerry 
// Bug fixes, phase II of new links.
 * Revision 1.2.1.4  1993/01/26  05:13:40  glenn
 * Fewer include files.
 * Use gtCallbackMgrXm.
 * Remove unnecessary member variables.
 * Add dtor.
 *
 * Revision 1.2.1.3  1992/11/30  21:13:46  smit
 * Use stdarg not stdargs.
 *
// Revision 1.4  92/07/31  16:40:05  rfermier
// added callbacks, expanded menu_history functions
// 
// Revision 1.3  92/07/30  23:55:54  rfermier
// GT method name changes
// 
// Revision 1.2  92/07/22  13:39:50  rfermier
// added named function
// 
// Revision 1.1  92/07/08  20:41:02  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtOptionMenuXm_h
