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
// IpFrame.cpp : implementation of the CInPlaceFrame class
//

#include "stdafx.h"
#include "DiscoverMDI.h"

#include "IpFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInPlaceFrame

IMPLEMENT_DYNCREATE(CInPlaceFrame, COleDocIPFrameWnd)

BEGIN_MESSAGE_MAP(CInPlaceFrame, COleDocIPFrameWnd)
	//{{AFX_MSG_MAP(CInPlaceFrame) 
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	// Global help commands
	// ON_COMMAND(ID_HELP_FINDER, COleDocIPFrameWnd::OnHelpFinder)
	// ON_COMMAND(ID_HELP, COleDocIPFrameWnd::OnHelp)
	// ON_COMMAND(ID_DEFAULT_HELP, COleDocIPFrameWnd::OnHelpFinder)
	// ON_COMMAND(ID_CONTEXT_HELP, COleDocIPFrameWnd::OnContextHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInPlaceFrame construction/destruction

CInPlaceFrame::CInPlaceFrame()
{
}

CInPlaceFrame::~CInPlaceFrame()
{
}

int CInPlaceFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (COleDocIPFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// CResizeBar implements in-place resizing.
	if (!m_wndResizeBar.Create(this))
	{
		TRACE0("Failed to create resize bar\n");
		return -1;      // fail to create
	}

	// By default, it is a good idea to register a drop-target that does
	//  nothing with your frame window.  This prevents drops from
	//  "falling through" to a container that supports drag-drop.
	m_dropTarget.Register(this);

	return 0;
}

// OnCreateControlBars is called by the framework to create control bars on the
//  container application's windows.  pWndFrame is the top level frame window of
//  the container and is always non-NULL.  pWndDoc is the doc level frame window
//  and will be NULL when the container is an SDI application.  A server
//  application can place MFC control bars on either window.
BOOL CInPlaceFrame::OnCreateControlBars(CFrameWnd* pWndFrame, CFrameWnd* pWndDoc)
{
	// Remove this if you use pWndDoc
	UNREFERENCED_PARAMETER(pWndDoc);

	// Set owner to this window, so messages are delivered to correct app
	//m_wndToolBar.SetOwner(this);
	//m_Browser.SetOwner(this);

	// Create toolbar on client's frame window
/*	if (!m_wndToolBar.Create(pWndFrame) ||
		!m_wndToolBar.LoadToolBar(IDR_DISCOVTYPE_SRVR_IP))
	{
		TRACE0("Failed to create toolbar\n");
		return FALSE;
	}*/

	/*if (!m_Browser.Create(pWndFrame, IDD_COMMONVIEW,
		CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC, IDD_COMMONVIEW))
	{
		TRACE0("Failed to create toolbar\n");
		return FALSE;
	}*/

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
/*	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);*/
	/*m_Browser.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);*/

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	//m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	//m_Browser.EnableDocking(CBRS_ALIGN_ANY);

	//pWndFrame->EnableDocking(CBRS_ALIGN_ANY);

	//pWndFrame->DockControlBar(&m_wndToolBar);
	//pWndFrame->DockControlBar(&m_Browser);

	return TRUE;
}

BOOL CInPlaceFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return COleDocIPFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CInPlaceFrame diagnostics

#ifdef _DEBUG
void CInPlaceFrame::AssertValid() const
{
	COleDocIPFrameWnd::AssertValid();
}

void CInPlaceFrame::Dump(CDumpContext& dc) const
{
	COleDocIPFrameWnd::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CInPlaceFrame commands



