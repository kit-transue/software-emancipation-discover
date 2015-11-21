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
// ImpactChildFrame.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "MainFrm.h"
#include "ImpactChildFrame.h"
#include "IMAttachDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImpactChildFrame

IMPLEMENT_DYNCREATE(CImpactChildFrame, CMDIChildWnd)

CImpactChildFrame::CImpactChildFrame()
{
	m_pwndImpact = NULL;
	SetTitle("Impact");
}

CImpactChildFrame::~CImpactChildFrame()
{
}


BEGIN_MESSAGE_MAP(CImpactChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CImpactChildFrame)
	ON_WM_MDIACTIVATE()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_SERVICE_DISCONNECT,OnServiceDisconnect)
	ON_COMMAND(IDC_SERVICECHANGED,OnServiceChanged)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(IDC_SAVEREPORT, OnUpdateSaveReport)
	ON_COMMAND(IDC_SAVEREPORT, OnSaveReport)
	ON_COMMAND(IDC_IMATTACHMENT, OnIMAttach)
	ON_UPDATE_COMMAND_UI(IDC_IMATTACHMENT, OnUpdateIMAttach)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImpactChildFrame message handlers

void CImpactChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd) 
{		
	if(bActivate==TRUE)
		((CMainFrame *)GetMDIFrame())->SetNewMenu(IDR_IMPACT);	

	GetMDIFrame()->ShowControlBar(&m_ImpactTools,bActivate,TRUE);
	GetMDIFrame()->RecalcLayout();
	GetMDIFrame()->DrawMenuBar();	
	CMDIChildWnd::OnMDIActivate(bActivate,pActivateWnd,pDeactivateWnd);
}

BOOL CImpactChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	if(m_pwndImpact==NULL) {
		CRuntimeClass* pRuntimeClass = RUNTIME_CLASS(CImpactForm);
		CObject* pObject = pRuntimeClass->CreateObject();
		ASSERT( pObject->IsKindOf(RUNTIME_CLASS(CImpactForm)));
		m_pwndImpact=(CImpactForm *) pObject;
		CRect rcClient; 
		GetClientRect(rcClient);
		m_pwndImpact->Create(this,rcClient);
		m_pwndImpact->ShowWindow(SW_SHOW);
	}
	return CMDIChildWnd::OnCreateClient(lpcs, pContext);
}

void CImpactChildFrame::OnClose() 
{
	CMainFrame* main = (CMainFrame*)AfxGetMainWnd();
	main->m_wndImpact=NULL;
	main->SetNewMenu(IDR_MAINFRAME);		
	CMDIChildWnd::OnClose();
}

void CImpactChildFrame::OnSize(UINT nType, int cx, int cy) 
{
	CMDIChildWnd::OnSize(nType, cx, cy);
		
	if(m_pwndImpact!=NULL) {
		CRect rcClient; 
		GetClientRect(rcClient);
		m_pwndImpact->MoveWindow(&rcClient);
	}
}

BOOL CImpactChildFrame::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}

bool CImpactChildFrame::Update()
{
	if(m_pwndImpact!=NULL) 
		return m_pwndImpact->Update();
	return false;
}

LRESULT CImpactChildFrame::OnServiceDisconnect(WPARAM wparam, LPARAM lparam) {
	return TRUE;
}

void CImpactChildFrame::OnServiceChanged(void)  {
   Update();
}

int CImpactChildFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_ImpactTools.Create(GetMDIFrame()) ||
		!m_ImpactTools.LoadToolBar(IDR_IMPACT)) {
			TRACE0("Failed to create task flow toolbar\n");
	}

	m_ImpactTools.SetBarStyle(m_ImpactTools.GetBarStyle() |
							CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_ImpactTools.EnableDocking(CBRS_ALIGN_ANY);

	m_ImpactTools.EnableDocking(CBRS_ALIGN_ANY);
	GetMDIFrame()->DockControlBar(&m_ImpactTools);
	GetMDIFrame()->RecalcLayout();
	
	return 0;
}

void CImpactChildFrame::OnUpdateSaveReport(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_pwndImpact->IsReportView());
}

void CImpactChildFrame::OnSaveReport() 
{
	CFileDialog saveDlg(FALSE,"htm","Impact Analysis Report",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					"HTML files (*.htm) |*.htm ||",
					this);
    CString to;
    if(saveDlg.DoModal()==IDOK) {
	    to = saveDlg.GetPathName();
        m_pwndImpact->SaveReportTo(to);
    }	
}

void CImpactChildFrame::OnIMAttach() 
{
	CString szReport = m_pwndImpact->GetReportName();
	if(!szReport.IsEmpty()) {
		CIMAttachDlg dlg;

		if(dlg.DoModal()==IDOK) {
			dlg.AttachFileToIMIssue(szReport);
		}
	}
}

void CImpactChildFrame::OnUpdateIMAttach(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_pwndImpact->IsReportView());
}

void CImpactChildFrame::SetEditorCtrl(CCodeRoverIntegrator* pEditorCtrl)
{
	m_pwndImpact->SetEditorCtrl(pEditorCtrl);
}
