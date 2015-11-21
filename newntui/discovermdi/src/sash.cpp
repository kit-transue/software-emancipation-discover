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
// Sash.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "Sash.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSash

CSash::CSash() {
	m_OnMove=FALSE;
    m_MessageID=WM_SASHMOVED;
}

CSash::~CSash()
{
}


BEGIN_MESSAGE_MAP(CSash, CStatic)
	//{{AFX_MSG_MAP(CSash)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSash message handlers

void CSash::OnLButtonDown(UINT nFlags, CPoint point) {
	m_OnMove=TRUE;
	SetCapture();

	CWnd* borwser = this;
	CRect frameRect,browserRect;
    m_ContextWindow->GetWindowRect(&frameRect);
    borwser->GetWindowRect(&browserRect);
    CWindowDC dc(m_ContextWindow);
	CPen pen(PS_DOT,1,RGB(255,255,255));
	dc.SelectObject(&pen);
	int oldMode = dc.SetROP2(R2_XORPEN);
	if(browserRect.Height()>browserRect.Width()) {
        m_SashRect=CRect((browserRect.left-frameRect.left)+point.x-3,
			             (browserRect.top-frameRect.top),
					     (browserRect.left-frameRect.left)+point.x+3,
			             (browserRect.top-frameRect.top)  +browserRect.Height());

		dc.MoveTo(m_SashRect.left,m_SashRect.top);
		dc.LineTo(m_SashRect.left,m_SashRect.bottom);

		dc.MoveTo(m_SashRect.left+2,m_SashRect.top);
		dc.LineTo(m_SashRect.left+2,m_SashRect.bottom);

		dc.MoveTo(m_SashRect.left+4,m_SashRect.top);
		dc.LineTo(m_SashRect.left+4,m_SashRect.bottom);


	} else {

		m_SashRect=CRect((browserRect.left-frameRect.left),
			             (browserRect.top-frameRect.top)+point.y-3,
					     (browserRect.left-frameRect.left)+browserRect.Width(),
					     (browserRect.top-frameRect.top)  +point.y+3);

		dc.MoveTo(m_SashRect.left,m_SashRect.top);
		dc.LineTo(m_SashRect.right,m_SashRect.top);
		dc.MoveTo(m_SashRect.left,m_SashRect.top+2);
		dc.LineTo(m_SashRect.right,m_SashRect.top+2);
		dc.MoveTo(m_SashRect.left,m_SashRect.top+4);
		dc.LineTo(m_SashRect.right,m_SashRect.top+4);
	}
	dc.SetROP2(oldMode);
}

void CSash::OnLButtonUp(UINT nFlags, CPoint point)  {
	if(m_OnMove==TRUE) {
		ReleaseCapture();
		m_OnMove=FALSE;
	    CWnd* borwser = this;
	    CRect frameRect,browserRect;
        m_ContextWindow->GetWindowRect(&frameRect);
        borwser->GetWindowRect(&browserRect);

		borwser->GetWindowRect(&browserRect);
        if( (browserRect.left-frameRect.left)+point.x<=5 ) 
			   point.x=5-(browserRect.left-frameRect.left);
		if( (browserRect.left-frameRect.left)+point.x>=(frameRect.Width()-8) )
			   point.x=(frameRect.Width()-8)-(browserRect.left-frameRect.left);


        CWindowDC dc(m_ContextWindow);
	    CPen pen(PS_DOT,1,RGB(255,255,255));
	    dc.SelectObject(&pen);
	    int oldMode = dc.SetROP2(R2_XORPEN);
	    if(browserRect.Height()>browserRect.Width()) {

		    dc.MoveTo(m_SashRect.left,m_SashRect.top);
		    dc.LineTo(m_SashRect.left,m_SashRect.bottom);

		    dc.MoveTo(m_SashRect.left+2,m_SashRect.top);
		    dc.LineTo(m_SashRect.left+2,m_SashRect.bottom);

		    dc.MoveTo(m_SashRect.left+4,m_SashRect.top);
		    dc.LineTo(m_SashRect.left+4,m_SashRect.bottom);


		} else {

		    dc.MoveTo(m_SashRect.left,m_SashRect.top);
		    dc.LineTo(m_SashRect.right,m_SashRect.top);
		    dc.MoveTo(m_SashRect.left,m_SashRect.top+2);
		    dc.LineTo(m_SashRect.right,m_SashRect.top+2);
		    dc.MoveTo(m_SashRect.left,m_SashRect.top+4);
		    dc.LineTo(m_SashRect.right,m_SashRect.top+4);
		}
	    dc.SetROP2(oldMode);
	    GetParent()->SendMessage(m_MessageID,point.x,point.y);
	} 
	
}

void CSash::OnMouseMove(UINT nFlags, CPoint point) {

	if(m_OnMove==TRUE) {
		CWnd* borwser = this;
		CRect frameRect,browserRect;
        m_ContextWindow->GetWindowRect(&frameRect);
		borwser->GetWindowRect(&browserRect);

        if( (browserRect.left-frameRect.left)+point.x<=8 ) 
			   point.x=5-(browserRect.left-frameRect.left);
		if( (browserRect.left-frameRect.left)+point.x>=(frameRect.Width()-8) )
			   point.x=(frameRect.Width()-5)-(browserRect.left-frameRect.left);   

		CWindowDC dc(m_ContextWindow);
		CPen pen(PS_DOT,1,RGB(255,255,255));
		dc.SelectObject(&pen);
		int oldMode = dc.SetROP2(R2_XORPEN);
		if(browserRect.Height()>browserRect.Width()) {
         
		   dc.MoveTo(m_SashRect.left,m_SashRect.top);
		   dc.LineTo(m_SashRect.left,m_SashRect.bottom);

		   dc.MoveTo(m_SashRect.left+2,m_SashRect.top);
		   dc.LineTo(m_SashRect.left+2,m_SashRect.bottom);

		   dc.MoveTo(m_SashRect.left+4,m_SashRect.top);
		   dc.LineTo(m_SashRect.left+4,m_SashRect.bottom);

		   m_SashRect=CRect((browserRect.left-frameRect.left)+point.x-3,
			                (browserRect.top-frameRect.top),
							(browserRect.left-frameRect.left)+point.x+3,
			                (browserRect.top-frameRect.top)  +browserRect.Height());

		   dc.MoveTo(m_SashRect.left,m_SashRect.top);
		   dc.LineTo(m_SashRect.left,m_SashRect.bottom);

		   dc.MoveTo(m_SashRect.left+2,m_SashRect.top);
		   dc.LineTo(m_SashRect.left+2,m_SashRect.bottom);

		   dc.MoveTo(m_SashRect.left+4,m_SashRect.top);
		   dc.LineTo(m_SashRect.left+4,m_SashRect.bottom);


		} else {

		   dc.MoveTo(m_SashRect.left,m_SashRect.top);
		   dc.LineTo(m_SashRect.right,m_SashRect.top);
		   dc.MoveTo(m_SashRect.left,m_SashRect.top+2);
		   dc.LineTo(m_SashRect.right,m_SashRect.top+2);
		   dc.MoveTo(m_SashRect.left,m_SashRect.top+4);
		   dc.LineTo(m_SashRect.right,m_SashRect.top+4);

		   m_SashRect=CRect((browserRect.left-frameRect.left),
			                (browserRect.top-frameRect.top)+point.y-3,
							(browserRect.left-frameRect.left)+browserRect.Width(),
							(browserRect.top-frameRect.top)  +point.y+3);

		   dc.MoveTo(m_SashRect.left,m_SashRect.top);
		   dc.LineTo(m_SashRect.right,m_SashRect.top);
		   dc.MoveTo(m_SashRect.left,m_SashRect.top+2);
		   dc.LineTo(m_SashRect.right,m_SashRect.top+2);
		   dc.MoveTo(m_SashRect.left,m_SashRect.top+4);
		   dc.LineTo(m_SashRect.right,m_SashRect.top+4);
		}
		dc.SetROP2(oldMode);
	}
    CStatic::OnMouseMove(nFlags,point);
}




BOOL CSash::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)  {
CRect winRect;
HCURSOR cursor;

	BOOL result = CStatic::OnSetCursor(pWnd, nHitTest, message);
    GetWindowRect(&winRect);
	if(winRect.Height()>winRect.Width())
		cursor = ::LoadCursor(NULL,IDC_SIZEWE );
	else
        cursor = ::LoadCursor(NULL,IDC_SIZENS);
	::SetCursor(cursor);
	return result;
	
}

void CSash::SetContextWindow(CWnd * contextWin) {
	m_ContextWindow=contextWin;

}


void CSash::SetNotificationMessage( UINT message) {
	m_MessageID = message;
}
