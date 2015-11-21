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
#ifndef tipWindowINCLUDED
#define tipwinINCLUDED 1

#ifndef  vportINCLUDED
#include <vport.h>
#endif

#ifndef  vwindowINCLUDED
#include vwindowHEADER
#endif
#ifndef  vscribeINCLUDED
#include vscribeHEADER
#endif
#ifndef  vrectINCLUDED
#include vrectHEADER
#endif
#ifndef  vportINCLUDED
#include vportHEADER
#endif
#ifndef  vdialogINCLUDED
#include vdialogHEADER
#endif
#ifndef  vobjectINCLUDED
#include vobjectHEADER
#endif
#ifndef  vclassINCLUDED
#include vclassHEADER
#endif
#ifndef  vcharINCLUDED
#include vcharHEADER
#endif
#ifndef  vtimerINCLUDED
#include vtimerHEADER
#endif
#ifndef  vcontainerINCLUDED
#include <vcontain.h>
#endif
#include "gString.h"



/*
 * * * * * * * * * * PUBLIC CLASS DECLARATIONS * * * * * * * * * *
 */

#define INSET 1

class gdDrawingArea;

class tipWindow : public vwindow
{
public:

    // Galaxy ctors and RTTI functions

    vloadableINLINE_CONSTRUCTORS(tipWindow, vwindow);
    vkindTYPED_FULL_DECLARATION(tipWindow);

    static void                   Startup(void);
	static void                   ResetTimer(void);
	static void                   SetContainer(vcontainer* container);
    static void                   Display(vdialogItem*, vdialog*, const vchar*);
    static void                   Display(gdDrawingArea*, vdialog*, int cell, int diff_x, int diff_y, const vchar*);

    // new methods

    void SetDialog (vdialog*);
    vdialog *GetDialog() { return dialog; }
    void SetText (const vchar*);

    // overridden methods

    virtual void                  Open();
    virtual void                  Draw();

    static void TipTimerObserveProc( vtimer * );
    static void TipTimerObserveProcDrawing( vtimer * );

protected:

    // overridden methods

    virtual void                  Init();

private:

    // instance variables

    vdialog *dialog;
    vstr    *text;

    

    // class variables

    static vtimer                m_Timer;
    static tipWindow             *tipwin;
    static vcolor                *DefaultBackground;
    static vcolor                *DefaultForeground;
    static vdialog               *m_PendingDialog;
    static vcontainer            *m_oldContainer;
    static gString               m_PendingText;
    static int                   m_nTimerCount;
    static vdialogItem           *m_PendingItem;
    static int                   m_PendingCell;
    static gdDrawingArea         *m_PendingArea;
    static int                   diff_x, diff_y;
};

#endif /* #ifndef tipWindowINCLUDED */
