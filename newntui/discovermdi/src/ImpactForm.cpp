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
// ImpactForm.cpp : implementation file
//

#include "stdafx.h"
#include "discovermdi.h"
#include "datasource.h"
#include "impact.h"
#include "ImpactForm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImpactForm

IMPLEMENT_DYNCREATE(CImpactForm, CFormView)

CImpactForm::CImpactForm()
	: CFormView(CImpactForm::IDD)
{
	//{{AFX_DATA_INIT(CImpactForm)
	//}}AFX_DATA_INIT
}

CImpactForm::~CImpactForm()
{
}

void CImpactForm::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImpactForm)
	DDX_Control(pDX, IDC_DATASOURCECTRL, m_DataSource);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImpactForm, CFormView)
	//{{AFX_MSG_MAP(CImpactForm)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImpactForm diagnostics

#ifdef _DEBUG
void CImpactForm::AssertValid() const
{
	CFormView::AssertValid();
}

void CImpactForm::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CImpactForm message handlers

bool CImpactForm::Create(CWnd *pParent, CRect &rect)
{
	CCreateContext Context;
	if(CFormView::Create(NULL,"ImpactComponent",WS_CHILD|WS_VISIBLE,rect,pParent,0,&Context)) {
		SendMessage(WM_INITIALUPDATE);
		return true;
	}
	return false;
}

void CImpactForm::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	CImpactCtrl* pCtrl = (CImpactCtrl*)GetDlgItem(IDC_IMPACTCTRL);
	if(pCtrl!=NULL && pCtrl->GetSafeHwnd()!=NULL) {
		CRect rcClient;
		GetClientRect(&rcClient);
		pCtrl->MoveWindow(&rcClient);	
	}
}

BOOL CImpactForm::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}

void CImpactForm::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	
	Update();
}

bool CImpactForm::Update()
{
	CImpactCtrl* pCtrl = (CImpactCtrl*)GetDlgItem(IDC_IMPACTCTRL);
	if(pCtrl!=NULL && pCtrl->GetSafeHwnd()!=NULL) {
		LPDISPATCH lpDispatch;
		m_DataSource.GetControlUnknown()->QueryInterface(IID_IDispatch,(void **)&lpDispatch);
		pCtrl->SetDataSource(lpDispatch);
		pCtrl->Update();
		return true;
	}
	return false;
}

bool CImpactForm::IsReportView()
{
	CImpactCtrl* pCtrl = (CImpactCtrl*)GetDlgItem(IDC_IMPACTCTRL);
	if(pCtrl!=NULL && pCtrl->GetSafeHwnd()!=NULL)
		return pCtrl->IsReportView();
	return false;
}

bool CImpactForm::SaveReportTo(CString &szFileName)
{
	CImpactCtrl* pCtrl = (CImpactCtrl*)GetDlgItem(IDC_IMPACTCTRL);
	if(pCtrl!=NULL && pCtrl->GetSafeHwnd()!=NULL)
		return pCtrl->SaveReportTo((LPCTSTR)szFileName);
	return false;
}

CString CImpactForm::GetReportName()
{
	CString szName;
	CImpactCtrl* pCtrl = (CImpactCtrl*)GetDlgItem(IDC_IMPACTCTRL);
	if(pCtrl!=NULL && pCtrl->GetSafeHwnd()!=NULL)
		szName = pCtrl->GetReportName();
	return szName;
}

void CImpactForm::SetEditorCtrl(CCodeRoverIntegrator* pEditorCtrl)
{
	CImpactCtrl* pCtrl = (CImpactCtrl*)GetDlgItem(IDC_IMPACTCTRL);
	if(pCtrl!=NULL && pCtrl->GetSafeHwnd()!=NULL) {
		LPDISPATCH lpDispatch;
		pEditorCtrl->GetControlUnknown()->QueryInterface(IID_IDispatch,(void **)&lpDispatch);
		pCtrl->SetEditorCtrl(lpDispatch);
	}
}
