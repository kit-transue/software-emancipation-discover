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
// gtStringEdXm.h.C
//------------------------------------------
// synopsis:
// Member function source code for gtStringEditor class in OSF Motif
//------------------------------------------

// INCLUDE FILES

#include <gtBaseXm.h>
#include <gtStringEdXm.h>

#include <Xm/Text.h>


// FUNCTION DEFINITIONS

gtStringEditor* gtStringEditor::create(
    gtBase *parent, const char* name, const char *contents)
{
    return new gtStringEditorXm(parent, name, contents);
}

gtStringEditorXm::gtStringEditorXm(
    gtBase *parent, const char* name, const char* contents)
{
    if(parent)
    {
	Arg args[2];
	int n = 0;

	XtSetArg(args[n], XmNeditMode, XmSINGLE_LINE_EDIT); n++;
	if (contents) { XtSetArg(args[n], XmNvalue, contents); n++; }

	r->widget(XmCreateText(parent->container()->rep()->widget(),
			       (char*)name, args, n));
    }
    destroy_init();
}


gtStringEditorXm::~gtStringEditorXm()
{}


void gtStringEditorXm::editable(int ed)
{
    Arg	args[1];
    XtSetArg(args[0], XmNeditable, ed ? True : False);
    XtSetValues(r->widget(), args, 1);
}


void gtStringEditorXm::read_only()
{
    Arg	args[3];
    
    XtSetArg(args[0], XmNeditable, False);
    XtSetArg(args[1], XmNnavigationType, XmNONE);

    XtSetValues(r->widget(), args, 2);
}

    
long gtStringEditorXm::get_last_position()
{
    return XmTextGetLastPosition(r->widget());
}


void gtStringEditorXm::set_selection(long first, long last)
{
    XmTextSetSelection(r->widget(), first, last, NULL);
}


void gtStringEditorXm::set_callback(gtStringCB callback, void* client_data)
{
    add_callback(XmNactivateCallback, this,
		 gtCallbackFunc(callback), client_data);
}

void gtStringEditorXm::activate_callback(gtStringCB callback, void* client_data)
{
    add_callback(XmNactivateCallback, this,
		 gtCallbackFunc(callback), client_data);
}

void gtStringEditorXm::focus_callback(gtStringCB callback, void* client_data)
{
    add_callback(XmNfocusCallback, this,
		 gtCallbackFunc(callback), client_data);
}

void gtStringEditorXm::unfocus_callback(gtStringCB callback, void* client_data)
{
    add_callback(XmNlosingFocusCallback, this,
		 gtCallbackFunc(callback), client_data);
}

void gtStringEditorXm::changed_callback(gtStringCB callback, void* client_data)
{
    add_callback(XmNvalueChangedCallback, this,
		 gtCallbackFunc(callback), client_data);
}

char *gtStringEditorXm::text()
{
    return XmTextGetString(r->widget());
}
    
void gtStringEditorXm::text(const char* contents)
{
    // Do not use XmTextSetString() here,
    // since that triggers all the callbacks.

    Arg arg[1];
    XtSetArg(arg[0], XmNvalue, contents);
    XtSetValues(r->widget(), arg, 1);
}

void gtStringEditorXm::columns(int c)
{
    Arg	args[1];
    XtSetArg(args[0], XmNcolumns, c);
    XtSetValues(r->widget(), args, 1);
}

static void show_cursor_callback(Widget w, void* data, XmAnyCallbackStruct*)
{
    static int started = 0;
    Arg args[1];

    if (started) return;

    started = 1;
    XtSetArg(args[0], XmNcursorPositionVisible, data);
    XtSetValues(w, args, 1);
    started = 0;
}

void gtStringEditorXm::show_cursor(int flag)
{
    show_cursor_callback(r->widget(), (void*)flag, 0);
}

void gtStringEditorXm::hide_unfocused_cursor()
{
    XtAddCallback(r->widget(), XmNfocusCallback,
		  XtCallbackProc(show_cursor_callback), XtPointer(1));
    XtAddCallback(r->widget(), XmNmotionVerifyCallback,
		  XtCallbackProc(show_cursor_callback), XtPointer(1));
    XtAddCallback(r->widget(), XmNvalueChangedCallback,
		  XtCallbackProc(show_cursor_callback), XtPointer(1));
    XtAddCallback(r->widget(), XmNlosingFocusCallback,
		  XtCallbackProc(show_cursor_callback), XtPointer(0));
}


/*
   START-LOG-------------------------------------------

   $Log: gtStringEdXm.h.C  $
   Revision 1.3 2001/01/12 10:57:06EST sschmidt 
   Port to new SGI compiler
Revision 1.2.1.5  1993/10/13  15:06:15  kws
Fix critical recursion from happening on hp

Revision 1.2.1.4  1993/01/26  04:59:27  glenn
Check parent in ctor.
Use gtCallbackMgrXm.
Add dtor.

Revision 1.2.1.3  1993/01/21  14:59:14  jon
Added member function changed_callback

Revision 1.2.1.2  1992/10/09  19:03:53  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
