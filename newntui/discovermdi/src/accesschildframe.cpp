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
// AccessChildFrame.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "AccessChildFrame.h"
#include "AccessWnd.h"
#include "MainFrm.h"
#include "GraphView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAccessChildFrame

IMPLEMENT_DYNCREATE(CAccessChildFrame, CMDIChildWnd)

CAccessChildFrame::CAccessChildFrame()
{
}

CAccessChildFrame::~CAccessChildFrame()
{
}


BEGIN_MESSAGE_MAP(CAccessChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CAccessChildFrame)
	ON_WM_CLOSE()
	ON_WM_MDIACTIVATE()
	ON_MESSAGE(WM_SERVICE_DISCONNECT,OnServiceDisconnect)
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAccessChildFrame message handlers

BOOL CAccessChildFrame::PreCreateWindow(CREATESTRUCT& cs)  {
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

BOOL CAccessChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)  {
static CCreateContext newContext;
static BOOL bCommDlgShown = FALSE;

	newContext.m_pNewViewClass=RUNTIME_CLASS(CAccessWnd);
    CAccessWnd* proc = (CAccessWnd *)CreateView(&newContext);
	m_AccessForm=proc;
	proc->OnInitialUpdate();
	if(m_AccessForm && ::IsWindow(*m_AccessForm)) {	
		  CString query;
		  BSTR sysQuery;
		  query="printformat \"%s \t %s\" name kind";
          sysQuery=query.AllocSysString();
		  ((CAccessWnd *)m_AccessForm)->m_DataSource.AccessSync(&sysQuery);
		  ::SysFreeString(sysQuery);
		  if (((CAccessWnd *)m_AccessForm)->m_DataSource.IsConnectionLost() == TRUE &&
			  bCommDlgShown != TRUE ) {
		 	  ::MessageBox(m_hWnd, 
					   _T("Connection with server is lost."
					   " Make sure the server is running."),
					   _T("Server Communication Error."), 
					   MB_OK | MB_ICONINFORMATION); 
			  bCommDlgShown = TRUE;
		  } else if (((CAccessWnd *)m_AccessForm)->m_DataSource.IsConnectionLost() != TRUE) {
			  bCommDlgShown = FALSE;
		  }	
	}
	return TRUE;


}

LRESULT CAccessChildFrame::OnServiceDisconnect(WPARAM wparam, LPARAM lparam) {
	return TRUE;
}

void CAccessChildFrame::OnClose()  {
    CMenu mainMenu;
    mainMenu.LoadMenu(IDR_MAINFRAME);
    ((CMainFrame *)GetMDIFrame())->m_wndAccess=NULL;	
    ((CMainFrame *)GetMDIFrame())->SetNewMenu(IDR_MAINFRAME);	
	CMDIChildWnd::OnClose();
}

void CAccessChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)  {
  if(bActivate==TRUE) {
      ((CMainFrame *)GetMDIFrame())->SetNewMenu(IDR_ACCESS);	
   }
   GetMDIFrame()->RecalcLayout();
   GetMDIFrame()->DrawMenuBar();	
}




//---------------------------------------------------------------------------------------------------
// We will set default printformat when this window receves focus
//---------------------------------------------------------------------------------------------------
void CAccessChildFrame::OnSetFocus(CWnd* pOldWnd)  {
	CMDIChildWnd::OnSetFocus(pOldWnd);
}
//---------------------------------------------------------------------------------------------------
