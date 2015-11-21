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
#ifndef _gtFormXm_h
#define _gtFormXm_h

// gtFormXm.h
//------------------------------------------
// synopsis:
// Class definitions and headers for XMotif implentation of gtForm
//------------------------------------------

#include <gtForm.h>
#include <gtCallbackMgrXm.h>


class gtFormXm : public gtForm, public gtCallbackMgrXm
{
  public:
    gtFormXm(gtBase* parent, const char* name);
    gtFormXm(Widget parent, const char* name);
    ~gtFormXm();

    void help_callback(gtFormCB callback, void* client_data);
};


/*
   START-LOG-------------------------------------------

// $Log: gtFormXm.h  $
// Revision 1.2 1995/03/27 18:04:42EST azaparov 
// Bug track: N/A
// Minibrowser stuff & fixes around
 * Revision 1.2.1.3  1993/01/26  05:13:40  glenn
 * Fewer include files.
 * Use gtCallbackMgrXm.
 * Remove unnecessary member variables.
 * Add dtor.
 *
// Revision 1.3  92/07/30  23:55:49  rfermier
// GT method name changes
// 
// Revision 1.2  92/07/13  15:22:16  rfermier
// Added help callback hooks & mechanism
// 
// Revision 1.1  92/07/08  20:40:56  rfermier
// Initial revision

   END-LOG---------------------------------------------
*/

#endif // _gtFormXm_h
