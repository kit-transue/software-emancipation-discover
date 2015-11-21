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
#include <tipwin.h>

#ifndef  vportINCLUDED
#include <vport.h>
#endif

#ifndef  vapplicationINCLUDED
#include vapplicationHEADER
#endif
#ifndef  vwindowINCLUDED
#include vwindowHEADER
#endif
#ifndef  vstrINCLUDED
#include vstrHEADER
#endif
#ifndef  vscribeINCLUDED
#include vscribeHEADER
#endif
#ifndef  vrectINCLUDED
#include vrectHEADER
#endif
#ifndef  vinstanceINCLUDED
#include vinstanceHEADER
#endif
#ifndef  vimageINCLUDED
#include vimageHEADER
#endif
#ifndef  vfontINCLUDED
#include vfontHEADER
#endif
#ifndef  vexINCLUDED
#include vexHEADER
#endif
#ifndef  vdrawINCLUDED
#include vdrawHEADER
#endif
#ifndef  vdialogINCLUDED
#include vdialogHEADER
#endif
#ifndef  vdebugINCLUDED
#include vdebugHEADER
#endif
#ifndef  vclassINCLUDED
#include vclassHEADER
#endif
#ifndef  vcursorINCLUDED
#include vcursorHEADER
#endif
#ifndef  vimageINCLUDED
#include vimageHEADER
#endif
#ifndef  vcharINCLUDED
#include vcharHEADER
#endif
#include vdomainviewHEADER
#include vliststrHEADER
#include vtimerHEADER
#include vlistitemHEADER

#include <gtoolbar.h>
#include "gdialog.h"
#include "gDrawingArea.h"

#if (vdebugDEBUG)
const char *_tag = "";
#undef vdebugTAG
#define vdebugTAG _tag
#endif

/* --------------------------------------------------------------------------
 * globals
 * ------------------------------------------------------------------------ */

tipWindow* tipWindow::tipwin = NULL;

vcolor *tipWindow::DefaultBackground = NULL;
vcolor *tipWindow::DefaultForeground = NULL;
vdialog *tipWindow::m_PendingDialog = NULL; 
vcontainer *tipWindow::m_oldContainer=NULL;
gdDrawingArea *tipWindow::m_PendingArea = NULL; 
int tipWindow::m_PendingCell = -1;
gString tipWindow::m_PendingText = (const vchar *)"";    
int tipWindow::m_nTimerCount = 0;
vdialogItem* tipWindow::m_PendingItem = NULL;
vtimer tipWindow::m_Timer = vtimer();
int tipWindow::diff_x = 0;
int tipWindow::diff_y = 0;


/* --------------------------------------------------------------------------
 * overridden methods
 * ------------------------------------------------------------------------ */


void tipWindow::SetDialog (vdialog* d)
{
    dialog = d;
}

void tipWindow::SetText (const vchar* t)
{
    text = vstrClone (t);
}

/*
 * Init(): Initialize a tip window object.
 */

void tipWindow::Init()
{
    vwindow::Init();
    
    SetStyle(vwindowSTYLE_BACKGROUND);
    SetPopup(TRUE);
}


/*
 * Open(): Open a tip window.  Override to properly position tip window
 *         relative to current dialog item.
 */

void tipWindow::Open(void)
{
    vrect objectRect;
    const vrect *parentRect;
    const vrect *rootRect;
    vrect newRect;
    int x, y;
    int defaultWidth;
    vcursor *cursor;

    /*
     * set rect appropriately
     */

#ifdef WANT_RELATIVE_TO_ITEM

    /** if this is to work, we'd need to put driver->pointerObject back in **/
    objectRect = *((vrect*)driver->GetPointerObject()
                                 ->GetAttribute(vname_Rect));

    /* translate objectRect into window coordinate system */
    if (driver->GetPointerObject()->IsKindOf(vwindow::StaticGetKind()))
        /* relative to the window, the window's own rect is at (0, 0) */
        objectRect.x = objectRect.y = 0;
    else
        driver->GetPointerObject()->GetItemList()->TransformRect(&objectRect,
                                                                 &objectRect);

#else

    vwindow::SyncEvents();
    dialog->GetPointer(&x, &y);
    objectRect.x = x;
    objectRect.y = y;

    cursor = vapplication::GetCurrent()->DetermineCursor();
    if (cursor != NULL)
    {
        objectRect.w = cursor->GetWidth();
        objectRect.h = cursor->GetHeight();
        objectRect.x -= cursor->GetHotX();
        objectRect.y -= cursor->GetHotY();
    }
    else
    {
        /* assume default size with hot spot at top left */
#ifdef vlafWINDOWS
        if (vlaf::Appear() == vlafWINDOWS)
            defaultWidth = 21;
        else
#endif
            defaultWidth = 15;
        objectRect.w = objectRect.h = defaultWidth;
        objectRect.y -= defaultWidth;
    }

#endif

    parentRect = dialog->GetRect();
    newRect = *GetRect();
    rootRect = vwindow::GetRoot()->GetRect();

#ifdef WANT_CENTERED
    newRect.x = parentRect->x + (objectRect.x + (objectRect.w / 2))
                 - (newRect.w / 2);
    newRect.y = parentRect->y
                 + (objectRect.y - newRect.h);
#else /* WANT_LEFT_JUSTIFIED */
    newRect.x = parentRect->x + objectRect.x;
    newRect.y = parentRect->y
                 + (objectRect.y - newRect.h);
#endif

    if (newRect.y < rootRect->y)
        newRect.y = (parentRect->y + (objectRect.y + objectRect.h));


   vfont* f = vdraw::GetFont();

   int w, h;
   f->StringWidth (text, &w, &h);
   newRect.y += 82;
   newRect.w = w+10;
   newRect.h = h+18;

    newRect.Pin(&newRect, rootRect);

    SetRect(&newRect);
    

    vwindow::Open();
}

void tipWindow::ResetTimer() {
   if(m_Timer.IsActive()) m_Timer.Stop();
   m_Timer.SetPeriod( 0, 300000000 ); //0.45 sec
   m_Timer.SetRecurrent();
   m_Timer.SetObserveTimerProc( tipWindow::TipTimerObserveProc );
   m_nTimerCount = 0;
}

void tipWindow::SetContainer(vcontainer* container) {
	// if container changed, we will reset timer and next tooltip will be displayed later.
	if(m_oldContainer!=container) {
		ResetTimer();
        m_Timer.Start();
		m_oldContainer=container;
	}
}


void tipWindow::Display (vdialogItem * i, vdialog* d , const vchar* text) {
    m_PendingDialog = NULL;
    m_PendingItem   = NULL;
	// close tipwindow wich was previously opened
    if(tipwin) {
	   tipwin->Close();
       delete tipwin;
       tipwin = NULL;
	   m_nTimerCount=5;
    } else if(m_Timer.IsActive()) {
		 ResetTimer();
	     m_Timer.Start();
	}
// tew tip window will be opened by the timer observer
    if (text && i && i->IsKindOf (&ToolBarButton::Kind)) {
       m_PendingDialog = d; 
       m_PendingText = text;
       m_PendingItem = i;
	   if(!m_Timer.IsActive()) m_Timer.Start();
	   TipTimerObserveProc(&m_Timer);
    }
}

void tipWindow::Display (gdDrawingArea *gd, vdialog* d, int cell, int dx, int dy, const vchar* text)
{
    m_nTimerCount   = 0;
    m_PendingDialog = NULL;
    m_PendingItem   = NULL;
    m_PendingCell   = -1;
    diff_x          = dx;
    diff_y          = dy;
    if( m_Timer.IsActive() ){
	m_Timer.Stop();
    }

    if (tipwin) {
	tipwin->Close();
	delete tipwin;
	tipwin        = NULL;
	m_nTimerCount = 4;
    }

    if (text){
	//Start the timer, tooltip will be displayed when m_nTimerCount is 5
	if( !m_Timer.IsActive() ){
	    m_Timer.SetPeriod( 0, 150000000 ); //0.15 sec
	    m_Timer.SetRecurrent();
	    m_Timer.SetObserveTimerProc( tipWindow::TipTimerObserveProcDrawing );
	    m_Timer.Start();
	}
	m_PendingDialog = d; 
	   m_PendingText   = text;
	   m_PendingCell   = cell;
	   m_PendingArea   = gd;
    }
}


/*
 * Draw(): Draw a tip window.
 */

void tipWindow::Draw(void) {
    const vrect  *windowRect;
    vrect         drawRect;

    vwindow::Draw();

    windowRect = GetRect();
    drawRect.Set(0, 0, windowRect->w, windowRect->h);

    vdraw::GSave();

    //vdraw::SetFont(driver->DetermineFont());
    vdraw::SetColor(tipWindow::DefaultBackground);
    vdraw::RectFill(0, 0, windowRect->w, windowRect->h);
    vdraw::SetColor(tipWindow::DefaultForeground);
    vdraw::SetLineWidth(0);
    vdraw::RectsStrokeInside(&drawRect, 1);

    vrectInset(&drawRect, INSET, INSET, &drawRect);

#if 0
    if (tip->icon != NULL)
    {
        int dx;

        vdraw::MoveTo((drawRect.x + INSET),
                      (drawRect.y + INSET));
        vdraw::ImageCompositeIdent(tip->icon);

        dx = tip->icon->GetWidth() + (INSET * 2);
        drawRect.x += dx;
        drawRect.w -= dx;
    }
#endif

    vdraw::ScribedRectShowCenter(vcharScribe(text), &drawRect);

    vdraw::GRestore();
}


/*
 * Startup(): Initialize class variables.
 */

void tipWindow::Startup(void)
{
    vcolorSpec spec;

    /* overridden class variables */

    spec.FSetRGB(1.0, 1.0, 0.5);  /* yellow */
    tipWindow::DefaultBackground = vcolor::Intern(&spec);
    tipWindow::DefaultForeground = vcolor::GetBlack();
}

void tipWindow::TipTimerObserveProc( vtimer * ) {
  if( m_nTimerCount == 5 && m_PendingDialog && m_PendingItem )  {
     int x, y;
     //Find x and y for a dialog item relative to its parent
     m_PendingDialog->GetPointer( &x, &y );
     vcontainer *parent = NULL;
     vdialogItemList* dil = m_PendingItem->GetItemListIn ();
     if ( dil ) parent = vcontainer::CastDown (dil->GetOwner());
     while( parent ) {
       const vrect *pRect = parent->GetBounds();
       x -= pRect->x;
       y -= pRect->y;
       dil = parent->GetItemListIn ();
       if ( dil ) parent = vcontainer::CastDown (dil->GetOwner());
       else       parent = NULL;
     }
     if( m_PendingItem->ContainsPoint(x,y) ) {
       tipwin = new tipWindow;
       tipwin->SetDialog (m_PendingDialog);
       tipwin->SetText (m_PendingText);
       tipwin->Open();
     }
  } else {
	if( m_nTimerCount > 5 && m_nTimerCount < 55 && tipwin ) {
       MyDialog *pDlg = (MyDialog *)tipwin->dialog;
       if( pDlg ) pDlg->DisplayTip();
    } else {
	    if( m_nTimerCount >= 55 && tipwin ) {
            if(m_Timer.IsActive()) m_Timer.Stop();
	        tipwin->Close();
            delete tipwin;
            tipwin = NULL;
            m_PendingDialog = NULL;
            m_PendingItem = NULL;
            m_nTimerCount = 0;
        }
	}
  }
  m_nTimerCount++;
}

void tipWindow::TipTimerObserveProcDrawing( vtimer * )
{
    if(m_nTimerCount >= 5 && m_PendingDialog && m_PendingCell >= 0){
	vpoint pt;
	int    x, y;
	//Find x and y for a dialog item relative to its parent
	m_PendingDialog->GetPointer( &x, &y );
	pt.x = x + diff_x;
	pt.y = y + diff_y;
	m_PendingArea->TranslatePoint(&pt, vdomainviewTO_DOMAIN);
 	int row, col, cell;
	m_PendingArea->FindRowColumnCell(pt, row, col, cell);
	if(m_PendingCell == cell){
	    if(!tipwin){
		tipwin = new tipWindow;
		tipwin->SetDialog (m_PendingDialog);
		tipwin->SetText (m_PendingText);
		tipwin->Open();
	    }
	} else {
	    if(tipwin){
		if( m_Timer.IsActive() )
		    m_Timer.Stop();
		tipwin->Close();
		delete tipwin;
		tipwin          = NULL;
		m_PendingDialog = NULL;
		m_PendingCell   = NULL;
		m_nTimerCount   = 0;
		
	    }
	}
    }  else if( m_nTimerCount >= 55 && tipwin ) { 
	if( m_Timer.IsActive() )
	    m_Timer.Stop();
	tipwin->Close();
	delete tipwin;
	tipwin          = NULL;
	m_PendingDialog = NULL;
	m_PendingCell   = -1;
	m_nTimerCount   = 0;
    }
    m_nTimerCount++;
}

vkindTYPED_FULL_DEFINITION(tipWindow, vwindow, "tipWindow");
