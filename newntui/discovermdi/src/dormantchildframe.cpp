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
// DormantChildFrame.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "DormantChildFrame.h"
#include "DormantProcessor.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDormantChildFrame

IMPLEMENT_DYNCREATE(CDormantChildFrame, CMDIChildWnd)

CDormantChildFrame::CDormantChildFrame() {
	m_DormantProcessor=NULL;
}

CDormantChildFrame::~CDormantChildFrame()
{
}


BEGIN_MESSAGE_MAP(CDormantChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CDormantChildFrame)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_SERVICE_DISCONNECT,OnServiceDisconnect)
	ON_COMMAND(IDC_SERVICECHANGED,OnServiceChanged)
	ON_WM_MDIACTIVATE()
	ON_MESSAGE(WM_UPDATEGROUPS,OnUpdateGroups)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDormantChildFrame message handlers

BOOL CDormantChildFrame::PreCreateWindow(CREATESTRUCT& cs)  {
	cs.style|=(WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	CRect rect;
	((CMainFrame *)AfxGetMainWnd())->NegotiateBorderSpace( 1, &rect);
	cs.x=0;
	cs.y=0;
	cs.cx=rect.Width();
	cs.cy=rect.Height();
	cs.cx=rect.Width()-5;
	cs.cy=rect.Height()-5;
	if(cs.cx<400) cs.cx=400;
	if(cs.cy<400) cs.cy=400;
	return CMDIChildWnd::PreCreateWindow(cs);
}

BOOL CDormantChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)  {
static CCreateContext newContext;
	newContext.m_pNewViewClass=RUNTIME_CLASS(CDormantProcessor);
    CDormantProcessor* proc = (CDormantProcessor *)CreateView(&newContext);
	m_DormantProcessor=proc;
	proc->OnInitialUpdate();
	return TRUE;
}

void CDormantChildFrame::OnClose()  {
    ((CMainFrame *)GetMDIFrame())->m_wndDormant=NULL;	
    ((CMainFrame *)GetMDIFrame())->SetNewMenu(IDR_MAINFRAME);	
	CMDIChildWnd::OnClose();
}

LRESULT CDormantChildFrame::OnServiceDisconnect(WPARAM wparam, LPARAM lparam) {
	return TRUE;
}

void CDormantChildFrame::OnServiceChanged(void)  {
   if(m_DormantProcessor!=NULL) ((CDormantProcessor *)m_DormantProcessor)->UpdateSelection();
}

void CDormantChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)  {
  if(bActivate==TRUE) {
      ((CMainFrame *)GetMDIFrame())->SetNewMenu(IDR_DORMANT);	
   }
   GetMDIFrame()->RecalcLayout();
   GetMDIFrame()->DrawMenuBar();	
	
}

bool CDormantChildFrame::Update()
{
   if(m_DormantProcessor!=NULL) ((CDormantProcessor *)m_DormantProcessor)->UpdateSelection();
   return m_DormantProcessor!=NULL;
}

LRESULT CDormantChildFrame::OnUpdateGroups(WPARAM wParam, LPARAM lParam) {
	GetParentFrame()->SendMessage(WM_UPDATEGROUPS,wParam,lParam);
	return 0;
}
