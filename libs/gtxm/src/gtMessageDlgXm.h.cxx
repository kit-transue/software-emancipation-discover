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
// gtMessageDlgXm
//------------------------------------------
// synopsis:
// Implentation of class gtMessageDialogXm
//
//------------------------------------------

// INCLUDE FILES

#include <gtShell.h>
#include <gtPushButton.h>
#include <gtNoParent.h>

#include <gtBaseXm.h>
#include <gtPushButtonXm.h>
#include <gtMessageDlgXm.h>

#include <Xm/Xm.h>
#include <Xm/MessageB.h>

#include <systemMessages.h>
#include <motifString.h>

// FUNCTION DEFINITIONS

gtMessageDialog* gtMessageDialog::create(
    gtBase *parent, const char* name, const char *title, gtMessageType mtype)
{
    return new gtMessageDialogXm(parent, name, title, mtype);
};

extern "C" HelpUI_HelpWidget_CB();

gtMessageDialogXm::gtMessageDialogXm(
    gtBase *parent, const char *name, const char* title, gtMessageType mtype)
{
    if (parent == NULL)
      if (gtDialog::dflt_parent)
	parent = gtDialog::dflt_parent;
      else parent = gtNoParent::app_shell();

    Arg args[1];
    int n = 0;

    motifString xm_title((char*)gettext(title ? title : ""));
    if(title) { XtSetArg(args[0], XmNdialogTitle, (XmString)xm_title); n++; }

    mbox = XmCreateMessageDialog(
	parent->container()->rep()->widget(), (char*)name, args, n);

    r->widget(XtParent(mbox));

    set_type(mtype);

    destroy_init();

    XtAddCallback(mbox, XmNhelpCallback, (XtCallbackProc)HelpUI_HelpWidget_CB, NULL);
    if(gtPushButtonXm::saved_help_context)
        {
	    help_context = gtPushButtonXm::saved_help_context;
            set_widget_help_context(mbox, (char *)help_context);
            gtPushButtonXm::saved_help_context = NULL;
	}
};

void gtMessageDialogXm::set_type(gtMessageType mtype)
{
    int dialog_type;
    switch(mtype)
    {
      case gtMsgError:		dialog_type = XmDIALOG_ERROR;       break;
      case gtMsgInformation:	dialog_type = XmDIALOG_INFORMATION; break;
      case gtMsgQuestion:	dialog_type = XmDIALOG_QUESTION;    break;
      case gtMsgWarning:	dialog_type = XmDIALOG_WARNING;     break;
      case gtMsgWorking:	dialog_type = XmDIALOG_WORKING;     break;
      case gtMsgPlain:
      default:			dialog_type = XmDIALOG_MESSAGE;     break;
    }
    Arg args[1];
    XtSetArg(args[0], XmNdialogType, dialog_type);
    XtSetValues(mbox, args, 1);
}

gtPushButton *gtMessageDialogXm::help_button()
{
    gtPushButton* pb = gtPushButton::create(NULL, "help", "", NULL, NULL);
    pb->release_rep();
    pb->attach_rep(
	new gtRep(XmMessageBoxGetChild(mbox, XmDIALOG_HELP_BUTTON)));

    return pb;
}

gtPushButton *gtMessageDialogXm::cancel_button()
{
    gtPushButton* pb = gtPushButton::create(NULL, "cancel", "", NULL, NULL);
    pb->release_rep();
    pb->attach_rep(
	new gtRep(XmMessageBoxGetChild(mbox, XmDIALOG_CANCEL_BUTTON)));

    return pb;
}

gtPushButton *gtMessageDialogXm::ok_button()
{
    gtPushButton* pb = gtPushButton::create(NULL, "ok", "", NULL, NULL);
    pb->release_rep();
    pb->attach_rep(
	new gtRep(XmMessageBoxGetChild(mbox, XmDIALOG_OK_BUTTON)));

    return pb;
}

void gtMessageDialogXm::message(const char *t)
{
    XmString str =
	XmStringCreateLtoR((char*)gettext(t), XmSTRING_DEFAULT_CHARSET);

    Arg	args[1];
    XtSetArg(args[0], XmNmessageString, str);

    XtSetValues(mbox, args, 1);

    XmStringFree(str);
}

/*
   START-LOG-------------------------------------------

   $Log: gtMessageDlgXm.h.C  $
   Revision 1.2 1995/01/31 10:41:53EST azaparov 
   Bug track: 9233
   Fixed bug 9233
Revision 1.2.1.4  1993/08/04  15:48:59  swu
part of the fix for bug 4111

Revision 1.2.1.3  1993/04/28  12:28:17  jon
Set help context from gtPushButtonXm::saved_help_context

Revision 1.2.1.2  1992/10/09  19:03:36  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
