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
// GroupsPage.cpp : implementation file
//

#include "stdafx.h"
#include "GroupsPage.h"

#ifndef __ADDSELECTORDLG__
   #define __ADDSELECTORDLG__
   #include "AddSelectorDlg.h"
#endif
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGroupsPage property page

IMPLEMENT_DYNCREATE(CGroupsPage, CPropertyPage)

CGroupsPage::CGroupsPage() : CPropertyPage(CGroupsPage::IDD)
{
	//{{AFX_DATA_INIT(CGroupsPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CGroupsPage::~CGroupsPage()
{
}

void CGroupsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGroupsPage)
	DDX_Control(pDX, IDC_DATASOURCE, m_DataSource);
	DDX_Control(pDX, IDC_GROUPS, m_Groups);
	//}}AFX_DATA_MAP
}



BEGIN_MESSAGE_MAP(CGroupsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CGroupsPage)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGroupsPage message handlers

BOOL CGroupsPage::OnInitDialog()  {
	CPropertyPage::OnInitDialog();
	LPDISPATCH lpDispatch;
	m_DataSource.GetControlUnknown()->QueryInterface(IID_IDispatch,(void **)&lpDispatch);
	m_Groups.SetDataSource(lpDispatch);
	m_Groups.SetSelectionVariable("__GROUPSSELECTOR__");
	m_Groups.SetControlsEnable(FALSE);
	m_Groups.Update();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//----------------------------------------------------------------------------------------
// Callback runs when the user press New button in the export elements dialog on the 
// group page.
//----------------------------------------------------------------------------------------
void CGroupsPage::OnNew()  {
CAddSelectorDlg* dlg;
CMainFrame* mainFrm = (CMainFrame *)AfxGetMainWnd();

	dlg=(CAddSelectorDlg *) GetParent();
	CGroupsCombo* Group = (CGroupsCombo *) mainFrm->m_wndBrowser.GetDlgItem(IDC_GROUPS);
    m_Groups.CreateGroup();	
    Group->Update();
}
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void CGroupsPage::OnOK()  {
CAddSelectorDlg* dlg;
CMainFrame* mainFrm = (CMainFrame *)AfxGetMainWnd();

	dlg=(CAddSelectorDlg *) GetParent();
	if(dlg->GetActiveIndex()!=0) return;

	m_Groups.SetSelectionVariable("DiscoverSelection"); 
    m_Groups.AddElements();

	// Updating the group container page
	CGroupsCombo* Group = (CGroupsCombo *) mainFrm->m_wndBrowser.GetDlgItem(IDC_GROUPS);
    Group->Update();
	// updating the elements list
	CQueryResults* Result = (CQueryResults *) mainFrm->m_wndBrowser.GetDlgItem(IDC_RESULTS);
	Result->SetQuery("");
	Result->Update();
	CPropertyPage::OnOK();
}
//----------------------------------------------------------------------------------------

