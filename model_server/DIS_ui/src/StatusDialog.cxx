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
// StatusDialog.h.C
//------------------------------------------
// synopsis:
//
// Dialog box to display the status of one function
// which supply the cancel button and
// show the completion rate in a slider bar
//
//------------------------------------------

// INCLUDE FILES

#ifndef ISO_CPP_HEADERS
#include <stdio.h>
#else /* ISO_CPP_HEADERS */
#include <cstdio>
namespace std {};
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <cLibraryFunctions.h>
#ifndef _genError_h
#include <genError.h>
#endif

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
#endif /* ISO_CPP_HEADERS */
#include <genString.h>
#include "../../DIS_ui/interface.h"
#undef printf
extern Application* DISui_app;

#ifndef _StatusDialog_h
#include <StatusDialog.h>
#endif

#define TXT(a) a

StatusDialog *StatusDialog::dialogs;

StatusDialog::StatusDialog()
// create the status dialog box
{
    Initialize(StatusDialog::StatusDialog);

    genString command;
    command.printf ("dis_launch_dialog statusDialog");
    _vstrptr dialogIdStr = rcall_dis_DISui_eval (DISui_app, (vstr *)command.str());
    dialogId = atoi ((char *) dialogIdStr);
    next = dialogs;
    dialogs = this;
}

StatusDialog::~StatusDialog()
// destructor
{
    Initialize(StatusDialog::~StatusDialog);
    popdown();
}

StatusDialog *StatusDialog::FindDialog (int id) 
{
    Initialize(StatusDialog::FindDialog);
    for (StatusDialog *d = dialogs; d; d = d->next) {
        if (d->dialogId == id) return d;
    }

    return NULL;
}

void StatusDialog::popdown()
// make the status dialog box disappear
{
    Initialize(StatusDialog::popdown);

    genString command;
    command.printf ("dis_gdDialogIDEval {%d} {puts [eval destroyDialog]}", dialogId);
    rcall_dis_DISui_eval_async (DISui_app, (vstr *)command.str());
}

void StatusDialog::ok_cb ()
{
    ok = 1;
}

void StatusDialog::cancel_cb () 
{
    ok = -1;
}

int StatusDialog::popup(char *header_text, char *label_text, int items)
// show the dialog box
// header_text: is the title of this dialog box
// label_text: is the label displayed in this dialog box
// items is the max number for the slider (thermometer)
//   The next routine is_cancelled passed back the number of completed items.
//   The ratio of this number and "items" will be shown as the length of the
//   slider
{
    Initialize(StatusDialog::popup);
    genString command;
    command.printf ("dis_gdDialogIDEval {%d} {loadDialog {%s} {%s} {%d};set returnValue [dis_prompt {YesNoDialog} {%s} {%s}]; if {$returnValue == \"Yes\"} {return 1} else {return 0}}", 
                    dialogId, header_text, "", items, header_text, label_text);
    vchar *returnValue = rcall_dis_DISui_eval (DISui_app, (vstr *)command.str());
    return atoi ((char *)returnValue);

    return ok == 1 ? 1 : 0;
}

int StatusDialog::is_cancelled(int value, char *text)
// the value will be used to adjust the thermometer display
//     (see the previous routine popup)
// text will be shown as the label in the dialog box
// if the cancel button is pressed it will return 1.
// otherwise, it will return 0.
{
	const char* result;
    Initialize(StatusDialog::is_cancelled);

    genString command;
    command.printf ("dis_gdDialogIDEval {%d} {updateDialog {%s} {%d}}", 
                    dialogId, text, value);
    result = (const char *)rcall_dis_DISui_eval(DISui_app, (vstr *)command.str());
	return atoi(result);
    //return 0;
}

#if 0 //_WIN32

//
// get_closure_state() from projectBrowser1.h.C
//

static bool closure = false;

bool get_closure_state() {
   return closure;
}

#endif

/*
$Log: StatusDialog.cxx  $
Revision 1.5 2000/07/10 22:50:35EDT ktrans 
mainline merge from Visual C++ 6/ISO (extensionless) standard header files
*/
