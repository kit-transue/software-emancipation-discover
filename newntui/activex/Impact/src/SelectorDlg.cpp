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
// BaseClassDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Impact.h"
#include "SelectorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectorDlg dialog


CSelectorDlg::CSelectorDlg(CString& szDlgName,CString& szSelectorName,CWnd* pParent /*=NULL*/)
	: CDialog(CSelectorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectorDlg)
	m_szSelection = _T("");
	//}}AFX_DATA_INIT
	m_szTitle = szDlgName;
	m_szSelectorName = szSelectorName;
	m_pResults = NULL;
	m_nSelectionPos = CB_ERR;
	justCreated = true;
}


void CSelectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectorDlg)
	DDX_Control(pDX, IDOK, m_OkBtn);
	DDX_Control(pDX, IDC_SELECTOR, m_wndSelector);
	DDX_CBString(pDX, IDC_SELECTOR, m_szSelection);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectorDlg, CDialog)
	//{{AFX_MSG_MAP(CSelectorDlg)
	ON_CBN_SELCHANGE(IDC_SELECTOR, OnSelchangeSelector)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectorDlg message handlers

CString& CSelectorDlg::GetSelection()
{
	return m_szSelection;
}

BOOL CSelectorDlg::OnInitDialog() 
{
	BOOL bRet;
	if((bRet=CDialog::OnInitDialog())) {
		
		SetWindowText(m_szTitle);

		CComboBox* pSelector = (CComboBox*)GetDlgItem(IDC_SELECTOR);
		CStatic* pLabel = (CStatic*)GetDlgItem(IDC_SELECTORNAME);
		pLabel->SetWindowText(m_szSelectorName);

		CDC* pDc = pLabel->GetDC();
		CSize nameSize = pDc->GetTextExtent(m_szSelectorName);

		pLabel->SetWindowPos(NULL,0,0,nameSize.cx,nameSize.cy,SWP_NOMOVE|SWP_NOZORDER);

		CRect rcParent;
		GetWindowRect(rcParent);
		CRect rcParentClient;
		GetClientRect(rcParentClient);
		CRect rcSelector;
		pSelector->GetWindowRect(rcSelector);
		CRect rcName;
		pLabel->GetWindowRect(rcName);

		pSelector->MoveWindow(rcName.right - rcParent.left,
							rcSelector.top - rcParent.top - (rcParent.Height() - rcParentClient.Height()),
							rcSelector.right - rcName.right,
							rcSelector.Height());
		
		if(m_pResults!=NULL) ShowQueryResults();
		m_OkBtn.EnableWindow(FALSE);
	}
	return bRet;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CString CSelectorDlg::GetSelectionTag()
{
	CString szTag;
	if(m_pResults!=NULL) {
		if(m_nSelectionPos != CB_ERR)
			szTag = *(*m_pResults->getRecord(m_nSelectionPos))[1];
	}
	return szTag;
}

void CSelectorDlg::ShowQueryResults()
{
	POSITION iter = m_pResults->getStartPos();
	while(iter!=NULL) {
		TRecord* pRec = m_pResults->getNext(iter);
		m_wndSelector.AddString(*((*pRec))[0]);
	}
}

void CSelectorDlg::SetQueryResults(CAttributesQueryResult* pResult) 
{
	m_pResults = pResult;
}

void CSelectorDlg::OnSelchangeSelector() 
{
	m_nSelectionPos = m_wndSelector.GetCurSel();
	m_OkBtn.EnableWindow(m_nSelectionPos!=CB_ERR);
}



void CSelectorDlg::OnSize(UINT nType, int cx, int cy) 
{
	// when we first pop up the window, we dont want the OnSize
	// method to be called because some of the components in it 
	// may not have finished being created.  
	if (justCreated ) {
		// the next time this dialog changes size, execute the following code
		justCreated = false;
	} else {
		CDialog::OnSize(nType, cx, cy);
		CRect r;
		GetClientRect(&r);
		CRect lb;
		m_wndSelector.GetWindowRect(&lb);
		ScreenToClient(&lb);
		// change the size of the combo box so that the reader can read 
		// all items in it
		m_wndSelector.SetWindowPos(NULL, 0, 0, cx - 100 , cy,
                           SWP_NOMOVE | SWP_NOZORDER);
		
	}
	
}
