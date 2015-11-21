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
#include "displayBoard.H"

DisplayBoard::DisplayBoard(UIObject& parent, application& app, boardFunc brdFunc,
                           int height, int width, color& col):
                           UIObject("displayBoard", parent, xmRowColumnWidgetClass,
                                   XmNrowColumnType, XmWORK_AREA,
                                   XmNpacking, XmPACK_COLUMN,
                                   XmNnumColumns, width,
                                   XmNorientation, XmHORIZONTAL,
                                   XmNadjustLast, FALSE,
                                   NULL),
                           Board(height, width)

{
  XColor bgclr, fgclr;
  allocColor("Linen", &fgclr);
  allocColor("PaleVioletRed3", &bgclr);
  Pixel fgRet, top, bottom, select;
  getColors(bgclr.pixel, &fgRet, &top, &bottom, &select);
  setValues(XmNforeground, fgclr.pixel,
            XmNbackground, bgclr.pixel,
            XmNtopShadowColor, top,
            XmNbottomShadowColor, bottom,
            XmNborderColor, fgRet,
            NULL);

  for(int y=0; y<height; y++) {
    for(int x=0; x<width; x++)
      squares[x + (y * width)] = new DisplaySquare(*this, app, brdFunc, x, y, col);
  }
}

Square& DisplayBoard::getSquare(location& loc)
{
  return (*(squares[loc.x() + (loc.y() * width)]));
}

void DisplayBoard::fillSquare(location& loc, color& col) { getSquare(loc).fill(col); }
void DisplayBoard::clearSquare(location& loc) { getSquare(loc).clear(); }

void DisplayBoard::reset()
{
  for(int i=0; i<height; i++)
    for(int j=0; j<width; j++) {
      location loc(i,j);
      getSquare(loc).reset();
    }
}

void DisplayBoard::deactivate()
{
  for(int i=0; i<height; i++)
    for(int j=0; j<width; j++) {
      location loc(i,j);
      getSquare(loc).deactivate();
    }
}

void DisplayBoard::activate()
{
  for(int i=0; i<height; i++)
    for(int j=0; j<width; j++) {
      location loc(i,j);
      getSquare(loc).activate();
    }
}

void DisplayBoard::deactivateSquare(location& loc)
{
  if (!(validLocation(loc))) return;
  getSquare(loc).deactivate();
}

void DisplayBoard::activateSquare(location& loc)
{
  if (!(validLocation(loc))) return;
  getSquare(loc).activate();
}


void DisplayBoard::deemphasize()
{
  for(int i=0; i<height; i++)
    for(int j=0; j<width; j++) {
      location loc(i,j);
      getSquare(loc).deemphasize();
    }
}

void DisplayBoard::emphasize()
{
  for(int i=0; i<height; i++)
    for(int j=0; j<width; j++) {
      location loc(i,j);
      getSquare(loc).emphasize();
    }
}

void DisplayBoard::deemphasizeSquare(location& loc)
{
  if (!(validLocation(loc))) return;
  getSquare(loc).deemphasize();
}

void DisplayBoard::emphasizeSquare(location& loc)
{
  if (!(validLocation(loc))) return;
  getSquare(loc).emphasize();
}


void DisplayBoard::save(char* ) { }
void DisplayBoard::restore(char* ) { }
