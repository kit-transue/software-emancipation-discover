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
#ifndef _gtCascadeBXm_h
#define _gtCascadeBXm_h

// gtCascadeBXm.h
//------------------------------------------
// synopsis:
// Class description for the gtCascadeButton object in OSF Motif
//------------------------------------------

#include <gtCascadeB.h>
#include <gtCallbackMgrXm.h>


class gtPulldownMenuXm;

class gtCascadeButtonXm : public gtCascadeButton, public gtCallbackMgrXm
{
  public:
    gtCascadeButtonXm(gtBase *parent, const char *name, const char *contents,
		      gtCascadeCB callback, void *client_data);
    ~gtCascadeButtonXm();

    void sub_menu(gtPrimitive*);
    void label(const char*);
    void pulldown_menu(const char*name ...);
    void insert_entries(int loc ...);
    void remove_entry(int loc);
    gtPrimitive* entry(int n);
    gtCascadeButton* cascade(int n);
    gtPrimitive* button(const char *c);

  private:
    gtBase* parent;
    gtPulldownMenuXm* sub_m;
};


/*
   START-LOG-------------------------------------------

// $Log: gtCascadeBXm.h  $
// Revision 1.1 1993/07/29 10:37:47EDT builder 
// made from unix file
 * Revision 1.2.1.3  1993/01/26  05:13:40  glenn
 * Fewer include files.
 * Use gtCallbackMgrXm.
 * Remove unnecessary member variables.
 *
// Revision 1.6  92/08/11  10:29:44  rfermier
// Better access to associated pulldown, added pulldown_menu to replace
// gtPulldownMenu Class
// 
// Revision 1.5  92/08/10  10:26:03  rfermier
// *** empty log message ***
// 
// Revision 1.4  92/08/10  09:52:15  rfermier
//  added pulldown_menu function
// 
// Revision 1.3  92/07/30  23:55:46  rfermier
// GT method name changes
// 
// Revision 1.2  92/07/29  18:41:09  rfermier
// added label(const char *)
// 
// Revision 1.1  92/07/08  20:40:51  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtCascadeBXm_h
