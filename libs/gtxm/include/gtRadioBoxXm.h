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
#ifndef _gtRadioBoxXm_h
#define _gtRadioBoxXm_h

// gtRadioBoxXm.h
//------------------------------------------
// synopsis:
// Class description for the gtRadioBox object in OSF Motif
//------------------------------------------

#include <gtRadioBox.h>


class gtRadioBoxXm : public gtRadioBox
{
  public:
    gtRadioBoxXm(
	gtBase* parent, const char* name, const char* title, va_list*);
    ~gtRadioBoxXm();

    void num_columns(int);

    int  item_set();
    void item_set(int num, int value, int flag);

    gtToggleButton* entries(int n) { return buttons[n]; };
    gtToggleButton* button(const char *nm);

    gtBase* container();

  private:
    gtNoParent* xm_radio_box;
    int	num_buttons;
    gtToggleButton** buttons;
};


/*
   START-LOG-------------------------------------------

// $Log: gtRadioBoxXm.h  $
// Revision 1.1 1993/07/29 10:37:53EDT builder 
// made from unix file
 * Revision 1.2.1.3  1993/01/26  05:13:40  glenn
 * Fewer include files.
 * Use gtCallbackMgrXm.
 * Remove unnecessary member variables.
 * Add dtor.
 *
// Revision 1.3  92/07/30  23:55:59  rfermier
// GT method name changes
// 
// Revision 1.2  92/07/09  12:53:39  rfermier
// Added multiple column support
// 
// Revision 1.1  92/07/08  20:41:09  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtRadioBoxXm_h
