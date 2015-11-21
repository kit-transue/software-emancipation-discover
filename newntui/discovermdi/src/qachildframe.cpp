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
// QAChildFrame.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "QAChildFrame.h"
#include "QATree.h"
#include "QAParams.h"
#include "MainFrm.h"
#include "IMAttachDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQAChildFrame

IMPLEMENT_DYNCREATE(CQAChildFrame, CMDIChildWnd)

CQAChildFrame::CQAChildFrame() {
	m_ResultSetExist=FALSE;
	m_ResultsReady = FALSE;
}

CQAChildFrame::~CQAChildFrame()
{
}


BEGIN_MESSAGE_MAP(CQAChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CQAChildFrame)
	ON_WM_CLOSE()
	ON_WM_CANCELMODE()
	ON_WM_MDIACTIVATE()
	ON_COMMAND(ID_QARESET, OnQAReset)
	ON_COMMAND(ID_QARUN, OnQARun)
	ON_COMMAND(ID_QARUNSEL, OnQARunSel)
	ON_UPDATE_COMMAND_UI(ID_QABROWSE, OnUpdateQaBrowse)
	ON_COMMAND(ID_QABROWSE, OnQaBrowse)
	ON_MESSAGE(WM_SERVICE_DISCONNECT,OnServiceDisconnect)
	ON_COMMAND(IDC_SERVICECHANGED,OnServiceChanged)
	ON_COMMAND(ID_QAOPEN, OnOpen)
	ON_COMMAND(ID_QASAVE, OnSave)
	ON_COMMAND(IDC_SAVEREPORT, OnSaveReport)
	ON_UPDATE_COMMAND_UI(IDC_SAVEREPORT, OnUpdateSavereport)
	ON_UPDATE_COMMAND_UI(ID_QASAVE, OnUpdateQasave)
	ON_COMMAND(IDC_IMATTACHMENT, OnIMAttach)
	ON_UPDATE_COMMAND_UI(IDC_IMATTACHMENT, OnUpdateIMAttach)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQAChildFrame message handlers

BOOL CQAChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)  {
CRect winRect;

   GetWindowRect(winRect);
   if (!m_wndSplitter.CreateStatic(this,1,2)) {
      TRACE0("Failed to create split bar ");
      return FALSE;
   } 
   CSize size(winRect.Width()/2,winRect.Height());
   if(m_wndSplitter.CreateView(0,0,RUNTIME_CLASS(CQATree),size,pContext)==FALSE) return FALSE;
   if(m_wndSplitter.CreateView(0,1,RUNTIME_CLASS(CQAParams),size, pContext)==FALSE) return FALSE;
   return TRUE;

}

BOOL CQAChildFrame::PreCreateWindow(CREATESTRUCT& cs)  {
	cs.style|=(WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	CRect rect;
	((CMainFrame *)AfxGetMainWnd())->NegotiateBorderSpace( 1, &rect);
	cs.x=0;
	cs.y=0;
	cs.cx=rect.Width();
	cs.cy=rect.Height();
	cs.cx=rect.Width()-5;
	cs.cy=rect.Height()-35;
	if(cs.cx<400) cs.cx=400;
	if(cs.cy<400) cs.cy=400;
	return CMDIChildWnd::PreCreateWindow(cs);
}

void CQAChildFrame::OnClose()  {
    ((CMainFrame *)GetMDIFrame())->m_wndQA=NULL;	
    ((CMainFrame *)GetMDIFrame())->SetNewMenu(IDR_MAINFRAME);	
	
	if(m_wndSplitter.IsWindowVisible()) {
		CQATree* p=(CQATree*)m_wndSplitter.GetPane(0,0);
		p->SendMessage(WM_CLOSE);
		CQAParams* pr=(CQAParams*)m_wndSplitter.GetPane(0,1);
		pr->SendMessage(WM_CLOSE);
	}
	CMDIChildWnd::OnClose();
}

void CQAChildFrame::OnCancelMode()  {
    ((CMainFrame *)GetMDIFrame())->SetNewMenu(IDR_MAINFRAME);	
    ((CMainFrame *)GetMDIFrame())->m_wndQA=NULL;	
	CMDIChildWnd::OnClose();
}

void CQAChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)  {

   if(bActivate==TRUE) {
      ((CMainFrame *)GetMDIFrame())->SetNewMenu(IDR_QA);	
      GetMDIFrame()->DrawMenuBar();
   }
   GetMDIFrame()->ShowControlBar(&m_QATools,bActivate,TRUE);
   GetMDIFrame()->RecalcLayout();
   GetMDIFrame()->DrawMenuBar();	
	
}


BOOL CQAChildFrame::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CMDIFrameWnd* pParentWnd, CCreateContext* pContext)  {
	
   if(CMDIChildWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, pContext)==-1) 
	   return -1;
   if (!m_QATools.Create(GetMDIFrame()) ||
	  !m_QATools.LoadToolBar(IDR_QA)) {
	   TRACE0("Failed to create task flow toolbar\n");
   }

   m_QATools.SetBarStyle(m_QATools.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
   m_QATools.EnableDocking(CBRS_ALIGN_ANY);

   m_QATools.EnableDocking(CBRS_ALIGN_ANY);
   GetMDIFrame()->DockControlBar(&m_QATools);
   GetMDIFrame()->RecalcLayout();
   return 0;
	
}

void CQAChildFrame::OnQAReset()  {
CQATree* p=(CQATree*)m_wndSplitter.GetPane(0,0);

    p->OnReset();
	m_ResultsReady = FALSE;
	
}

void CQAChildFrame::OnQARun()  {
CQATree* p=(CQATree*)m_wndSplitter.GetPane(0,0);
CQAParams* pr=(CQAParams*)m_wndSplitter.GetPane(0,1);

    p->OnRunAll();
	pr->UpdateReport();
	m_ResultsReady = TRUE;
	
}

void CQAChildFrame::OnQARunSel()  {
CQATree* p=(CQATree*)m_wndSplitter.GetPane(0,0);
CQAParams* pr=(CQAParams*)m_wndSplitter.GetPane(0,1);

    p->OnRunSelected();
	pr->UpdateReport();
	m_ResultsReady = TRUE;
	
}

void CQAChildFrame::OnUpdateQaBrowse(CCmdUI* pCmdUI)  {
CMainFrame* mainFrame;
mainFrame=(CMainFrame *)AfxGetMainWnd();

    pCmdUI->Enable(m_ResultSetExist && mainFrame->m_Lic[LIC_QABROWSE]>0);	
}

void CQAChildFrame::SetCurrentSetEnable(BOOL status) {
    m_ResultSetExist=status;
}

//---------------------------------------------------------------------------------------
// This callback will run when the user wants to browse all symbols from the current
// QA query results.
//---------------------------------------------------------------------------------------
void CQAChildFrame::OnQaBrowse()  {
CQAParams* p=(CQAParams*)m_wndSplitter.GetPane(0,1);
    p->GetCurrentSet(CString("qa_results"));
	CMainFrame* mainFrame;
	mainFrame=(CMainFrame *)AfxGetMainWnd();
	if(mainFrame!=NULL) {
		mainFrame->m_wndBrowser.BrowseGivenSet(CString("qa_results"));
	}
    p->FreeCurrentSet(CString("qa_results"));
}
//---------------------------------------------------------------------------------------

LRESULT CQAChildFrame::OnServiceDisconnect(WPARAM wparam, LPARAM lparam) {
	return TRUE;
}

void CQAChildFrame::OnServiceChanged(void)  {
CQATree* tree     = (CQATree*)m_wndSplitter.GetPane(0,0);
    tree->Update();
}

//---------------------------------------------------------------------------------------
// Callback runs when the user try to open saved QA session from menu or toolbar
//---------------------------------------------------------------------------------------
void CQAChildFrame::OnOpen()  {
CQATree* tree     = (CQATree*)m_wndSplitter.GetPane(0,0);
CQAParams* pr=(CQAParams*)m_wndSplitter.GetPane(0,1);

    if(tree->Open()==TRUE) {
	    tree->FillNewTree();
	    pr->UpdateReport();
        m_ResultsReady=TRUE;
	}
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Callback runs when the user try to save current QA session from menu or toolbar
//---------------------------------------------------------------------------------------
void CQAChildFrame::OnSave()  {
CQATree* tree     = (CQATree*)m_wndSplitter.GetPane(0,0);

    tree->Save();
	
}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
// Callback will run every time the user want to copy report from the PSETHOME/reports.qa
// to some specified location and name.
//---------------------------------------------------------------------------------------
void CQAChildFrame::OnSaveReport()  {
CFileDialog saveDlg(FALSE,"htm","Quality Analyser Report",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					"HTML files (*.htm) |*.htm |Text files (*.txt) |*.txt||",
					this);
CQATree* tree     = (CQATree*)m_wndSplitter.GetPane(0,0);

    CString to;
    if(saveDlg.DoModal()==IDOK) {
	    to = saveDlg.GetPathName();
		DeleteFile(to);
        tree->GenerateReport(to);
    }	

}
//---------------------------------------------------------------------------------------

void CQAChildFrame::OnUpdateSavereport(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_ResultsReady);	
	CQAParams* pr=(CQAParams*)m_wndSplitter.GetPane(0,1);
	pr->EnableReport(m_ResultsReady);
}

void CQAChildFrame::OnUpdateQasave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_ResultsReady);	
	CQAParams* pr=(CQAParams*)m_wndSplitter.GetPane(0,1);
	pr->EnableReport(m_ResultsReady);
}

void CQAChildFrame::OnIMAttach() 
{
	CQATree* tree = (CQATree*)m_wndSplitter.GetPane(0,0);
	CString szReport = tree->GetReportName();
	if(!szReport.IsEmpty()) {
		CIMAttachDlg dlg;

		if(dlg.DoModal()==IDOK) {
			if(dlg.AttachFileToIMIssue(szReport)) {
				CString szImageName;
				for(int i=0;i<7;i++) {
					szImageName.Format(_T("%s.chart%d.gif"),(LPCTSTR)szReport,i);
					if(!dlg.AttachFileToIMIssue(szImageName))
						MessageBox(CString("Can not attach file ")+szImageName,"Error",MB_OK|MB_ICONERROR);
				}
			} else
				MessageBox(CString("Can not attach file ")+szReport,"Error",MB_OK|MB_ICONERROR);
		}
	}
}

void CQAChildFrame::OnUpdateIMAttach(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_ResultsReady);	
}
