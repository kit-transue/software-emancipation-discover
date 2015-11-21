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
// top_widgets.C
//------------------------------------------
// synopsis:
// A group of subroutines to remember and operate on the
// top-level widgets in an application.
//------------------------------------------

// include files

#include <cLibraryFunctions.h>

#ifndef ISO_CPP_HEADERS
#include <iostream.h>
#else /* ISO_CPP_HEADERS */
#include <iostream>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include <msg.h>

#define Object XObject
#define XTFUNCPROTO
//#include <uimx/UxLib.h>  03/08/93
#include <UxLib.h>
#undef Object

#include <gt.h>
#include "genError.h"
#include "genArr.h"
#include <gtDisplay.h>
#include "top_widgets.h"

#define PSET_OWN_CALLBACK 114   //for instance, 65 is the last used in Xatom.h

// static variables

static genArr(Widget) top_widgets;
static genArr(Cursor) cursor_stack;

static Cursor busy_cursor;
static int top_no_epoch = 0;

// external functions

void ste_interface_set_busy_cursor (int);

// Large "busy" cursor:

#define time32_width 32
#define time32_height 32
#define time32_x_hot 15
#define time32_y_hot 15
static char time32_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0x7f, 0xfe, 0xff, 0xff, 0x7f,
   0x8c, 0x00, 0x00, 0x31, 0x4c, 0x00, 0x00, 0x32, 0x4c, 0x00, 0x00, 0x32,
   0x4c, 0x00, 0x00, 0x32, 0x4c, 0x00, 0x00, 0x32, 0x4c, 0x00, 0x00, 0x32,
   0x8c, 0x00, 0x00, 0x31, 0x0c, 0x7f, 0xfe, 0x30, 0x0c, 0xfe, 0x7f, 0x30,
   0x0c, 0xfc, 0x3f, 0x30, 0x0c, 0xf8, 0x1f, 0x30, 0x0c, 0xe0, 0x07, 0x30,
   0x0c, 0x80, 0x01, 0x30, 0x0c, 0x80, 0x01, 0x30, 0x0c, 0x60, 0x06, 0x30,
   0x0c, 0x18, 0x18, 0x30, 0x0c, 0x04, 0x20, 0x30, 0x0c, 0x02, 0x40, 0x30,
   0x0c, 0x01, 0x80, 0x30, 0x8c, 0x00, 0x00, 0x31, 0x4c, 0x80, 0x01, 0x32,
   0x4c, 0xc0, 0x03, 0x32, 0x4c, 0xf0, 0x1f, 0x32, 0x4c, 0xff, 0xff, 0x32,
   0xcc, 0xff, 0xff, 0x33, 0x8c, 0xff, 0xff, 0x31, 0xfe, 0xff, 0xff, 0x7f,
   0xfe, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00};

#define time32m_width 32
#define time32m_height 32
static char time32m_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xcf, 0x00, 0x00, 0xf3, 0x6e, 0x00, 0x00, 0x76, 0x6e, 0x00, 0x00, 0x76,
   0x6e, 0x00, 0x00, 0x76, 0x6e, 0x00, 0x00, 0x76, 0x6e, 0x00, 0x00, 0x76,
   0xce, 0x00, 0x00, 0x73, 0x8e, 0x7f, 0xfe, 0x71, 0x0e, 0xff, 0xff, 0x70,
   0x0e, 0xfe, 0x7f, 0x70, 0x0e, 0xfc, 0x3f, 0x70, 0x0e, 0xf8, 0x1f, 0x70,
   0x0e, 0xe0, 0x07, 0x70, 0x0e, 0xe0, 0x07, 0x70, 0x0e, 0x78, 0x1e, 0x70,
   0x0e, 0x1c, 0x38, 0x70, 0x0e, 0x06, 0x60, 0x70, 0x0e, 0x03, 0xc0, 0x70,
   0x8e, 0x01, 0x80, 0x71, 0xce, 0x00, 0x00, 0x73, 0x6e, 0x80, 0x01, 0x76,
   0x6e, 0xc0, 0x03, 0x76, 0x6e, 0xf0, 0x1f, 0x76, 0x6e, 0xff, 0xff, 0x76,
   0xee, 0xff, 0xff, 0x77, 0xcf, 0xff, 0xff, 0xf3, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

// Small "busy" cursor:

#define time16_x_hot 7
#define time16_y_hot 7
#define time16_width 16
#define time16_height 16
static char time16_bits[] = {
   0x00, 0x00, 0xfe, 0x7f, 0x14, 0x28, 0x14, 0x28, 0x14, 0x28, 0x24, 0x24,
   0x44, 0x22, 0x84, 0x21, 0x84, 0x21, 0x44, 0x22, 0x24, 0x24, 0x14, 0x28,
   0x94, 0x29, 0xd4, 0x2b, 0xfe, 0x7f, 0x00, 0x00};

#define time16m_width 16
#define time16m_height 16
static char time16m_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f,
   0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f,
   0xfe, 0x7f, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff};

struct cursorData
{
    char*        bits;
    char*        maskBits;
    unsigned int width;
    unsigned int height;
    unsigned int xHotspot;
    unsigned int yHotspot;
};

static cursorData time32 = {
    time32_bits, time32m_bits, time32_width, time32_height,
    time32_x_hot, time32_y_hot
};

extern "C" void set_top_no_epoch (int no_epoch)
{
    top_no_epoch = no_epoch;
}

//------------------------------------------
// default_cursor
//------------------------------------------

static Cursor default_cursor()
{
    static Cursor def_cursor = NULL;
#ifndef NEW_UI
    if ((def_cursor == NULL) && (UxDisplay != NULL))
	def_cursor = XCreateFontCursor(UxDisplay, 68);
#endif
    return def_cursor ? def_cursor : None;
}

//------------------------------------------
// add_top_widget
//------------------------------------------

extern "C" void add_top_widget(Widget w)
{
#ifndef NEW_UI
    for(int i = 0; i < top_widgets.size(); ++i)
    {
	if(w == *top_widgets[i])
	{
	    msg("add_top_widget: duplicate widget.") << eom;
	    return;
	}
    }
    top_widgets.append(&w);

    const int last = cursor_stack.size() - 1;
    Cursor c = default_cursor();
    if(last >= 0)
	c = *cursor_stack[last];
    if(XtIsRealized(w))
    {
	if (UxDisplay == 0)
	    UxDisplay = XtDisplay(w);
	XDefineCursor(UxDisplay, XtWindow(w), c);
    }
    /* Following checking was added because of using this routine from parasniff where UxDisplay
       equal to 0 first several calls to this function (Isn\'t this strange ?) */
    if(UxDisplay)
	XFlush(UxDisplay);
#endif
}

//------------------------------------------
// rem_top_widget
//------------------------------------------

extern "C" void rem_top_widget(Widget w)
{
    Initialize(rem_top_widget);
   
#ifndef NEW_UI 
    for(int i = 0; i < top_widgets.size(); ++i)
    {
	if(w == *top_widgets[i])
	{
	    top_widgets.remove(i);
	    return;
	}
    }
    
    DBG {
	msg("rem_top_widget: missing widget.") << eom;
    }
#endif
}

int find_top_widget(Widget w)
{
#ifndef NEW_UI
    char * name = XtName(w);
    int ii = 0;
    for(int i = 0; i < top_widgets.size(); ++i)
    {
        Widget ww = *top_widgets[i];
	if(w == ww)
	{
	    return ii+1;
	}
        char * wn = XtName(ww);
        if(strcmp(name, wn)==0)
           ++ ii;
    }
#endif
    return 0;

}

Widget find_top_widget(char * name, int ind)
{
#ifndef NEW_UI
    int ii = 0;
    for(int i = 0; i < top_widgets.size(); ++i)
    {
       Widget w = *top_widgets[i];
       char * wn = XtName(w);
       if(strcmp(name, wn) == 0)
	{
           ++ii;
           if(ii==ind)
         	    return w;
	}
    }
#endif
    return 0;
}

//------------------------------------------
// set_cursor
//------------------------------------------

extern "C" void set_cursor(Cursor c)
{
#ifndef NEW_UI
    if (c == None)
	c = default_cursor();
    for(int i = 0; i < top_widgets.size(); ++i)
    {
	Widget w = *top_widgets[i];
	if(XtIsRealized(w))
	{
	    if (UxDisplay == 0)
		UxDisplay = XtDisplay(w);
	    XDefineCursor(UxDisplay, XtWindow(w), c);
	}
    }
    XFlush(UxDisplay);
    if (top_no_epoch == 0)
	ste_interface_set_busy_cursor( (c == busy_cursor) );
#endif
}

//------------------------------------------
// push_cursor
//------------------------------------------

extern "C" void push_cursor(Cursor c)
{
#ifndef NEW_UI
    if (c == None)
	c = default_cursor();
    cursor_stack.append(&c);
    set_cursor(c);
#endif
}

//------------------------------------------
// pop_cursor
//------------------------------------------

extern "C" void pop_cursor()
{
#ifndef NEW_UI
    // if non-interactive, do not do anything
    if (!gtDisplay::is_open()) return;

    Cursor c = default_cursor();
    const int last = cursor_stack.size() - 1;
    if(last >= 0)
    {
	cursor_stack.remove(last);
	if(last > 0)
	    c = *cursor_stack[last - 1];
    }
    set_cursor(c);
#endif
}

//------------------------------------------
// reset_cursor
//------------------------------------------

extern "C" void reset_cursor()
{
#ifndef NEW_UI
//    if(cursor_stack.size() > 0)
//      {
	cursor_stack.reset();
	set_cursor(default_cursor());
//    }
#endif
}

//------------------------------------------
// reset_current_cursor
//------------------------------------------

extern "C" void reset_current_cursor()
{
#ifndef NEW_UI
    Cursor c = default_cursor();
    const int curr = cursor_stack.size() - 1;
    if(curr >= 0)
	c = *cursor_stack[curr];
    set_cursor(c);
#endif
}

//------------------------------------------
// make_cursor
//------------------------------------------

static unsigned long make_cursor(cursorData& cd)
{
#ifndef NEW_UI
    Pixmap pixmap = XCreateBitmapFromData(
	UxDisplay, UxRootWindow, cd.bits, cd.width, cd.height);

    Pixmap maskPixmap = XCreateBitmapFromData(
	UxDisplay, UxRootWindow, cd.maskBits, cd.width, cd.height);

    Screen*const screen = ScreenOfDisplay(UxDisplay, UxScreen);

    XColor xcolors[2];
    xcolors[0].pixel = BlackPixelOfScreen(screen);
    xcolors[1].pixel = WhitePixelOfScreen(screen);

    XQueryColors(UxDisplay, DefaultColormapOfScreen(screen), xcolors, 2);

    Cursor cursor = XCreatePixmapCursor(UxDisplay, pixmap, maskPixmap,
					&(xcolors[0]), &(xcolors[1]),
					cd.xHotspot, cd.yHotspot);
    XFreePixmap(UxDisplay, pixmap);
    XFreePixmap(UxDisplay, maskPixmap);

    return cursor;
#else
    return NULL;
#endif
}

//------------------------------------------
// get_busy_cursor
//------------------------------------------

extern "C" unsigned long get_busy_cursor()
{
#ifndef NEW_UI
    if(!busy_cursor)
	busy_cursor = make_cursor(time32);
#endif
    return busy_cursor;
}


//------------------------------------------
// push_busy_cursor
//------------------------------------------

extern "C" void push_busy_cursor()
{
#ifndef NEW_UI
    // if non-interactive, do not do anything
    if (!gtDisplay::is_open()) return;

    push_cursor(get_busy_cursor());
#endif
}

extern long ste_get_pset_uxid();

extern "C" Window get_any_paraset_window ()
{
#ifndef NEW_UI
    Window id = (Window)ste_get_pset_uxid ();
    if (id)
	return id;
    for(int i = 0; i < top_widgets.size(); ++i)
    {
        Widget w = *top_widgets[i];
        if(XtIsRealized(w))
            return XtWindow(w);
    }
#endif
    return 0;
}

void pset_send_own_callback (void (*func)(void *), void *dt)
{
    Initialize(pset_send_own_callback);
#ifndef NEW_UI
    if (!gtDisplay::is_open())
	return;
    
    XEvent ev;
    Window win = get_any_paraset_window ();
    ev.xclient.window = win;
    ev.xany.type = ClientMessage;
    ev.xclient.message_type = PSET_OWN_CALLBACK;
    ev.xclient.format = 8;
    
    for (int i=0; i<5; ++i)
        ev.xclient.data.l[i] = 0;
    
    ev.xclient.data.l[0] = (int)func;
    ev.xclient.data.l[1] = (int)dt;

    //boris signature
    ev.xclient.data.l[2] = (int)'f';
    ev.xclient.data.l[3] = (int)'b';
    ev.xclient.data.l[4] = (int)'n';
    
    XSendEvent(UxDisplay, win, FALSE, (1L<<25)-1L, &ev);
    XFlush(UxDisplay);
#endif
}

extern "C" void ping_paraset ()
{
#ifndef NEW_UI
    if (!gtDisplay::is_open())
	return;

    pset_send_own_callback ();
#else
#endif
}

void pset_dispatch_own_callback (XEvent *ev)
{
#ifndef NEW_UI
    if (ev->xclient.message_type == PSET_OWN_CALLBACK &&
	ev->xclient.format == 8  &&
	//boris signature
	ev->xclient.data.l[2] == (int)'f' &&
	ev->xclient.data.l[3] == (int)'b' &&
	ev->xclient.data.l[4] == (int)'n'
	)
    {
	void (*func)(void *);
	    
	func = (void (*)(void *)) ev->xclient.data.l[0];
	void *dt = (void *) ev->xclient.data.l[1];
	
	if (func) 
	    (*func)(dt);
    }
#endif
}

tempCursor::tempCursor (Cursor c)
{
    Initialize(tempCursor::tempCursor);
    if (c == None)
	push_busy_cursor();
    else
	push_cursor (c);
}

tempCursor::~tempCursor ()
{
    pop_cursor ();
}

/*
   START-LOG-------------------------------------------

   $Log: top_widgets.C  $
   Revision 1.12 2000/07/12 18:15:55EDT ktrans 
   merge from stream_message branch
Revision 1.2.1.17  1994/08/01  19:17:13  farber
Bug track: 7898
add function to destroy all the widgets pointed to by array top_widgets (and reset the array top_widgets)

Revision 1.2.1.16  1994/07/07  01:08:47  boris
Bug track: 6713
Added Batch mode to Ext Clients

Revision 1.2.1.15  1994/03/19  01:32:57  aharlap
check for user interface in ping_paraset

Revision 1.2.1.14  1994/03/16  21:33:44  azaparov
Bug track: N/A
Fixed parasniff crash in add_top_widget function

Revision 1.2.1.13  1994/03/10  16:51:32  jon
Bug track: 6624
Set cursor to pointer when cursor stack is dry instead of
setting cursor to "None". Added hook into gtShellXm.h.C
to reset the cursor when gt dialogs are popped up
(the reset in add_top_widgets is too early, the widget
has not been realized at that point).

Revision 1.2.1.12  1993/11/22  15:46:23  jon
Fixed set_cursor to init UxDisplay if it is 0

Revision 1.2.1.11  1993/10/09  17:37:47  aharlap
async_parsing

Revision 1.2.1.10  1993/07/21  00:21:00  boris
added set_top_no_epoch() static function to block Epoch mouse cursor munipulayion

Revision 1.2.1.9  1993/06/17  21:07:59  davea
bug 3736 - disable push_busy_cursor() and pop_cursor()
if not in GUI mode

Revision 1.2.1.8  1993/05/11  00:04:06  boris
Fixed bug #580 with "busy cursor"

Revision 1.2.1.7  1993/05/10  23:19:34  boris
*** empty log message ***

Revision 1.2.1.6  1993/05/07  19:52:13  jon
Added find_top_widget and prt_top_widgets for the command tool

Revision 1.2.1.5  1993/03/08  21:49:09  sergey
Replaced <uimx/UxLib.h> by <UxLIb.h> to be able to compile it.

Revision 1.2.1.4  1992/10/23  17:54:29  builder
Fix stupid typos.

Revision 1.2.1.3  92/10/23  17:47:09  glenn
Add get_busy_cursor, make_cursor.

Revision 1.2.1.2  92/10/09  20:14:00  jon
RCS History Marker Fixup

   END-LOG---------------------------------------------
*/
