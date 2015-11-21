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
// HtmlCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "impact.h"
#include "HtmlCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHtmlCtrl

IMPLEMENT_DYNCREATE(CHtmlCtrl, CHtmlView)

CHtmlCtrl::CHtmlCtrl()
{
	//{{AFX_DATA_INIT(CHtmlCtrl)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CHtmlCtrl::~CHtmlCtrl()
{
}

void CHtmlCtrl::DoDataExchange(CDataExchange* pDX)
{
	CHtmlView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHtmlCtrl)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHtmlCtrl, CHtmlView)
	//{{AFX_MSG_MAP(CHtmlCtrl)
	ON_WM_DESTROY()
	ON_WM_MOUSEACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHtmlCtrl diagnostics

#ifdef _DEBUG
void CHtmlCtrl::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CHtmlCtrl::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CHtmlCtrl message handlers

BOOL CHtmlCtrl::CreateFromStatic(UINT nID, CWnd *pParent)
{
	CStatic wndStatic;
    if (!wndStatic.SubclassDlgItem(nID, pParent))
       return FALSE;
 
    // Get static control rect, convert to parent's client coords.
    CRect rc;
    wndStatic.GetWindowRect(&rc);
    pParent->ScreenToClient(&rc);
    wndStatic.DestroyWindow();
 
    // create HTML control (CHtmlView)
    return CHtmlView::Create(NULL,                  // class name
       NULL,                             // title
       (WS_CHILD | WS_VISIBLE ),         // style
       rc,                               // rectangle
       pParent,                          // parent
       nID,                              // control ID
       NULL);                            // frame/doc context not used
}

void CHtmlCtrl::PostNcDestroy() 
{
	// do nothing
	
	// Normally, CHtmlView destroys itself in PostNcDestroy,
    // but we don't want to do that for a control since a control
    // is usually implemented as a stack object in a dialog.
}

void CHtmlCtrl::OnDestroy() 
{
	// This is probably unecessary since ~CHtmlView does it, but
    // safer to mimic CHtmlView::OnDestroy.
    if (m_pBrowserApp) {
       m_pBrowserApp->Release();
       m_pBrowserApp = NULL;
    }
    CWnd::OnDestroy(); // bypass CView doc/frame stuf	
}

int CHtmlCtrl::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message) 
{
	// bypass CView doc/frame stuff
    return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

BOOL CHtmlCtrl::Create(DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID)
{
	return CHtmlView::Create(NULL,                  // class name
				  NULL,                  // title
				  dwStyle,				 // style
				  rect,                  // rectangle
				  pParentWnd,            // parent
				  nID,                   // control ID
				  NULL);                 // frame/doc context not used
}
