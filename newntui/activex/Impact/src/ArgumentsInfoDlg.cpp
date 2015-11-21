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
// ArgumentsInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Impact.h"
#include "ArgumentsInfoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CArgumentsInfoDlg dialog


CArgumentsInfoDlg::CArgumentsInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CArgumentsInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CArgumentsInfoDlg)
	//}}AFX_DATA_INIT
	m_pArgsCtrl = new CArgumentsTableCtrl();
}

CArgumentsInfoDlg::~CArgumentsInfoDlg() {
	delete m_pArgsCtrl;
}

void CArgumentsInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CArgumentsInfoDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CArgumentsInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CArgumentsInfoDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CArgumentsInfoDlg message handlers

CArguments* CArgumentsInfoDlg::GetArguments()
{
	return m_pArgsCtrl->GetArguments();
}

void CArgumentsInfoDlg::SetArguments(CArguments* pArgs)
{
	m_pArgsCtrl->SetArguments(pArgs);
}

BOOL CArgumentsInfoDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CRect rcWindow;
	GetWindowRect(&rcWindow);
	CRect rcClient;
	GetClientRect(&rcClient);
	CButton* pOkButt = (CButton*)GetDlgItem(IDOK);
	CRect rcButt;
	pOkButt->GetWindowRect(&rcButt);
	int nHeaderHeight = rcWindow.Height() - rcClient.Height();
	m_pArgsCtrl->Create(WS_CHILD | WS_VISIBLE,
					CRect(rcClient.left + 6, rcClient.top + 6,
						  rcClient.left + rcClient.Width() - 6,
						  rcButt.top - rcWindow.top - rcClient.top - nHeaderHeight - 3),
					this,IDC_ARGUMENTS);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
