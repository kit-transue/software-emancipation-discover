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
#ifndef _JournalDlg_h
#define _JournalDlg_h

// JournalDlg.h
//------------------------------------------
// synopsis:
// 
// A simple dialog box to prompt for a line of input
//
//------------------------------------------

#ifndef _gt_h
#include <gt.h>
#endif

#ifndef _genString_h
#include <genString.h>
#endif

#include <gtOptionMenu.h>

class JournalDlg
{
  public:
    JournalDlg(gtBase* parent);
    ~JournalDlg();

    int oneshot(genString& answer, genString& bug, genString& group,
		genString& pdf_name, genString& project_name);

    int ask(genString& answer, genString& bug, genString& group,
	    genString& pdf_name, genString& project_name) 
    { return oneshot(answer, bug, group, pdf_name, project_name); }

  private:
    static void OK_CB (gtPushButton*, gtEventPtr, void*, gtReason);
    static void Cancel_CB (gtPushButton*, gtEventPtr, void*, gtReason);
    static int take_control_CB (void*);
    static void subsystem_CB(void*, gtEventPtr, void*, gtReason);
			    
    gtDialogTemplate*	shell;
    gtLabel*		label;
//    gtLabel*		subsystem_label;
    gtLabel*		bug_number_label;
//    gtLabel*		pdf_used;
    gtLabel*            project_used;
    gtOptionMenu*       subsystem;
    gtStringEditor*	answer_text;
    gtStringEditor*	bug_number_text;
//    gtStringEditor*     pdf_text;
//    gtStringEditor*     project_text;
//    gtStringEditor*     create_date_text;
    int			confirm;

};

typedef JournalDlg* JournalDlg_Ptr;

#endif // _JournalDlg_h


/*
   START-LOG-------------------------------------------

   $Log: JournalDlg.h  $
   Revision 1.1 1994/03/08 11:49:17EST builder 
   made from unix file
// Revision 1.2  1994/03/06  16:37:45  mg
// Bug track: 0
// validation groups
//
// Revision 1.1  1993/09/21  21:58:25  andrea
// Initial revision
// 

  END-LOG---------------------------------------------
*/

