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
using namespace std;
#endif /* ISO_CPP_HEADERS */

#ifndef _genError_h
#include <genError.h>
#endif

#ifndef _systemMessages_h
#include <systemMessages.h>
#endif

#include <x11_intrinsic.h>

#ifndef _ParaCancel_h
#include <ParaCancel.h>
#endif

#ifndef _StatusDialog_h
#include <StatusDialog.h>
#endif

StatusDialog::StatusDialog()
// create the status dialog box
{
    Initialize(StatusDialog::StatusDialog);

    shell = gtDialogTemplate::create(NULL, "Progress indicator",
                                     TXT(""));
    shell->add_button("okbutt", TXT("OK"), &StatusDialog::OK_CB, this);
    shell->add_button("canbutt", TXT("Cancel"), &StatusDialog::Cancel_CB, this);
//    shell->add_button("helpbutt", TXT("Help"), NULL, NULL);
    shell->default_button(NULL);

    gtForm *form = gtForm::create(shell, "form");
    form->attach(gtTop);
    form->attach(gtBottom);
    form->attach(gtLeft);
    form->attach(gtRight);
    form->manage();

    label = gtLabel::create(form, "shell_label", TXT("Filter"));
    label->attach(gtLeft);
    label->attach(gtTop);
    label->attach(gtRight);
    label->manage();

    gtLabel *completed_label = gtLabel::create(form, "completed_label", TXT("Completed:"));
    completed_label->attach(gtTop, label, 10);
    completed_label->attach(gtLeft);
    completed_label->manage();

    complete_count = gtStringEditor::create(form, "complete_count", NULL);
    complete_count->attach (gtTop, label);
    complete_count->attach (gtLeft, completed_label);
    complete_count->columns(10);
    complete_count->manage();
    complete_count->text("0");

    bar = gtScrollbar::create(form, "scroll_bar", NULL, NULL, gtHORZ);
    bar->attach(gtLeft, NULL, 10);
    bar->attach(gtRight, NULL, 10);
    bar->attach(gtTop, complete_count, 10);
    bar->arrows(0);
    bar->manage();
}

StatusDialog::~StatusDialog()
// destructor
{
    Initialize(StatusDialog::~StatusDialog);

    popdown();
}

void StatusDialog::popdown()
// make the status dialog box disappear
{
    Initialize(StatusDialog::popdown);

    if (ParaCancel::canceller)
    {
        delete ParaCancel::canceller;
        ParaCancel::canceller = NULL;
    }
    shell->popdown();
}

void StatusDialog::reestablish_canceller() {
   if (!ParaCancel::canceller) {
      new ParaCancel("Cancel", 0, 0,
		     (int) XtWindow(shell->button("canbutt")->rep()->widget()));
   }
}

void StatusDialog::OK_CB(gtPushButton *, gtEvent *, void *cd, gtReason)
// the call back routine for the ok button in the status dialog box
{
    Initialize(StatusDialog::OK_CB);

    StatusDialog *sd = (StatusDialog *)cd;
    if (!ParaCancel::canceller)
        new ParaCancel("Cancel", 0, 0,
                       (int)XtWindow(sd->shell->button("canbutt")->rep()->widget()));
    sd->shell->button("okbutt")->set_sensitive(0);
    sd->shell->button("canbutt")->set_sensitive(0);
//    sd->shell->button("canbutt")->set_sensitive(1);
    sd->shell->flush();
    sd->shell->button("okbutt")->update_display();
    sd->shell->button("canbutt")->update_display();
    sd->ok = 1;

    return;
}

void StatusDialog::Cancel_CB(gtPushButton *, gtEvent *, void *cd, gtReason)
// the call back routine for the cancel button in the status dialog box
{
    Initialize(StatusDialog::Cancel_CB);

    StatusDialog *sd = (StatusDialog *)cd;
    sd->shell->popdown();
    sd->ok = -1;
    return;
}

int StatusDialog::popup(char *header_text, char *label_text, int items)
{
  return popup(header_text, label_text, items, false);
}

int StatusDialog::popup(char *header_text, char *label_text, int items, bool nowait)
// show the dialog box
// header_text: is the title of this dialog box
// label_text: is the label displayed in this dialog box
// items is the max number for the slider (thermometer)
//   The next routine is_cancelled passed back the number of completed items.
//   The ratio of this number and "items" will be shown as the length of the
//   slider
{
    Initialize(StatusDialog::popup);

    msg = label_text;
    last_count = 1;
    total_count = items;
    shell->title(header_text);
    label->text(label_text);
    bar->value(1);
    bar->slider_size(1);
    bar->set_limits(1, items + 1);
    shell->popup(1);
    if(nowait){
      OK_CB(NULL, NULL, this, (gtReason)NULL);
      ok = 1;
    } else {
      ok = 0;
      shell->take_control(&check_ok, this);
//    while(ok == 0)
//        XtAppProcessEvent(gtApplicationShellXm::get_context(), XtIMAll);
    }
    return ok == 1 ? 1 : 0;
}

int StatusDialog::check_ok(void *data)
{
    Initialize(StatusDialog::check_ok);

    return ((StatusDialog *)data)->ok;
}

int StatusDialog::is_cancelled(int value, char *text)
// the value will be used to adjust the thermometer display
//     (see the previous routine popup)
// text will be shown as the label in the dialog box
// if the cancel button is pressed it will return 1.
// otherwise, it will return 0.
{
    Initialize(StatusDialog::is_cancelled);

    char buf[12];

    sprintf(buf,"%d",value);
    complete_count->text(buf);
    complete_count->flush();

    if (text && strcmp(msg.str(), text))
    {
        msg = text;
        label->text(text);
        label->flush();
        bar->update_display();
    }
    shell->flush();
    if (value > 0 && (value < last_count || (value - last_count) * 200 > total_count))
    {
        last_count = value;
        bar->slider_size(value);
        bar->flush();
        bar->update_display();
    }
    if (ParaCancel::is_cancelled())
    {
        delete ParaCancel::canceller;
        ParaCancel::canceller = NULL;
        shell->popdown();
        return 1;
    }
    else
        return 0;
}
