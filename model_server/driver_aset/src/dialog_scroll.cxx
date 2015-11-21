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
// dialog_scroll.C
//------------------------------------------
// synopsis:
// Scrollbar & panner methods for dialogs
//------------------------------------------

// Include files

#include "machdep.h"
#include "objOper.h"
#include "dialog.h"
#include "Panner.h"
#include "ui.h"
#include "genError.h"
#include "viewGraHeader.h"

//------------------------------------------
// manipulate_perspective
//
// Change the boundaries and/or magnification of the visible area in the
// specified view.
//------------------------------------------

void dialog::manipulate_perspective(
    int view_num, int operation, int i1, int i2, int i3, int i4)
{
    Initialize(dialog::manipulate_perspective);

    viewPtr viewp = get_view_ptr(view_num);
    if(viewp)
	viewp->manipulate_perspective(operation, i1, i2, i3, i4);

    Return
}

//------------------------------------------
// get_slider_style
//
// Return the flags that describe the slider's location and visibility.
//------------------------------------------

int dialog::get_slider_style(int view_num)
{
    Initialize(dialog::get_slider_style);

    viewGraHeader* viewp = checked_cast(viewGraHeader,get_view_ptr(view_num));
    if(viewp)
	ReturnValue(viewp->get_slider_style());
    else
	ReturnValue(0);
}

//------------------------------------------
// set_slider_style
//
// Modify the slider's location and/or visibility.
//------------------------------------------

void dialog::set_slider_style(int view_num, int style)
{
    Initialize(dialog::set_slider_style);

    viewGraHeader* viewp = checked_cast(viewGraHeader,get_view_ptr(view_num));
    if(viewp)
	viewp->set_slider_style(style);

    Return
}

//------------------------------------------
// register_scrollbars
//
// Inform the dialog of the presence of scrollbars for the view.
//------------------------------------------

void dialog::register_scrollbars(int view_num, void* hscroll, void* vscroll)
{
    Initialize(dialog::register_scrollbars);

    if((view_num >= 0) && (view_num < max_view_num()))
    {
	get_horz_scroll(view_num) = hscroll;
	get_vert_scroll(view_num) = vscroll;
    }

    Return
}

//------------------------------------------
// register_panner
//
// Inform the dialog of the presence of a panner for the view.
//------------------------------------------

void dialog::register_panner(int view_num, void* panner)
{
    Initialize(dialog::register_panner);

    if((view_num >= 0) && (view_num < max_view_num()))
	get_panner(view_num) = panner;

    Return
}

//------------------------------------------
// unregister_scrollbars
//
//------------------------------------------

void dialog::unregister_scrollbars(int view_num)
{
    Initialize(dialog::unregister_scrollbars);

    if((view_num >= 0) && (view_num < max_view_num()))
    {
	get_horz_scroll(view_num) = 0;
	get_vert_scroll(view_num) = 0;
    }

    Return
}

//------------------------------------------
// unregister_panner
//
//------------------------------------------

void dialog::unregister_panner(int view_num)
{
    Initialize(dialog::unregister_panner);

    if((view_num >= 0) && (view_num < max_view_num()))
	get_panner(view_num) = 0;

    Return
}

//------------------------------------------
// set_horiz_scrollbar_value
//
// Set the position of the rightmost edge of the horizontal thumb.
//------------------------------------------

void dialog::set_horiz_scrollbar_value(int view_num, int val)
{
    Initialize(dialog::set_horiz_scrollbar_value);

    if((view_num >= 0) && (view_num < max_view_num()))
    {
	Widget hscroll = Widget(get_horz_scroll(view_num));
	if (hscroll)
	    ui_set_scroll_value(hscroll, val);

	Widget panner = Widget(get_panner(view_num));
	if(panner)
	{
	    int min = 0, max = 100;
	    ui_get_scroll_range (hscroll, &min, &max);
	    float factor = 100.0 / (max > min ? max - min : 1);

	    int x0, y0, w, h, cx, cy, cw, ch;
	    Panner_values(panner, &x0, &y0, &w, &h, &cx, &cy, &cw, &ch);

	    // The x position varies between the thumb width and 100
	    // EXCEPT when the graphic is smaller in width than the
	    // viewer.  Then the thumb width is 100 and val = 0;
	    cx = val - cw;
	    if (cx < 0)
		cx = 0;
	    if (cx > 100)
		cx = 100;

	    Panner_perspective(panner, 0, 0, 100, 100,
			       (int)(cx * factor), cy, cw, ch);
	}
    }

    Return
}

//------------------------------------------
// set_vert_scrollbar_value
//
// Set the position of the bottommost edge of the vertical thumb.
//------------------------------------------

void dialog::set_vert_scrollbar_value(int view_num, int val)
{
    Initialize(dialog::set_vert_scrollbar_value);

    if((view_num >= 0) && (view_num < max_view_num()))
    {
	Widget vscroll = Widget(get_vert_scroll(view_num));
	if (!vscroll) Return;

	ui_set_scroll_value(vscroll, val);

	Widget panner = Widget(get_panner(view_num));
	if(panner)
	{
	    int min = 0, max = 100;
	    ui_get_scroll_range (vscroll, &min, &max);
	    float factor = 100.0 / (max > min ? max - min : 1);

	    int x0, y0, w, h, cx, cy, cw, ch;
	    Panner_values(panner, &x0, &y0, &w, &h, &cx, &cy, &cw, &ch);
	    Panner_perspective(panner, 0, 0, 100, 100,
			       cx, (int) ((max - val) * factor), cw, ch);
	}
    }

    Return
}

//------------------------------------------
// set_horiz_slider_size
//
// Set the size of the horizontal thumb.
//------------------------------------------

void dialog::set_horiz_slider_size(int view_num, int val)
{
    Initialize(dialog::set_horiz_slider_size);

    if((view_num >= 0) && (view_num < max_view_num()))
    {
	Widget hscroll = Widget(get_horz_scroll(view_num));
	if(!hscroll) Return;

	ui_set_slider_size(hscroll, val);
	Widget panner = Widget(get_panner(view_num));
	if(panner)
	{
	    int min = 0, max = 100;
	    ui_get_scroll_range (hscroll, &min, &max);
	    float factor = 100.0 / (max > min ? max - min : 1);

	    int x0, y0, w, h, cx, cy, cw, ch;
	    Panner_values(panner, &x0, &y0, &w, &h, &cx, &cy, &cw, &ch);
	    Panner_perspective(panner, 0, 0, 100, 100,
			       cx, cy, (int)(val * factor), ch);
	}
    }

    Return
}

//------------------------------------------
// set_vert_slider_size
//
// Set the size of the vertical thumb.
//------------------------------------------

void dialog::set_vert_slider_size(int view_num, int val)
{
    Initialize(dialog::set_vert_slider_size);

    if((view_num >= 0) && (view_num < max_view_num()))
    {
	Widget vscroll = Widget(get_vert_scroll(view_num));
	if(!vscroll) Return;

	ui_set_slider_size(vscroll, val);
	Widget panner = Widget(get_panner(view_num));
	if(panner)
	{
	    int min = 0, max = 100;
	    ui_get_scroll_range (vscroll, &min, &max);
	    float factor = 100.0 / (max > min ? max - min : 1);

	    int x0, y0, w, h, cx, cy, cw, ch;
	    Panner_values(panner, &x0, &y0, &w, &h, &cx, &cy, &cw, &ch);
	    Panner_perspective(panner, 0, 0, 100, 100,
			       cx, cy, cw, (int) (val * factor));
	}
    }

    Return
}

void dialog::set_vert_scrollbar_range (int view_num, int min, int max)
{
    Initialize (dialog::set_vert_scrollbar_range);

    if((view_num >= 0) && (view_num < max_view_num()))
    {
        Widget vscroll = Widget(get_vert_scroll(view_num));
        if(vscroll)
	    ui_set_scroll_range (vscroll, min, max);

	Widget panner = Widget(get_panner(view_num));
        if(panner)
        {
            int x0, y0, w, h, cx, cy, cw, ch;
            Panner_values(panner, &x0, &y0, &w, &h, &cx, &cy, &cw, &ch);
            Panner_perspective(panner, min, y0, w, max,
                               cx, cy, cw, ch);
	}
    }

    Return
}
/*
   START-LOG-------------------------------------------

   $Log: dialog_scroll.cxx  $
   Revision 1.2 1996/04/30 07:25:26EDT kws 
   IFDEF NEW_UI
Revision 1.2.1.6  1992/12/14  15:10:18  oak
Fixed the thumb box for cases when
the graphic is smaller than the view.

Revision 1.2.1.5  1992/11/21  21:04:38  builder
typesafe casts.

Revision 1.2.1.4  1992/11/18  17:48:27  oak
Fixed set_vert_slider_size routine.

Revision 1.2.1.3  1992/11/05  22:29:26  smit
fixed bug #1838

Revision 1.2.1.2  1992/10/09  18:44:04  smit
*** empty log message ***


   END-LOG---------------------------------------------

*/
