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
//  StatusDialog.h
//    Define class  StatusDialog

#ifndef _StatusDialog_h
#define _StatusDialog_h

#ifndef NEW_UI

#include <gtTopShell.h>
#include <gtBaseXm.h>
#include <gtAppShellXm.h>
#include <gtScrollbar.h>
#include <gtDlgTemplate.h>
#include <gtPushButton.h>
#include <gtStringEd.h>
#include <gtLabel.h>
#endif // NEW_UI

#include <genString.h>

class StatusDialog {
  public:
    StatusDialog();
    virtual ~StatusDialog();

    int  popup(char *header_text, char *label_text, int items);
    int  popup(char *header_text, char *label_text, int items, bool nowait);
    void popdown();
    int  is_cancelled(int current_value, char *text);
    void reestablish_canceller();

#ifndef NEW_UI
  private:
    static void OK_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static void Cancel_CB(gtPushButton *, gtEvent *, void *cd, gtReason);
    static int  check_ok(void *data);
#else
  public:
    static StatusDialog* FindDialog (int id);
    void ok_cb ();
    void cancel_cb ();
#endif
  private:
    genString        msg;
    int              total_count;
    int              last_count;

#ifndef NEW_UI
    gtStringEditor   *complete_count;
    gtLabel          *label;
    gtDialogTemplate *shell;
    gtScrollbar      *bar;
#else
    static StatusDialog *dialogs;
    StatusDialog *next;
    int dialogId;
#endif

    int              ok;
};

#endif

