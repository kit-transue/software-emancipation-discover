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
// include files

#include <general.h>
#include "Object.h"

#include <InterViews/enter-scope.h>
#include <InterViews/event.h>
#include <InterViews/rubrect.h>
#include <InterViews/rubline.h>
#include <InterViews/leave-scope.h>
#include "ivfix.h"

#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include "time_log.h"
#include "graSymbol.h"
#include "graResources.h"
#include "graWindow.h"
#include "graTools.h"

#include "viewNode.h"
#include "viewGraHeader.h"
#include "viewSelection.h"

static void track_rubberband(iv3(Interactor)* i, iv3(Rubberband)& r, iv3(Event)& e)
{
    Initialize(track_rubberband);

    r.SetPainter(new iv3(Painter));
    r.SetCanvas(i->GetCanvas());

    r.Track(e.x, e.y);
    r.Draw();
    while(i->Read(e), e.eventType != UpEvent)
    {
	e.target->GetRelative(e.x, e.y, i);
	r.Track(e.x, e.y);
    }
    r.Erase();

    Return
}

static void order_ascending(int& a, int& b)
{
    Initialize(order_ascending);

    if(a > b)
    {
	int temp = a;
	a = b;
	b = temp;
    }

    Return
}

void gra_drag_symbol(viewSelection *view_sel, iv3(Event)& e)
{
    Initialize(gra_drag_symbol);

    if(!is_viewSymbolNode(view_sel->view_node))
	Return

    viewSymbolNodePtr view_node = checked_cast(viewSymbolNode,view_sel->view_node);
    graParametricSymbol *symbol = view_node->get_picture();

    iv3(SlidingRect) *slide_rect = NULL;
    iv3(Rubberband)* feedback = symbol->get_rubband();
    if(feedback == NULL)
    {
	iv2(Coord) x0, y0, x1, y1;
	symbol->GetBox(x0, y0, x1, y1);
	slide_rect = new iv3(SlidingRect)(nil, nil, x0, y0, x1, y1, e.x, e.y);
	slide_rect->SetPainter(new iv3(Painter));
	feedback = slide_rect;
    }

    track_rubberband(e.target, *feedback, e);

    delete slide_rect;		// This is a no-op if symbol has a rubband.

    Return
}

void gra_line_select(iv3(Event)& e, iv2(Coord)& x0, iv2(Coord)& y0, iv2(Coord)& x1, iv2(Coord)& y1)
{
    Initialize(gra_line_select);

    iv3(RubberLine) feedback(nil, nil, e.x, e.y, e.x, e.y);

    track_rubberband(e.target, feedback, e);

    feedback.GetCurrent(x0, y0, x1, y1);

    Return
}

void gra_rect_select(iv3(Event)& e, iv2(Coord)& x0, iv2(Coord)& y0, iv2(Coord)& x1, iv2(Coord)& y1)
{
    Initialize(gra_rect_select);

    iv3(RubberRect) feedback(nil, nil, e.x, e.y, e.x, e.y);

    track_rubberband(e.target, feedback, e);

    feedback.GetCurrent(x0, y0, x1, y1);

    order_ascending(x0, x1);
    order_ascending(y0, y1);

    Return
}

/*
// graDragSymbol.C
//------------------------------------------
// synopsis:
// Feedback for symbols while they are being dragged.
//
// description:
// gra_drag_symbol() handles input until an UpEvent is received.
// An image representing the symbol that is being moved replaces
// the usual mouse cursor during this time.
//------------------------------------------
// $Log: graDragSymbol.C  $
// Revision 1.4 2000/07/10 23:05:02EDT ktrans 
// mainline merge from Visual C++ 6/ISO (extensionless) standard header files
Revision 1.2.1.4  1994/07/01  22:10:02  jerry
Bug track: 7704
Smit bears full responsibility.

Revision 1.2.1.3  1992/11/23  18:12:55  wmm
typesafe casts.

Revision 1.2.1.2  1992/10/09  18:37:02  builder
fixed rcs header

Revision 1.2.1.1  92/10/07  20:42:27  smit
*** empty log message ***

Revision 1.2  92/10/07  20:42:26  smit
*** empty log message ***

Revision 1.1  92/10/07  18:20:34  smit
Initial revision

Revision 1.1  92/10/07  17:56:13  smit
Initial revision

// Revision 1.7  92/07/30  00:34:13  jon
// oops, missed an iv3() around Painter
// 
// Revision 1.6  92/07/29  23:44:42  jon
// InterViews scope fix
// 
// Revision 1.5  92/03/19  09:33:04  builder
// Fixed nesting problem.
// 
// Revision 1.4  92/02/25  21:15:15  builder
// fix bugs with include files.
// 
// Revision 1.3  92/01/27  16:51:36  glenn
// Add gra_rect_select for rubberband rectangle selection.
// Add track_rubberband to unify rubberband tracking.
// Add order_ascending to sort pairs of coordinates.
// 
//Revision 1.2  92/01/24  16:58:41  glenn
//Look for pre-defined RubberBand object when dragging.
//If undefined, just use a box.
//
//Revision 1.1  92/01/17  17:19:10  glenn
//Initial revision
//
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------
*/
