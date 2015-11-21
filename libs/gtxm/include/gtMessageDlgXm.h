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
#ifndef _gtMessageDlgXm_h
#define _gtMessageDlgXm_h

// gtMessageDlgXm.h
//------------------------------------------
// synopsis:
// Class definitions and headers for XMotif implentation of gtMessageDialog
//------------------------------------------

#include <gtMessageDlg.h>
#include <genString.h>


class gtMessageDialogXm : public gtMessageDialog
{
  public:
    gtMessageDialogXm(gtBase*, const char*, const char*, gtMessageType);

    void set_type(gtMessageType mtype);
    gtPushButton* help_button();
    gtPushButton* cancel_button();
    gtPushButton* ok_button();
    void message(const char*);

  private:
    Widget    mbox;
    genString help_context;
};


/*
   START-LOG-------------------------------------------

// $Log: gtMessageDlgXm.h  $
// Revision 1.2 1995/01/31 10:42:17EST azaparov 
// Bug track: 9233
// Fixed bug 9233
 * Revision 1.2.1.3  1993/01/26  05:13:40  glenn
 * Fewer include files.
 * Use gtCallbackMgrXm.
 * Remove unnecessary member variables.
 * Add dtor.
 *
// Revision 1.3  92/09/14  12:14:36  glenn
// Remove unused member variables, popup, popdown.
// 
// Revision 1.2  92/07/30  23:55:54  rfermier
// GT method name changes
// 
// Revision 1.1  92/07/17  13:40:38  rfermier
// Initial revision
// 
// Revision 1.3  92/07/09  18:01:48  rfermier
// moved resolution-independant code to gtBase
// 
// Revision 1.2  92/07/09  11:05:11  rfermier
// add screen resolution
// 
// Revision 1.1  92/07/08  20:40:52  rfermier
// Initial revision
// 
   END-LOG---------------------------------------------
*/

#endif // _gtMessageDlgXm_h
