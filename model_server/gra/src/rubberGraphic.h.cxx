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
#include <InterViews/enter-scope.h>
#include <InterViews/painter.h>
#include <InterViews/canvas.h>
#include <InterViews/Graphic/base.h>
#include <InterViews/leave-scope.h>
#include "ivfix.h"

#include "rubberGraphic.h"

void rubberGraphic::Erase()
{
    if(graphic && canvas && canvas->rep())
	iv3(RubberRect)::Erase();

    graphic = NULL;
}

void rubberGraphic::Draw()
{
    iv3(IntCoord) x0, y0, x1, y1;

    if(!drawn)
    {
	if(canvas && canvas->rep()) // Don't try to draw on unopened windows.
	{
	    GetCurrent(x0, y0, x1, y1);
	    if (x0 == x1 || y0 == y1)
	    {
		output->FillRect(canvas, x0 - 1, y0 - 1, x1 + 1, y1 + 1);
	    }
	    else
	    {
		output->Rect(canvas, x0 - 1, y0 - 1, x1 + 1, y1 + 1);
		output->Rect(canvas, x0 - 2, y0 - 2, x1 + 2, y1 + 2);
	    }
	}
	drawn = true;
    }
}

void rubberGraphic::SetGraphic(Graphic* g)
{
    if(graphic = g)
	graphic->GetBox(fixedx, fixedy, trackx, tracky);
}

/*
// rubberGraphic.h.C
//------------------------------------------
// synopsis:
// class rubberGraphic is used for quick hiliting.
//
// description:
// ...
//------------------------------------------
// $Log: rubberGraphic.h.C  $
// Revision 1.2 1999/08/26 16:46:06EDT sschmidt 
// HP 10 port
Revision 1.2.1.2  1992/10/09  18:37:16  builder
fixed rcs header

Revision 1.2.1.1  92/10/07  20:42:52  smit
*** empty log message ***

Revision 1.2  92/10/07  20:42:51  smit
*** empty log message ***

Revision 1.1  92/10/07  18:20:36  smit
Initial revision

Revision 1.1  92/10/07  17:56:15  smit
Initial revision

// Revision 1.5  92/07/29  23:46:06  jon
// InterViews scope fix
// 
// Revision 1.4  92/05/28  11:59:36  smit
// Include canvas.h
// 
// Revision 1.3  92/05/13  14:17:20  glenn
// Be extra, extra careful about checking for valid canvas by looking
// for non-NULL canvas->rep().  This really shouldn't be necessary...
// Also, clear graphic in Erase, even if we can't do the erase.
// 
// Revision 1.2  92/03/30  09:35:52  glenn
// Check for non-NULL canvas before drawing.
// 
// Revision 1.1  92/03/03  18:30:13  glenn
// Initial revision
// 
//------------------------------------------
// Restrictions:
// ...
//------------------------------------------
*/
