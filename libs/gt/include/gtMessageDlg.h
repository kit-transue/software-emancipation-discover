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
#ifndef _gtMessageDlg_h
#define _gtMessageDlg_h

// gtMessageDlg.h
//------------------------------------------
// synopsis:
// GT Dialog Shell class definitions
//
//------------------------------------------

#ifndef _gtDialog_h
#include <gtDialog.h>
#endif

class gtMessageDialog : public gtDialog
{
  public:
    // Creates a Dialog Shell specific to the toolkit.
    static gtMessageDialog* create(
	gtBase* parent, const char* name, const char* title,
	gtMessageType mtype = gtMsgPlain);

    ~gtMessageDialog();

    // specify the type of the message box
    virtual void set_type(gtMessageType mtype) = 0;

    // retrieve one of the buttons
    virtual gtPushButton* help_button() = 0;
    virtual gtPushButton* cancel_button() = 0;
    virtual gtPushButton* ok_button() = 0;

    // set the text of the message ('\n' => line-break)
    virtual void message(const char*) = 0;

  protected:
    gtMessageDialog();
};

#endif // _gtMessageDlg_h

/*
   START-LOG-------------------------------------------

// $Log: gtMessageDlg.h  $
// Revision 1.1 1993/07/28 19:47:45EDT builder 
// made from unix file
 * Revision 1.2.1.2  1992/10/09  18:10:20  jon
 * RCS history fixup
 *
 * Revision 1.2.1.1  92/10/07  20:34:02  smit
 * *** empty log message ***
 * 
 * Revision 1.2  92/10/07  20:34:01  smit
 * *** empty log message ***
 * 
 * Revision 1.1  92/10/07  18:19:53  smit
 * Initial revision
 * 
 * Revision 1.1  92/10/07  17:55:22  smit
 * Initial revision
 * 
// Revision 1.8  92/09/14  12:13:50  glenn
// Remove popup, popdown.
// 
// Revision 1.7  92/09/11  11:12:13  builder
// Make destructor public.
// 
// Revision 1.6  92/09/11  08:17:53  wmm
// Fix compilation errors due to incorrect protected/private access.
// 
// Revision 1.5  92/09/11  04:07:31  builder
// Add ctor and dtor.
// 
// Revision 1.4  92/08/15  09:09:17  rfermier
// Last comment should have been:
// Changed inheritance to subclass of gtDialog.
// 
// Revision 1.3  92/08/15  09:06:51  rfermier
// minor cleanup
// 
// Revision 1.2  92/07/30  23:44:47  glenn
// GT method name changes
// 
// Revision 1.1  92/07/17  13:40:42  rfermier
// Initial revision
// 
// Revision 1.3  92/07/09  18:02:10  rfermier
// moved resolution-independant code to gtBase
// 
// Revision 1.2  92/07/09  11:05:31  rfermier
// added screen resolution
// 
// Revision 1.1  92/07/08  20:38:52  rfermier
// Initial revision
// 

   END-LOG---------------------------------------------
*/
