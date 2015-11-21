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
// gtTopShellXm
//------------------------------------------
// synopsis:
// Implentation of member functions for class gtTopLevelShellXm
//
//------------------------------------------

// INCLUDE FILES

#include <gtNoParent.h>
#include <gtBaseXm.h>
#include <gtTopShellXm.h>

#include <X11/Shell.h>


// FUNCTION DEFINITIONS

gtTopLevelShell* gtTopLevelShell::create(
    gtBase *parent, const char* name)
{
    return new gtTopLevelShellXm(parent, name);
}


gtTopLevelShellXm::gtTopLevelShellXm(gtBase* parent, const char* name)
{
    if(parent == NULL)
	parent = gtNoParent::app_shell();

    r->widget(XtCreatePopupShell((char*)name, topLevelShellWidgetClass,
				 parent->container()->rep()->widget(),
				 NULL, 0));
    destroy_init();
}


gtTopLevelShellXm::~gtTopLevelShellXm()
{}


void gtTopLevelShellXm::title(const char* title)
{
    if(title)
    {
	title = gettext(title);
	Arg args[1];
	XtSetArg(args[0], XmNtitle, title);
	XtSetValues(r->widget(), args, 1);
    }
}

void gtTopLevelShellXm::min_width(int width)
{
    Arg args[1];
    XtSetArg(args[0], XmNminWidth, width);
    XtSetValues(r->widget(), args, 1);
}

void gtTopLevelShellXm::min_height(int height)
{
    Arg args[1];
    XtSetArg(args[0], XmNminHeight, height);
    XtSetValues(r->widget(), args, 1);
}

void gtTopLevelShellXm::max_width(int width)
{
    Arg args[1];
    XtSetArg(args[0], XmNmaxWidth, width);
    XtSetValues(r->widget(), args, 1);
}

void gtTopLevelShellXm::max_height(int height)
{
    Arg args[1];
    XtSetArg(args[0], XmNmaxHeight, height);
    XtSetValues(r->widget(), args, 1);
}

void gtTopLevelShellXm::clear_background()
{
    XSetWindowBackgroundPixmap(
	XtDisplay(r->widget()), XtWindow(r->widget()), None);
}

			   
void gtTopLevelShellXm::destroy_callback(
    gtTLShellCB callback, void *client_data)
{
    add_callback(XmNdestroyCallback, this,
		 gtCallbackFunc(callback), client_data);
}


/*
   START-LOG-------------------------------------------

   $Log: gtTopShellXm.h.C  $
   Revision 1.2 1995/05/04 15:32:00EDT kws 
   Product name conversion
Revision 1.2.1.4  1993/12/07  20:46:16  andrea
Bug track: 5197
I added min_width, min_height, max_width, max_height member functions

Revision 1.2.1.3  1993/01/26  05:02:45  glenn
Use gtCallbackMgrXm.
Add dtor.

Revision 1.2.1.2  1992/10/09  19:04:06  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
