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
// DormantProcessor.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "DormantProcessor.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDormantProcessor

IMPLEMENT_DYNCREATE(CDormantProcessor, CFormView)

CDormantProcessor::CDormantProcessor()
	: CFormView(CDormantProcessor::IDD)
{
	//{{AFX_DATA_INIT(CDormantProcessor)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bLicensed = false;
}

CDormantProcessor::~CDormantProcessor()
{
}

void CDormantProcessor::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDormantProcessor)
	DDX_Control(pDX, IDC_RUNDORMANT, m_StartButton);
	DDX_Control(pDX, IDC_DORMANTTITLE, m_DormantTitle);
	DDX_Control(pDX, IDC_DATASOURCE, m_DataSource);
	DDX_Control(pDX, IDC_DORMANTSELECTION, m_SelectionList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDormantProcessor, CFormView)
	//{{AFX_MSG_MAP(CDormantProcessor)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_RUNDORMANT, OnRunDormant)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDormantProcessor diagnostics

#ifdef _DEBUG
void CDormantProcessor::AssertValid() const
{
	CFormView::AssertValid();
}

void CDormantProcessor::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDormantProcessor message handlers

void CDormantProcessor::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	
    CString  query="license v co";
    BSTR sysQuery=query.AllocSysString();
    CString res = m_DataSource.AccessSync(&sysQuery);
	SysFreeString(sysQuery);
	if(res!="1") {
		MessageBox("No Dormant license is available.","Error",MB_OK|MB_ICONERROR);
		CFrameWnd* pParent = this->GetParentFrame();
		pParent->PostMessage(WM_CLOSE);
	} else
		m_bLicensed = true;

	if(m_Updated!=TRUE) {
		SIZE sizeTotal;
		sizeTotal.cx=150;
		sizeTotal.cy=150;
		SetScrollSizes( MM_TEXT, sizeTotal);

		m_Updated=TRUE;	
		LPDISPATCH lpDispatch;
		m_DataSource.GetControlUnknown()->QueryInterface(IID_IDispatch,(void **)&lpDispatch);
		m_SelectionList.SetDataSource(lpDispatch);
		CRect w;
		GetClientRect(w);
		int cx=w.Width();
		int cy=w.Height();
		if(cx<150) cx=150;
		if(cy<150) cy=150;
		if(::IsWindow(m_DormantTitle)) {
			CRect titleRect;
			CRect startRect;
			m_DormantTitle.GetWindowRect(&titleRect);
			m_StartButton.GetWindowRect(&startRect);
			int ypos=5;
			m_DormantTitle.MoveWindow(5,ypos,cx-5,titleRect.Height());
			ypos+=titleRect.Height()+5;
			m_SelectionList.MoveWindow(0,ypos,cx,cy-titleRect.Height()-startRect.Height()-20);
			ypos+=cy-titleRect.Height()-startRect.Height()-15;
			m_StartButton.MoveWindow((cx-startRect.Width()-10)/2,ypos,startRect.Width(),startRect.Height());
		}
		UpdateSelection();
	}
}

int CDormantProcessor::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_Updated=FALSE;

	return 0;
}

void CDormantProcessor::UpdateSelection() {
CString query;
BSTR sysQuery;

    // Set Dormant mode selection variable
    query="set DormantSelection [nil_set]; sappend $DormantSelection $DiscoverSelection";
    sysQuery=query.AllocSysString();
    m_DataSource.AccessOutput(&sysQuery);
	SysFreeString(sysQuery);

	m_SelectionList.SetFullListName("DormantSelection");
	m_SelectionList.SetFilterListName("DormantSelection_F");
	m_SelectionList.SetSelectionName("DormantSelection_S");
	m_SelectionList.SetQuery("");
	m_SelectionList.Update();
}

void CDormantProcessor::OnRunDormant()  {
CString query;
BSTR sysQuery;

    // Set Dormant mode selection variable
    query="RunDormantCodeAnalysis";
    sysQuery=query.AllocSysString();
    m_DataSource.AccessOutput(&sysQuery);
	SysFreeString(sysQuery);

	// Notifying user that the executinp process is finished.
	MessageBox("Dormant code analysis completed.\nUse group manager to browse results.","Dormant",MB_OK | MB_ICONINFORMATION);
	GetParent()->SendMessage(WM_UPDATEGROUPS,0,0);
}

void CDormantProcessor::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	if(cx<150) cx=150;
	if(cy<150) cy=150;

	CRect titleRect;
	CRect startRect;

	if(::IsWindow(m_DormantTitle)) {
	   m_DormantTitle.GetWindowRect(&titleRect);
	   m_StartButton.GetWindowRect(&startRect);
	   int ypos=5;
	   m_DormantTitle.MoveWindow(5,ypos,cx-5,titleRect.Height());
	   ypos+=titleRect.Height()+5;
	   m_SelectionList.MoveWindow(0,ypos,cx,cy-titleRect.Height()-startRect.Height()-20);
	   ypos+=cy-titleRect.Height()-startRect.Height()-15;
	   m_StartButton.MoveWindow((cx-startRect.Width()-10)/2,ypos,startRect.Width(),startRect.Height());
	}
}

void CDormantProcessor::OnDestroy() 
{
	if(m_bLicensed) {
		CString  query="license v ci";
		BSTR sysQuery=query.AllocSysString();
		m_DataSource.AccessSync(&sysQuery);
		SysFreeString(sysQuery);
	}

	CFormView::OnDestroy();
}
