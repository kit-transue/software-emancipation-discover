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
// MethodInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Impact.h"
#include "MethodInfoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMethodInfoDlg dialog


CMethodInfoDlg::CMethodInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMethodInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMethodInfoDlg)
	m_szName = _T("");
	m_szType = _T("");
	m_bVirtual = FALSE;
	//}}AFX_DATA_INIT
	m_pArgsCtrl = new CArgumentsTableCtrl();
}


CMethodInfoDlg::~CMethodInfoDlg() {
	delete m_pArgsCtrl;
}

void CMethodInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMethodInfoDlg)
	DDX_Text(pDX, IDC_NAME, m_szName);
	DDX_Text(pDX, IDC_TYPE, m_szType);
	DDX_Check(pDX, IDC_VIRTUAL, m_bVirtual);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMethodInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CMethodInfoDlg)
	ON_BN_CLICKED(IDC_PRIVATE, OnPrivate)
	ON_BN_CLICKED(IDC_PROTECTED, OnProtected)
	ON_BN_CLICKED(IDC_PUBLIC, OnPublic)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMethodInfoDlg message handlers

void CMethodInfoDlg::OnPrivate() 
{
	m_nPermission = PRIVATE;
}

void CMethodInfoDlg::OnProtected() 
{
	m_nPermission = PROTECTED;
}

void CMethodInfoDlg::OnPublic() 
{
	m_nPermission = PUBLIC;
}

CString& CMethodInfoDlg::GetName()
{
	return m_szName;
}

CString& CMethodInfoDlg::GetType()
{
	return m_szType;
}

bool CMethodInfoDlg::IsVirtual()
{
	return m_bVirtual;
}

CArguments* CMethodInfoDlg::GetArguments()
{
	return m_pArgsCtrl->GetArguments();
}

int CMethodInfoDlg::GetPermission()
{
	return m_nPermission;
}

BOOL CMethodInfoDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CRect rcWindow;
	GetWindowRect(&rcWindow);
	CRect rcClient;
	GetClientRect(&rcClient);
	CStatic* pAccessBorder = (CStatic*)GetDlgItem(IDC_ACCESSBORDER);
	CRect rcBorder;
	pAccessBorder->GetWindowRect(&rcBorder);
	CEdit* pNameEdit = (CEdit*)GetDlgItem(IDC_NAME);
	CRect rcName;
	pNameEdit->GetWindowRect(&rcName);

	int nHeaderHeight = rcWindow.Height() - rcClient.Height();
	m_pArgsCtrl->Create(WS_CHILD | WS_VISIBLE,
					CRect(rcClient.left + 6, 
						  rcName.bottom - rcWindow.top - rcClient.top - nHeaderHeight + 11,
						  rcClient.left + rcClient.Width() - 6,
						  rcBorder.top - rcWindow.top - rcClient.top - nHeaderHeight + 2),
					this,IDC_ARGUMENTS);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMethodInfoDlg::SetArguments(CArguments *pArgs)
{
	m_pArgsCtrl->SetArguments(pArgs);
}
