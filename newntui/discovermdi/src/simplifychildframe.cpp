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
// SimplifyChildFrame.cpp : implementation file
//
#include <afxrich.h>
#include "stdafx.h"
#include "discovermdi.h"
#include "SimplifyChildFrame.h"
#include "SimplifyTree.h"
#include "SimplifySelection.h"
#include "SimplifyText.h"
#include "mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimplifyChildFrame

IMPLEMENT_DYNCREATE(CSimplifyChildFrame, CMDIChildWnd)

CSimplifyChildFrame::CSimplifyChildFrame()
{
}

CSimplifyChildFrame::~CSimplifyChildFrame()
{
}


BEGIN_MESSAGE_MAP(CSimplifyChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CSimplifyChildFrame)
	ON_WM_MDIACTIVATE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_INCLUSION, OnInclusion)
	ON_UPDATE_COMMAND_UI(ID_INCLUSION, OnUpdateInclusion)
	ON_WM_CREATE()
	ON_COMMAND(ID_SIMPLIFY_UPDATE, OnSimplifyUpdate)
	ON_MESSAGE(WM_SERVICE_DISCONNECT,OnServiceDisconnect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimplifyChildFrame message handlers

BOOL CSimplifyChildFrame::PreCreateWindow(CREATESTRUCT& cs)  {
	cs.style|=(WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	return CMDIChildWnd::PreCreateWindow(cs);
}

void CSimplifyChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)  {
	
   if(bActivate==TRUE) {
         ((CMainFrame *)GetMDIFrame())->SetNewMenu(IDR_SIMPLIFY);	
   }
   GetMDIFrame()->ShowControlBar(&m_SimplifyTools,bActivate,TRUE);
   GetMDIFrame()->RecalcLayout();
   GetMDIFrame()->DrawMenuBar();	
	
}

BOOL CSimplifyChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)  {
CRect winRect;

   GetClientRect(winRect);
   CSize treeSize(winRect.Width()/2,winRect.Height());
   CSize selectionSize(winRect.Width()/2,winRect.Height()/2);
   CSize editSize(winRect.Width()/2,winRect.Height()/2);

   if (!m_wndSplitter.CreateStatic(this,1,2)) {
      TRACE0("Failed to create split bar ");
      return FALSE;
   } 

   if (!m_wndNestedSplitter.CreateStatic(&m_wndSplitter,2,1,WS_CHILD | WS_VISIBLE | WS_BORDER,m_wndSplitter.IdFromRowCol(0, 1)) ) {
      TRACE0("Failed to create split bar ");
      return FALSE;
   } 

   if(m_wndNestedSplitter.CreateView(0,0,RUNTIME_CLASS(CSimplifySelection),selectionSize,pContext)==FALSE) return FALSE;
   m_Selection = (CSimplifySelection *) m_wndNestedSplitter.GetPane(0,0);
   if(m_Selection->IsEmpty()) {
		MessageBox("No C/C++ module(s) has been selected. Nothing to simplify.","Warning",MB_OK|MB_ICONWARNING);
		return FALSE;
   }
   if(m_wndSplitter.CreateView(0,0,RUNTIME_CLASS(CSimplifyTree),treeSize,pContext)==FALSE) return FALSE;
   m_TreeView = (CSimplifyTree *) m_wndSplitter.GetPane(0,0);
   if(!m_TreeView->IsLicensed()) {
		MessageBox("No Simplify license is available.","Error",MB_OK|MB_ICONERROR);
		return FALSE;
   }

   if(m_wndNestedSplitter.CreateView(1,0,RUNTIME_CLASS(CSimplifyText),editSize, pContext)==FALSE) return FALSE;
   m_Editor = (CSimplifyText *)m_wndNestedSplitter.GetPane(1,0);

   CMainFrame* main = (CMainFrame*)AfxGetMainWnd();
   CString parserLogFile;
   parserLogFile=main->m_wndBrowser.ExecuteCommand(CString("els_tmpname simplify"));
   parserLogFile+=".err";
   main->ShowControlBar( &main->m_wndErrors, TRUE, FALSE);	
   main->m_wndErrors.StartFileLog(parserLogFile);
   return TRUE;

}

LRESULT CSimplifyChildFrame::OnServiceDisconnect(WPARAM wparam, LPARAM lparam) {
	SendMessage(WM_CLOSE);
	return TRUE;
}

void CSimplifyChildFrame::OnClose() 
{
   CMainFrame* main = (CMainFrame*)AfxGetMainWnd();
   main->m_wndErrors.StopFileLog();
   m_TreeView->Close();
   main->m_wndSimplify=NULL;
   main->SetNewMenu(IDR_MAINFRAME);	
   CMDIChildWnd::OnClose();
}

void CSimplifyChildFrame::OnInclusion()  {
CSimplifyTree* tree = (CSimplifyTree *) m_wndSplitter.GetPane(0,0);

      tree->EnableInclusionMode(!tree->GetInclusionMode());
	
}

void CSimplifyChildFrame::OnUpdateInclusion(CCmdUI* pCmdUI)  {
CSimplifyTree* tree = (CSimplifyTree *) m_wndSplitter.GetPane(0,0);
	pCmdUI->SetCheck(tree->GetInclusionMode());
}

int CSimplifyChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)  {
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
     if (!m_SimplifyTools.Create(GetMDIFrame()) ||
	     !m_SimplifyTools.LoadToolBar(MAKEINTRESOURCE(IDR_SIMPLIFY))) {
	    TRACE0("Failed to create submission check toolbar\n");
	 }
     m_SimplifyTools.SetBarStyle(m_SimplifyTools.GetBarStyle() |
		  CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC | CBRS_FLOAT_MULTI );

     m_SimplifyTools.EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM);
     GetMDIFrame()->DockControlBar(&m_SimplifyTools,AFX_IDW_DOCKBAR_TOP);
     GetMDIFrame()->RecalcLayout();
     return 0;
}

void CSimplifyChildFrame::OnSimplifyUpdate()  {
CSimplifyTree* tree = (CSimplifyTree *) m_wndSplitter.GetPane(0,0);
      tree->Done();
}
