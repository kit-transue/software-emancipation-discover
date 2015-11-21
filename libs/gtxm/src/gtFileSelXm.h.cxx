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
// gtFileSelXm.h.C
//------------------------------------------
// synopsis:
// Member function source code for gtFileSelector class in OSF Motif
//
//------------------------------------------

// INCLUDE FILES

#include <gtFileSelXm.h>
#include <Xm/FileSB.h>
#include <gtNoParent.h>
#include <gtPushButton.h>
#include <gtLabel.h>
#include <gtStringEd.h>
#include <gtList.h>

extern "C" HelpUI_HelpWidget_CB();


// FUNCTION DEFINITIONS

gtFileSelector* gtFileSelector::create(gtBase *parent, const char* name)
{
    return new gtFileSelectorXm(parent, name);
}

gtFileSelector *gtFileSelector::create(
    gtBase *parent, const char *name,
    gtPushCB OK_CB, void *OK_data,
    gtPushCB Cancel_CB, void *Cancel_data)
{
    gtFileSelector* fs = new gtFileSelectorXm(parent, name);
    fs->ok_button()->set_callback(OK_CB, OK_data);
    fs->cancel_button()->set_callback(Cancel_CB, Cancel_data);

    return fs;
}

gtFileSelectorXm::gtFileSelectorXm(gtBase *parent_arg, const char *name_arg)
{
    parent = parent_arg;
    name = name_arg;
    construct();
}

void gtFileSelectorXm::construct()
{
    if (!parent)
	parent = gtNoParent::app_shell();
    r->widget(XmCreateFileSelectionDialog(
	parent->container()->rep()->widget(), (char*)name, NULL, 0));
    destroy_init();

    XtAddCallback(r->widget(), XmNhelpCallback,
		  XtCallbackProc(HelpUI_HelpWidget_CB), NULL);
}    

gtBase* gtFileSelectorXm::container()
{
    return NULL;
}

char *gtFileSelectorXm::selected()
//
// Caller is responsible for calling gtFree() on result.
//
{
    Arg args[1];
    XmString sel;

    XtSetArg(args[0], XmNdirSpec, &sel);
    XtGetValues(r->widget(), args, 1);

    char* ans = NULL;
    XmStringGetLtoR(sel, XmSTRING_DEFAULT_CHARSET, &ans);

    return ans;
}

gtPushButton *gtFileSelectorXm::help_button()
{
    gtPushButton* pb = gtPushButton::create(NULL, "help", "", NULL, NULL);
    pb->release_rep();
    pb->attach_rep(
	new gtRep(XmFileSelectionBoxGetChild(r->widget(),XmDIALOG_HELP_BUTTON)));

    return pb;
}

gtPushButton *gtFileSelectorXm::ok_button()
{
    Widget butt;
    butt = XmFileSelectionBoxGetChild(r->widget(), XmDIALOG_OK_BUTTON);
    gtNoParent *np = gtNoParent::app_shell();
    gtPushButton *pb = gtPushButton::create(np, "pb_ok", "", NULL, NULL);
    pb->rep()->widget(butt);
    return pb;
}

gtPushButton *gtFileSelectorXm::cancel_button()
{
    Widget butt;
    butt = XmFileSelectionBoxGetChild(r->widget(), XmDIALOG_CANCEL_BUTTON);
    gtNoParent *np = gtNoParent::app_shell();
    gtPushButton *pb = gtPushButton::create(np, "pb_cancel", "", NULL, NULL);
    pb->rep()->widget(butt);
    return pb;
}

void gtFileSelectorXm::pattern(const char *p)
{
    Arg	args[1];
    XtSetArg(args[0], XmNpattern, p);
    XtSetValues(r->widget(), args, 1);
}

void gtFileSelectorXm::title(const char *t)
{
    XmString str = XmStringCreateSimple((char *)gettext(t));

    Arg	args[1];
    XtSetArg(args[0], XmNdialogTitle, str);
    XtSetValues(r->widget(), args, 1);

    XmStringFree(str);
}

char *gtFileSelectorXm::title()
//
// Caller is responsible for calling gtFree() on result.
//
{
    Arg args[1];
    XmString sel;
    XtSetArg(args[0], XmNdialogTitle, &sel);
    XtGetValues(r->widget(), args, 1);	

    char* ans;
    XmStringGetLtoR(sel, XmSTRING_DEFAULT_CHARSET, &ans);

    return ans;
}
    
void gtFileSelectorXm::select_label(const char *l)
{
    XmString str = XmStringCreateSimple((char *)l);

    Arg	args[1];
    XtSetArg(args[0], XmNselectionLabelString, str);
    XtSetValues(r->widget(), args, 1);

    XmStringFree(str);
}

gtLabel *gtFileSelectorXm::select_label()
{
    Widget butt =
	XmFileSelectionBoxGetChild(r->widget(), XmDIALOG_SELECTION_LABEL);
    gtNoParent* np = gtNoParent::app_shell();
    gtLabel* pb = gtLabel::create(np, "label_select", "");
    pb->rep()->widget(butt);

    return pb;
}

gtLabel *gtFileSelectorXm::list_label()
{
    Widget butt = XmFileSelectionBoxGetChild(r->widget(), XmDIALOG_LIST_LABEL);
    gtNoParent *np = gtNoParent::app_shell();
    gtLabel *pb = gtLabel::create(np, "label_list", "");
    pb->rep()->widget(butt);

    return pb;
}

gtStringEditor *gtFileSelectorXm::text()
{
    Widget butt = XmFileSelectionBoxGetChild(r->widget(), XmDIALOG_TEXT);
    gtNoParent *np = gtNoParent::app_shell();
    gtStringEditor *t = gtStringEditor::create(np, "text", "");
    t->rep()->widget(butt);

    return t;
}

gtList *gtFileSelectorXm::list()
{
    Widget butt = XmFileSelectionBoxGetChild(r->widget(), XmDIALOG_LIST);
    gtNoParent *np = gtNoParent::app_shell();
    gtList *t = gtList::create(np, "list", NULL, gtSingle, NULL, 0);
    t->rep()->widget(butt);

    return t;
}

gtList *gtFileSelectorXm::dir_list()
{
    Widget butt = XmFileSelectionBoxGetChild(r->widget(), XmDIALOG_DIR_LIST);
    gtNoParent *np = gtNoParent::app_shell();
    gtList *t = gtList::create(np, "list_label", NULL, gtSingle, NULL, 0);
    t->rep()->widget(butt);

    return t;
}

void gtFileSelectorXm::dir_mode(int mode)
{
    Arg	args[1];
    XtSetArg(args[0], XmNfileTypeMask,
	     mode ? XmFILE_DIRECTORY : XmFILE_REGULAR);
    XtSetValues(r->widget(), args, 1);
}

/*
   START-LOG-------------------------------------------

   $Log: gtFileSelXm.h.C  $
   Revision 1.1 1993/04/18 14:51:41EDT builder 
   made from unix file
Revision 1.2.1.3  1993/04/18  18:51:44  glenn
Do not call XmStringFree on XmStrings obtained from XtGetValues.
Add comments about callers responsibility to call gtFree on results
of some calls.

Revision 1.2.1.2  1992/10/09  19:03:30  jon
RCS History Marker Fixup



   END-LOG---------------------------------------------

*/
