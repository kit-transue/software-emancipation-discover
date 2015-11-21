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
#ifndef _gtToggleBoxXm_h
#define _gtToggleBoxXm_h

// gtToggleBoxXm.h
//------------------------------------------
// synopsis:
// Class description for the gtToggleBox object in OSF Motif
//------------------------------------------

#include <gtToggleBox.h>


class gtToggleBoxXm : public gtToggleBox
{
  public:
    gtToggleBoxXm(gtBase *parent, const char *name, const char *title, va_list *pap);
    ~gtToggleBoxXm();

    int  item_set(int num);
    void item_set(int num, int value, int flag);
    void num_columns(int n);

    gtToggleButton* button(const char*);

    gtBase* container();

  private:
    int num_buttons;
    gtToggleButton** buttons;
    gtVertBox* button_group;
};


/*
   START-LOG-------------------------------------------

   $Log: gtToggleBoxXm.h  $
   Revision 1.1 1993/02/24 00:00:46EST builder 
   made from unix file
 * Revision 1.2.1.3  1993/01/26  05:13:40  glenn
 * Fewer include files.
 * Use gtCallbackMgrXm.
 * Remove unnecessary member variables.
 * Add dtor.
 *
 * Revision 1.2.1.2  1992/10/09  18:44:33  jon
 * RCS History Marker Fixup
 *
   END-LOG---------------------------------------------
*/

#endif // _gtToggleBoxXm_h
