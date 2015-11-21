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
#ifndef _gra_ivInterface_h
#define _gra_ivInterface_h

#include "graInterface.h"
#include <InterViews/enter-scope.h>
#include <InterViews/Graphic/picture.h>
#include <InterViews/leave-scope.h>
#include "ivfix.h"

class graWindow;

class gra_ivInterface : public graInterface {
  public:
    gra_ivInterface();

    virtual graInterface *Copy();
    virtual void         CreatePicture();
    virtual void         CreateWin(viewGraHeader *, long parent_window);
    virtual void         CloseWin();
    virtual void         DeletePicture();
    virtual void         ManipulatePrespective(int op, int i1, int i2, int i3, int i4);
    virtual int          GetSliderStyle();
    virtual void         SetSliderStyle(int style);
    virtual int          WindowExists();
    virtual graParametricSymbol *GetTextFocus();
    virtual void         DamageArea(graParametricSymbol *sym);
    virtual void         InvalidatePicture();
    virtual void         RepaintView();
    virtual void         RefreshWindow();
    virtual void         Unhighlight();
    virtual void         InsertSymbol(graParametricSymbol *sym);
    virtual void         RemoveSymbol(graParametricSymbol *sym);
    virtual void         RemovePictures(void);
    virtual int          GetWindowSize(int& width, int& height);
    virtual void         SelectSymbol(graParametricSymbol *symbol);
    virtual void         UnselectSymbol(graParametricSymbol *symbol);
    virtual void         HighliteSymbol(graParametricSymbol *symbol);
    virtual void         UnhighliteSymbol(graParametricSymbol *symbol);
    virtual void         FocusSymbol(graParametricSymbol *symbol);
    virtual void         ComputePagesToPrint(int &rows, int &col);
    virtual float        ScaleToFitToPage();
    virtual int          SendPostscript(ostream& ostr);

    graWindow *GetWindow();
    Picture   *GetPicture();

  private:
    graWindow *window;
    Picture   *picture;
};

#endif
