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
#ifndef _gtScrollbarXm_h
#define _gtScrollbarXm_h

// gtScrollbarXm.h
//------------------------------------------
// synopsis:
// Class description for the gtLabel object in OSF Motif
//------------------------------------------

#include <gtScrollbar.h>
#include <gtCallbackMgrXm.h>


class gtScrollbarXm : public gtScrollbar, public gtCallbackMgrXm
{
  public:
    gtScrollbarXm(gtBase* parent, const char* name, 
		  gtScrollCB callback, void* client_data, gtDirection dir);
    ~gtScrollbarXm();

    void set_limits(int min, int max);
    int  value();
    void value(int v);
    void slider_size(int s);
    void arrows(int flag);

    void add_CB(gtScrollCB, void* client_data, gtReason);
};


/*
   START-LOG-------------------------------------------

   $Log: gtScrollbarXm.h  $
   Revision 1.1 1993/07/29 10:37:55EDT builder 
   made from unix file
 * Revision 1.2.1.4  1993/01/26  05:13:40  glenn
 * Fewer include files.
 * Use gtCallbackMgrXm.
 * Remove unnecessary member variables.
 * Add dtor.
 *
 * Revision 1.2.1.3  1992/10/17  17:47:43  wmm
 * Fix typo in #include line that prevented importation into aset.
 *
 * Revision 1.2.1.2  92/10/09  18:44:02  jon
 * RCS History Marker Fixup
 * 
   END-LOG---------------------------------------------
*/

#endif // _gtScrollbarXm_h
