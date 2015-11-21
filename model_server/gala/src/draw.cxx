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
#include <vport.h>
#include vdrawHEADER
#include vcolorHEADER

#include <gdraw.h>

void Draw3DBottomRight(const vrect* rect, int border, vcolor* borderColor)
{
  vdraw::GSave();
  vdraw::SetColor(borderColor);
  vdraw::SetLineWidth(1);

  int x = rect->GetX();
  int y = rect->GetY();
  int w = rect->GetWidth();
  int h = rect->GetHeight();
  
  vdraw::MoveTo(x, y);
  vdraw::LineTo(x + w, y);
  vdraw::LineTo(x + w, y + h);
  vdraw::LineTo(x + w - border, y + h - border);
  vdraw::LineTo(x + w - border, y + border);
  vdraw::LineTo(x + border, y + border);
  vdraw::LineTo(x, y);
  
  vdraw::Fill();
  vdraw::GRestore();
}


void Draw3DTopLeft(const vrect* rect, int border, vcolor* borderColor)
{
    vdraw::GSave();
    vdraw::SetColor(borderColor);
    vdraw::SetLineWidth(1);
    
    int x = rect->GetX();
    int y = rect->GetY();
    int w = rect->GetWidth();
    int h = rect->GetHeight();
    
    vdraw::MoveTo(x, y);
    vdraw::LineTo(x, y + h);
    vdraw::LineTo(x + w, y + h);
    vdraw::LineTo(x + w - border, y + h - border);
    vdraw::LineTo(x + border, y + h - border);
    vdraw::LineTo(x + border, y + border);
    vdraw::LineTo(x, y);

    vdraw::Fill();
    vdraw::GRestore();
}


void Draw3DCenter(const vrect* rect, int border, vcolor* borderColor)
{
    vdraw::GSave();
    vdraw::SetColor(borderColor);
    vdraw::SetLineWidth(1);
    
    int x = rect->GetX();
    int y = rect->GetY();
    int w = rect->GetWidth();
    int h = rect->GetHeight();
    
    vdraw::MoveTo(x + border, y + border);
    vdraw::LineTo(x + border, y + h - border);
    vdraw::LineTo(x + w - border, y + h - border);
    vdraw::LineTo(x + w - border, y + border);
    vdraw::LineTo(x + border, y + border);

    vdraw::Fill();
    vdraw::GRestore();
}
