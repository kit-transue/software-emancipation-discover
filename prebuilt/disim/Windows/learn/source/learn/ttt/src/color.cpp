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
#include "color.H"

color::~color()  { delete name; }

void color::copyName(const char* colorName)
{
  if (!colorName) {
    name = new char[8];
    strcpy(name, " ");
  }
  name = new char[strlen(colorName) + 1];
  strcpy(name, colorName);
}

void color::save(char* ) { }
void color::restore(char* ) { }

void color::getPixmap(Widget window, Pixmap& pixmap)
{

    Pixel fg, bg;
    XtVaGetValues(window,
                  XmNforeground, &fg,
                  XmNbackground, &bg,
                  NULL);

    GC gc;
    gc = XCreateGC(XtDisplay(window),
                   RootWindowOfScreen(XtScreen(window)),
                   NULL, 0);
    XSetForeground(XtDisplay(window), gc, fg);
    XSetForeground(XtDisplay(window), gc, bg);

    pixmap = XmGetPixmap(XtScreen(window), name, fg, bg);

//    if (pixmap==XmUNSPECIFIED_PIXMAP) throw cannotGetPixmap(name);	

    if (pixmap==XmUNSPECIFIED_PIXMAP) {
      cout << "\nERROR: cannot open pixmap for color: " << name << "\n";
      exit(-1);
    }
}

int color::operator==(const color& c)
{
  return (!(strcmp(name, c.name)));
}

int color::operator!=(const color& c)
{
  return (!(!(strcmp(name, c.name))));
}

