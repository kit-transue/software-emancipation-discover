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
#include "displaySquare.H"

DisplaySquare::DisplaySquare(UIObject& parent, application& a, boardFunc brdFunc,
                             int x, int y, color& col):
                             UIObject("displaySquare", parent, xmDrawnButtonWidgetClass,
                                     XmNwidth, 76, XmNheight, 76, 				     
				     NULL)
{
  XColor fgclr, bgclr;
  allocColors(fgclr, bgclr);

  Pixel fgRet, top, bottom, select;
  getColors(bgclr.pixel, &fgRet, &top, &bottom, &select);
  setValues(XmNforeground, fgclr.pixel,
	    XmNbackground, bgclr.pixel,
	    XmNtopShadowColor, top,
	    XmNbottomShadowColor, bottom,
	    XmNselectColor, select,
	    XmNarmColor, select,
	    XmNborderColor, fgRet,
	    NULL);

  function = brdFunc;
  app = &a;
  loc = new location(x, y);

  defaultColor = new color(col);
  clear();

  lastShadowType = XmSHADOW_OUT;
  deemphasize();

  XtAddCallback(window, XmNactivateCallback, &DisplaySquare::genCallback, (XtPointer) this);
}

void DisplaySquare::allocColors(XColor& fgclr, XColor& bgclr)
{
  allocColor("Linen" , &fgclr);
  allocColor("PaleVioletRed3" , &bgclr);
}

void DisplaySquare::genCallback(Widget, XtPointer clientData, XtPointer)
{
  DisplaySquare& ds = *(DisplaySquare *) clientData;
  (*(ds.function))(*(ds.app), *(ds.loc));
}

void DisplaySquare::fill(color& col)
{
  Square::fill(col);

  Pixmap pixmap;
  col.getPixmap(window, pixmap);

  XtVaSetValues(window,
                XmNlabelType, XmPIXMAP,
                XmNlabelPixmap, pixmap,
                NULL);
}



void DisplaySquare::clear()
{
  fill(*defaultColor);
  Square::clear();
}


void DisplaySquare::activate()
{
  Pixel HighlightColor;
  XtVaGetValues(window, XmNhighlightColor, &HighlightColor, NULL);

  XtVaSetValues(window,
                XmNhighlightColor, HighlightColor,
                XmNshadowType, XmSHADOW_OUT,
                NULL);

  lastShadowType = XmSHADOW_OUT;
}

void DisplaySquare::deactivate()
{
  Pixel noHighlightColor;
  XtVaGetValues(window, XmNbackground, &noHighlightColor, NULL);

  XtVaSetValues(window,
                XmNhighlightColor, noHighlightColor,
                XmNshadowType, XmSHADOW_IN,
                NULL);

  lastShadowType = XmSHADOW_IN;
}

void DisplaySquare::deemphasize()
{
  XtVaSetValues(window,
                XmNshadowThickness, 2,
                XmNshadowType, lastShadowType,
                NULL);
}

void DisplaySquare::emphasize()
{
  XtVaSetValues(window,
                XmNshadowThickness, 4,
                XmNshadowType, XmSHADOW_ETCHED_IN,
                NULL);
}

void DisplaySquare::save(char* )
{
//  throw cannotSave();  
}


void DisplaySquare::restore(char* )
{
//  throw cannotRestore(); 
}

DisplaySquare::~DisplaySquare()
{
  delete loc;
  delete defaultColor;
}


