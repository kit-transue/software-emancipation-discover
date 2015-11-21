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
#ifndef _displaySquare_h_
#define _displaySquare_h_

// standard
#include "x11_intrinsic.h"
#include <Xm/Xm.h>
#include <Xm/DrawnB.h>

// inheritance
#include "display.H"
#include "square.H"

// usage
#include "application.H"
#include "color.H"
#include "location.H"
#include "funcTypes.H"

class DisplaySquare: public UIObject, public Square {

public:

  DisplaySquare(UIObject& parent, application& app, boardFunc brdFunc,
                int height, int width, color& col);
  virtual ~DisplaySquare();

  virtual void fill(color& col);
  virtual void clear();

  virtual void activate();
  virtual void deactivate();

  virtual void emphasize();
  virtual void deemphasize();

  virtual void save(char* );
  virtual void restore(char* );

protected:

  static void genCallback(Widget, XtPointer, XtPointer);
  void allocColors(XColor& fgclr, XColor& bgclr);

  boardFunc function;
  application* app;
  location* loc;
  unsigned char lastShadowType;
  color *defaultColor;
};

#endif


