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
#include <InterViews/cursor.h>
#include <InterViews/perspective.h>
#include <InterViews/leave-scope.h>
#include "ivfix.h"

#include <X11/cursorfont.h>
#ifndef ISO_CPP_HEADERS
#include <stdlib.h>
#else /* ISO_CPP_HEADERS */
#include <cstdlib>
using namespace std;
#endif /* ISO_CPP_HEADERS */

#include "perspective.h"
#include "representation.h"
#include "genError.h"
#include "graWindow.h"
#include "graSymbol.h"
#include "graResources.h"
#include "graTools.h"
#include "smt.h"
#include "driver.h"
#include "dialog.h"
#include "viewGraHeader.h"
#include "viewNode.h"

#ifdef ibm
   extern "C" int usleep(unsigned int);
#endif

// Define relational stuff

// External functions

extern "C" int getmagfact();

//------------------------------------------
// gra_find_selection
//
// Convert a location in a window into a selection.
//------------------------------------------

graParametricSymbol*
gra_find_selection(graWindow* window, int x, int y, viewSelection& view_sel)
{
    Initialize(gra_find_selection);

    view_sel.view_node = view_sel.connector = NULL;
    view_sel.sub_type = 0;

    Picture *picture = (Picture*)window->GetGraphic();
    BoxObj pt(x-TOOLPickAperture, y-TOOLPickAperture,
              x+TOOLPickAperture, y+TOOLPickAperture);
    graParametricSymbol* symbol =
	(graParametricSymbol*)(picture->LastGraphicIntersecting(pt));

    if(symbol) {
	Graphic **sub_symbols = NULL;
	int num_sub_symbols = symbol->GraphicsIntersecting(pt, sub_symbols);

	symbol->find_insertion(
	    window, sub_symbols, num_sub_symbols, &view_sel);

	if (num_sub_symbols > 0)
	    delete [] sub_symbols;	// Clean up after GraphicsIntersecting.
    }
    if(view_sel.view_node)
	ReturnValue(checked_cast(viewSymbolNode,view_sel.view_node)->get_picture());
    ReturnValue(NULL);
}




RelClass(astTree);
smtTree* ast_get_smt(astTree*);		// Helper function

//------------------------------------------
// for tracing in debugger mode
//------------------------------------------

static int gra_trace_stop_flag;
static int gra_tracing_on;
static int gra_tracing_delay;

void gra_set_trace_stop_flag(int stop_f) { gra_trace_stop_flag = stop_f;}
int  gra_get_trace_stop_flag() { return gra_trace_stop_flag ;}
void gra_set_tracing_on(int i) { gra_tracing_on = i;}
int  gra_get_tracing_on() { return gra_tracing_on ;}
void gra_set_tracing_delay(int i) { gra_tracing_delay = i;}
int  gra_get_tracing_delay() { return gra_tracing_delay;}

/*_______________________________________________________________________
!
! Function:	gra_manipulate_perspective
!
! Arguments:	graWindow * window
!		int op			Operation on the window.
!		int i1			Passed value.
!		int i2			Passed value.
!		int 		        Not Used.
!		int 		        Not Used.
!       
! Returns:	void
!
! Synopsis:	Recalculates the view coordinates and/or the magnification
!		of the current view.  
!
!		class Perspective 
!    		IntCoord x0, y0;		origin of the graphic
!    		int width, height;		total size of the graphic
!    		IntCoord curx, cury;		current view position
!    		int curwidth, curheight;	current view size
!    		int sx, sy, lx, ly;		small and large scrolling increments
!
!		Calculates curx and cury given a change in the slider
!		bars or the thumb box.  Here i1 and i2 usually contain 
!		the right or top edge values of the slider bars or the
!		thumb box.  i1 and i2 can be any value between the control's
!		min and (max - control' size) where the control's size is 
!		the percentage of the graphic shown in the view, roughly 
!		(max - min) * (view size / graphic size).
!
!		If the graphic is smaller than the view then the graphic
!		gets placed in the upper left corner of the view.  Interviews'
!		coordinates are euclidian so y goes - to +, bottom to top.
_______________________________________________________________________*/ 
void gra_manipulate_perspective(graWindow *window, int op, int i1, int i2,
				int, int)
{
    Initialize(gra_manipulate_perspective);

    if(window == NULL)
	Return

    iv3(Perspective)& ps = *window->GetPerspective();
    iv3(Perspective) nps;
    float mag;
    int ticks = 100;


    // Calculate the size of the scroll bars
    int horiz_size = ticks;
    if((ps.width) && (ps.width - ps.curwidth > 0))
	horiz_size = round(double(ticks) * double(ps.curwidth) / double(ps.width));
    if(horiz_size > ticks)
	horiz_size = ticks;
    
    int vert_size = ticks;
    if((ps.height) && (ps.height - ps.curheight > 0))
	vert_size = round(double(ticks) * double(ps.curheight) / double(ps.height));
    if(vert_size > ticks)
	vert_size = ticks;


    // Perform operation.
    switch(op) {
      case VIEW_PAN_XY:
	// i1 is the percentage from the left side of the horz. scrollbar.
	// i2 is the percentage from the top of the vert. scrollbar.
	nps = ps;
	if ((nps.width > nps.curwidth) && (ticks - horiz_size != 0))
	    nps.curx = nps.x0 + round((double(i1) / double(ticks - horiz_size)) 
				      * double(nps.width - nps.curwidth));
	else
	    nps.curx = nps.x0;
	if ((nps.height > nps.curheight) && (ticks - vert_size != 0))
	    nps.cury = nps.y0 + round((1.0 - (double(i2) / double(ticks - vert_size))) 
		* double(nps.height - nps.curheight));
	else
	    nps.cury = nps.y0 + nps.height - nps.curheight;
	window->Adjust(nps);
	break;

      case VIEW_PAN_X:
	// i1 is the percentage from the left side of the horz. scrollbar.
	// i2 is garbage.
	nps = ps;
	if ((nps.width > nps.curwidth) && (ticks - horiz_size != 0))
	    nps.curx = nps.x0 + round((double(i1) / double(ticks - horiz_size)) 
				      * double(nps.width - nps.curwidth));
	else
	    nps.curx = nps.x0;
	window->Adjust(nps);
	break;

      case VIEW_PAN_Y:
	// i1 is the percentage from the top of the vert. scrollbar.
	// i2 is garbage.
	nps = ps;
	if((nps.height > nps.curheight) && (ticks - vert_size != 0))
	    nps.cury = nps.y0 + round((1.0 - (double(i1) / double(ticks - vert_size))) 
				      * double(nps.height - nps.curheight));
	else
	    nps.cury = nps.y0 + nps.height - nps.curheight;
	window->Adjust(nps);
	break;

      case VIEW_PAN_SMALL_X:
      case VIEW_PAN_SMALL_Y:
      case VIEW_PAN_LARGE_X:
      case VIEW_PAN_LARGE_Y:
        break;

      case VIEW_ZOOM_XY:
	// getmagfact() is ten times the desired change in the magnification.
	// Sign of i1 indicates zooming in or out.

	mag = window->GetMagnification();
	if(i1 > 0)		// Zoom in.
	    mag *= getmagfact() / 10.0;
	else if(i1 < 0)		// Zoom out.
	    mag /= getmagfact() / 10.0;
	window->set_magnification(mag);
	break;

      case VIEW_ZOOM_X:
      case VIEW_ZOOM_Y:
	break;

      case VIEW_ZOOM_TO_FIT:
	mag = window->GetMagnification() *
	    min(float(ps.curwidth) / float(ps.width),
		float(ps.curheight) / float(ps.height));
	window->set_magnification(mag);
	break;

      case VIEW_SAVE:
      case VIEW_DELETE:
	break;

      case VIEW_RESET:
	window->set_magnification(1.0);
	break;

      case VIEW_PREV:
      case VIEW_NEXT:
	break;

      case VIEW_UPDATE:
	window->dialog_scrollbars();
	break;
    }

    Return
}

/*
// graTools
//------------------------------------------
// synopsis:
// Tools for handling local mouse events and functions for hiliting
//
// description:
// This file contains tools for handling local mouse events. Any mouse
// event that can't be handled locally, is dispatched to the driver. There
// are also general purpose functions to handle hiliting, selecting and 
// focusing symbols.
//------------------------------------------
// $Log: graTools.C  $
// Revision 1.6 2001/07/25 20:42:00EDT Sudha Kallem (sudha) 
// Changes to support model_server.
Revision 1.2.1.6  1994/07/20  20:53:45  mg
Bug track: NA
obsoleted playground

Revision 1.2.1.5  1992/12/10  14:49:28  oak
Added checked cast call.

Revision 1.2.1.4  1992/11/17  20:37:56  oak
Changed scroll bar -> perspective equations.

Revision 1.2.1.3  1992/10/28  14:33:47  oak
Fixed memory error with selecting text.

Revision 1.2.1.2  92/10/09  18:37:11  builder
fixed rcs header

Revision 1.2.1.1  92/10/07  20:42:40  smit
*** empty log message ***

Revision 1.2  92/10/07  20:42:39  smit
*** empty log message ***

Revision 1.1  92/10/07  18:20:35  smit
Initial revision

Revision 1.1  92/10/07  17:56:14  smit
Initial revision

// Revision 1.62  92/07/29  23:45:48  jon
// InterViews scope fix
// 
// Revision 1.61  92/05/28  20:47:27  glenn
// Remove unused header files.
// Check for NULL window in gra_manipulate_perspective.
// Add case VIEW_UPDATE.
// 
// Revision 1.60  92/05/27  13:19:10  glenn
// Implement case VIEW_PAN_XY in gra_manipulate_perspective.
// 
// Revision 1.59  92/05/18  08:30:30  smit
// fix gra_trace to use viewerShell.
// 
// Revision 1.58  92/05/01  01:42:45  smit
// *** empty log message ***
// 
// Revision 1.57  92/03/19  09:34:17  builder
// Fix nesting problem.
// 
// Revision 1.56  92/03/04  13:48:53  glenn
// Remove gra_position_cursor_in_symbol.
// 
// Revision 1.55  92/03/03  18:36:00  glenn
// Remove graTool and all tool functions (see graAction.h.C)
// 
// Revision 1.54  92/02/25  21:20:51  glenn
// Add perspective.h.
// 
// Revision 1.53  92/02/11  19:25:06  jont
// swapped popup and hierarchy
// 
// Revision 1.52  92/02/10  11:27:14  jont
// kluged hierarchical tool so dragging works
// 
// Revision 1.51  92/02/08  02:37:15  aharlap
// hiliting after change wiev in gra_trace
// 
// Revision 1.50  92/02/07  09:34:03  jont
// wrote code to bring up context-sensitive popup menus
// 
// Revision 1.49  92/01/29  19:39:48  smit
// Fixed couple of bugs.
// 
// Revision 1.48  92/01/29  11:16:02  kws
// Fix error trap in gra_trace to clear hilited symbol on an error
// 
// Revision 1.47  92/01/27  18:12:47  aharlap
// gra_trace will not always turn on hierachy diagramm
// 
// Revision 1.46  92/01/27  16:53:23  glenn
// Re-implement gra_zoom_tool for rubberband zooming, tied to Control-LeftDown.
// 
// Revision 1.1  91/09/01  19:57:57  kws
// Initial revision
//
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------
*/
