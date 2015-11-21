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
// gtMainWindowXm
//------------------------------------------
// synopsis:
// Implentation of member functions for the Form class (gtMainWindowXm)
//------------------------------------------

// INCLUDE FILES

#include <x11_intrinsic.h>
#include <Xm/MainW.h>

#include <gtNoParent.h>
#include <gtMenuBar.h>
#include <gtForm.h>
#include <gtLabel.h>
#include <gtBaseXm.h>
#include <gtMainWindowXm.h>

// FUNCTION DEFINITIONS

gtMainWindow* gtMainWindow::create(
    gtBase* parent, const char* name, int make_message_area)
{
    return new gtMainWindowXm(parent, name, make_message_area);
}


gtMainWindowXm::gtMainWindowXm(
    gtBase* parent, const char* name, int make_message_area)
: menubar(NULL), work_area(NULL), message_area(NULL)
{
    if (!parent)
	parent = gtNoParent::app_shell();

    r->widget(XmCreateMainWindow(parent->container()->rep()->widget(),
				 (char*)gettext(name), NULL, 0));
    destroy_init();

    Arg arg[3];
    int n = 0;

    menubar = gtMenuBar::create(this, "menubar");
    menubar->manage();

    XtSetArg(arg[n], XmNmenuBar, menubar->rep()->widget()); n++;

    gtForm* form = gtForm::create(this, "work_area");
    form->manage();

    XtSetArg(arg[n], XmNworkWindow, form->rep()->widget()); n++;

    if(make_message_area)
    {
	message_area = gtLabel::create(this, "message_area", " ");
	message_area->alignment(gtBeginning);
	message_area->manage();

	XtSetArg(arg[n], XmNmessageWindow, message_area->rep()->widget()); n++;
    }

    XtSetValues(r->widget(), arg, n);

    work_area = form;
}


gtBase* gtMainWindowXm::container()
{
    return work_area ? (gtBase*)work_area : (gtBase*)this;
}


gtMenuBar* gtMainWindowXm::menu_bar()
{
    return menubar;
}


void gtMainWindowXm::message(const char* string)
{
    if(message_area)
	message_area->text(string);
}


/*
   START-LOG-------------------------------------------

   $Log: gtMainWindowXm.h.C  $
   Revision 1.2 1995/05/04 15:32:26EDT kws 
   Product name conversion
// Revision 1.2  1993/01/26  04:32:20  glenn
// Do not reference trigger_count.
//
// Revision 1.1  1993/01/14  19:46:32  glenn
// Initial revision
//
   END-LOG---------------------------------------------
*/
