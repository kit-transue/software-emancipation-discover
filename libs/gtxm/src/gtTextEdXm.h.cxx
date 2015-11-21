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
// gtTextEdXm.h.C
//------------------------------------------
// synopsis:
// Member function source code for gtTextEditor class in OSF Motif
//
// description:
//------------------------------------------

// INCLUDE FILES

#include <gtBaseXm.h>
#include <gtTextEdXm.h>

#include <Xm/Text.h>


// FUNCTION DEFINITIONS

gtTextEditor* gtTextEditor::create(
    gtBase* parent, const char* name, const char *contents)
{
    return new gtTextEditorXm(parent, name, contents, 0);
}

gtTextEditor* gtTextEditor::create(
    gtBase* parent, const char* name, const char *contents, int wrap_lines)
{
    return new gtTextEditorXm(parent, name, contents, wrap_lines);
}

gtTextEditorXm::gtTextEditorXm(
    gtBase* parent, const char* name, const char* contents, int wrap_lines)
{
    if(parent)
    {
	Arg args[5];
	int n=0;

	XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT), n++;
	XtSetArg(args[n], XmNresizeHeight, True), n++;
	XtSetArg(args[n], XmNwordWrap, True), n++;
	if (contents)
	    XtSetArg(args[n], XmNvalue, contents), n++;
	if (wrap_lines)
	    XtSetArg(args[n], XmNscrollHorizontal, False), n++;

	txt = XmCreateScrolledText(parent->container()->rep()->widget(),
				   (char *)name, args, n);
	XtManageChild(txt);
	txt_rep.widget(txt);

	r->widget(XtParent(txt));
    }
    destroy_init();
}

gtTextEditorXm::~gtTextEditorXm()
{}

void gtTextEditorXm::num_rows(int rows)
{
    Arg	args[1];

    XtSetArg(args[0], XmNrows, (Cardinal)rows);
    XtSetValues(txt, args, 1);
}

void gtTextEditorXm::num_columns(int cols)
{
    Arg	args[1];

    XtSetArg(args[0], XmNcolumns, (Cardinal)cols);
    XtSetValues(txt, args, 1);
}

void gtTextEditorXm::read_only_mode()
{
    Arg		args[4];

    XtSetArg(args[0], XmNautoShowCursorPosition, False);
    XtSetArg(args[1], XmNcursorPositionVisible, False);
    XtSetArg(args[2], XmNeditable, False);
    XtSetArg(args[3], XmNtraversalOn, False);
    XtSetValues(txt, args, 4);
}
	   
void gtTextEditorXm::normal_mode()
{
    Arg		args[4];

    XtSetArg(args[0], XmNautoShowCursorPosition, True);
    XtSetArg(args[1], XmNcursorPositionVisible, True);
    XtSetArg(args[2], XmNeditable, True);
    XtSetArg(args[3], XmNtraversalOn, True);
    XtSetValues(txt, args, 4);
}
	   
long gtTextEditorXm::get_last_position()
{
    long ans = XmTextGetLastPosition(txt);
    return ans;
}

char *gtTextEditorXm::text()
{
    char* ans = XmTextGetString(txt);
    return ans;
}

void gtTextEditorXm::insert(long pos, const char* string)
{
    XmTextInsert(txt, pos, (char*)string);
}

void gtTextEditorXm::text(const char *string)
{
    XmTextSetString(txt, (char*)string);
}

void gtTextEditorXm::set_selection(long first, long last)
{
    XmTextSetSelection(txt, first, last, NULL);
}

void gtTextEditorXm::show_position(long pos)
{
    XmTextShowPosition(txt, pos);
}

long gtTextEditorXm::get_top_pos()
{
    return XmTextGetTopCharacter(txt);
}

void gtTextEditorXm::set_top_pos(long pos)
{
    XmTextSetTopCharacter(txt, pos);
}

gtBase* gtTextEditorXm::container()
{
    return NULL;
}    
    
void gtTextEditorXm::word_wrap(int flag)
{
    Arg args[1];
    XtSetArg(args[0], XmNwordWrap, flag);
    XtSetValues(txt, args, 1);
}

void gtTextEditorXm::append(const char *app_text)
{
    long pos = get_last_position();
    insert(pos, app_text);
    pos = get_last_position();
    show_position(pos);
}

long gtTextEditorXm::current_pos()
{
    long ans = XmTextGetInsertionPosition(txt);
    return ans;
}

void gtTextEditorXm::changed_callback(gtTextCB callback, void* client_data)
{
    add_callback(XmNvalueChangedCallback, this,
		 gtCallbackFunc(callback), client_data, &txt_rep);
}

void gtTextEditorXm::vertical_slider_position(int& value, int& size, int& maximum, int& minimum)
{
    Widget scrolled_window;
    Widget vertical_slider;

    scrolled_window = XtParent(txt);
    XtVaGetValues(scrolled_window, XmNverticalScrollBar, &vertical_slider, NULL);
    XtVaGetValues(vertical_slider, XmNvalue, &value, XmNsliderSize, &size,
		  XmNmaximum, &maximum, XmNminimum, &minimum, NULL);
}

int gtTextEditorXm::get_background_color()
{
    int color;
    XtVaGetValues(txt, XmNbackground, &color, 0);
    return color;
}

void gtTextEditorXm::set_background_color(int color)
{
    XtVaSetValues(txt, XmNbackground, color, 0);
}

/*
   START-LOG-------------------------------------------

$Log: gtTextEdXm.h.C  $
Revision 1.2 1995/04/19 08:29:37EDT so 
Bug track: n/a
fix bug 9597
Revision 1.2.1.7  1993/11/22  15:42:06  jon
Added member functions get_top_position(), set_top_position(),
and vertical_slider_position(), also added word_wrap parameter
to constructor

Revision 1.2.1.6  1993/06/02  20:18:08  glenn
Remove Initialize and Return macros.  Remove genError.h.

Revision 1.2.1.5  1993/05/24  17:12:58  glenn
Initialize txt_rep in ctor.
Add destructor.
Add changed_callback().

Revision 1.2.1.4  1993/05/22  16:15:18  glenn
Do not disable horizontal scrollbar during widget creation.

Revision 1.2.1.3  1993/01/26  05:00:19  glenn
Check parent in ctor.
Use gtCallbackMgrXm.
Return NULL for container.

Revision 1.2.1.2  1992/10/09  19:03:55  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
